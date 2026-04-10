#include "Platform/GameConnectServerBootstrap.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <sstream>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace platform
{
	namespace
	{
#if defined(_WIN32)
		typedef SOCKET NativeSocket;
		const NativeSocket kInvalidSocket = INVALID_SOCKET;
#else
		typedef int NativeSocket;
		const NativeSocket kInvalidSocket = -1;
#endif

		std::uint64_t GetTickMilliseconds()
		{
			const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
			return static_cast<std::uint64_t>(
				std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
		}

		int GetConnectTimeoutErrorCode()
		{
#if defined(_WIN32)
			return WSAETIMEDOUT;
#else
			return ETIMEDOUT;
#endif
		}

		NativeSocket FromStateSocket(const ConnectServerBootstrapState* state)
		{
			return state != NULL && state->socket_open ? static_cast<NativeSocket>(state->socket_handle) : kInvalidSocket;
		}

		void CloseNativeSocket(NativeSocket socket_value)
		{
			if (socket_value == kInvalidSocket)
			{
				return;
			}

#if defined(_WIN32)
			closesocket(socket_value);
#else
			close(socket_value);
#endif
		}

		int GetLastSocketErrorCode()
		{
#if defined(_WIN32)
			return WSAGetLastError();
#else
			return errno;
#endif
		}

		bool SetSocketNonBlocking(NativeSocket socket_value)
		{
			if (socket_value == kInvalidSocket)
			{
				return false;
			}

#if defined(_WIN32)
			u_long enabled = 1;
			return ioctlsocket(socket_value, FIONBIO, &enabled) == 0;
#else
			const int flags = fcntl(socket_value, F_GETFL, 0);
			if (flags < 0)
			{
				return false;
			}

			return fcntl(socket_value, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
		}

		bool IsConnectInProgressError(int error_code)
		{
#if defined(_WIN32)
			return error_code == WSAEWOULDBLOCK || error_code == WSAEINPROGRESS || error_code == WSAEINVAL;
#else
			return error_code == EINPROGRESS || error_code == EWOULDBLOCK;
#endif
		}

		std::string FormatSocketError(int error_code)
		{
			if (error_code == 0)
			{
				return std::string("ok");
			}

#if defined(_WIN32)
			char* message_buffer = NULL;
			const DWORD message_length = FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				static_cast<DWORD>(error_code),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR)&message_buffer,
				0,
				NULL);
			std::ostringstream stream;
			stream << "WSA(" << error_code << ")";
			if (message_length > 0 && message_buffer != NULL)
			{
				while (message_length > 0 && (message_buffer[message_length - 1] == '\r' || message_buffer[message_length - 1] == '\n'))
				{
					--message_length;
				}
				stream << " " << std::string(message_buffer, message_length);
			}
			if (message_buffer != NULL)
			{
				LocalFree(message_buffer);
			}
			return stream.str();
#else
			std::ostringstream stream;
			stream << "errno(" << error_code << ")";
			const char* message = strerror(error_code);
			if (message != NULL && message[0] != '\0')
			{
				stream << " " << message;
			}
			return stream.str();
#endif
		}

		std::string FormatResolveError(int resolve_result)
		{
			if (resolve_result == 0)
			{
				return std::string("ok");
			}

#if defined(_WIN32)
			const char* message = gai_strerrorA(resolve_result);
#else
			const char* message = gai_strerror(resolve_result);
#endif
			std::ostringstream stream;
			stream << "resolve(" << resolve_result << ")";
			if (message != NULL && message[0] != '\0')
			{
				stream << " " << message;
			}
			return stream.str();
		}

		std::string FormatAddress(const sockaddr* address, socklen_t address_length)
		{
			if (address == NULL || address_length == 0)
			{
				return std::string();
			}

			char host_buffer[NI_MAXHOST] = { 0 };
			char service_buffer[NI_MAXSERV] = { 0 };
			if (getnameinfo(
					address,
					address_length,
					host_buffer,
					sizeof(host_buffer),
					service_buffer,
					sizeof(service_buffer),
					NI_NUMERICHOST | NI_NUMERICSERV) != 0)
			{
				return std::string();
			}

			std::string result = host_buffer;
			if (service_buffer[0] != '\0')
			{
				result += ":";
				result += service_buffer;
			}

			return result;
		}

		void AdoptSocket(
			ConnectServerBootstrapState* state,
			NativeSocket socket_value,
			ConnectServerBootstrapStatus status,
			const std::string& endpoint,
			const std::string& message,
			int error_code)
		{
			if (state == NULL)
			{
				CloseNativeSocket(socket_value);
				return;
			}

			const NativeSocket current_socket = FromStateSocket(state);
			if (current_socket != kInvalidSocket && current_socket != socket_value)
			{
				CloseNativeSocket(current_socket);
			}

			state->socket_handle = static_cast<intptr_t>(socket_value);
			state->socket_open = socket_value != kInvalidSocket;
			state->status = status;
			state->resolved_endpoint = endpoint;
			state->status_message = message;
			state->last_error_code = error_code;
		}

		void SetFailure(ConnectServerBootstrapState* state, int error_code, const std::string& endpoint_hint)
		{
			if (state == NULL)
			{
				return;
			}

			const NativeSocket current_socket = FromStateSocket(state);
			if (current_socket != kInvalidSocket)
			{
				CloseNativeSocket(current_socket);
			}

			state->socket_handle = 0;
			state->socket_open = false;
			state->status = ConnectServerBootstrapStatus_Failed;
			state->resolved_endpoint = endpoint_hint;
			state->last_error_code = error_code;

			std::ostringstream stream;
			const std::uint64_t elapsed_millis =
				state->connect_started_at_millis > 0 ? (GetTickMilliseconds() - state->connect_started_at_millis) : 0;
			if (error_code == GetConnectTimeoutErrorCode())
			{
				stream << "Tempo esgotado ao conectar";
			}
			else
			{
				stream << "Falha ao conectar";
			}
			if (!endpoint_hint.empty())
			{
				stream << " " << endpoint_hint;
			}
			if (error_code != 0)
			{
				stream << " (" << FormatSocketError(error_code) << ")";
			}
			if (elapsed_millis > 0)
			{
				stream << " em " << elapsed_millis << "ms";
			}
			state->status_message = stream.str();
		}

		unsigned short ReadWord(const unsigned char* buffer)
		{
			if (buffer == NULL)
			{
				return 0;
			}

			return static_cast<unsigned short>(buffer[0] | (static_cast<unsigned short>(buffer[1]) << 8));
		}

		bool SendAll(NativeSocket socket_value, const unsigned char* data, size_t size)
		{
			if (socket_value == kInvalidSocket || data == NULL || size == 0)
			{
				return false;
			}

			size_t offset = 0;
			while (offset < size)
			{
				const int sent = send(
					socket_value,
					reinterpret_cast<const char*>(data + offset),
					static_cast<int>(size - offset),
					0);
				if (sent <= 0)
				{
					const int error_code = GetLastSocketErrorCode();
					if (IsConnectInProgressError(error_code))
					{
						continue;
					}
					return false;
				}

				offset += static_cast<size_t>(sent);
			}

			return true;
		}

		bool SendPacket(ConnectServerBootstrapState* state, const unsigned char* data, size_t size)
		{
			if (state == NULL || state->status != ConnectServerBootstrapStatus_Connected || !state->socket_open)
			{
				return false;
			}

			if (SendAll(FromStateSocket(state), data, size))
			{
				return true;
			}

			SetFailure(state, GetLastSocketErrorCode(), state->resolved_endpoint);
			return false;
		}

		bool ExtractPacketSize(const std::vector<unsigned char>& buffer, size_t* out_size)
		{
			if (out_size == NULL || buffer.empty())
			{
				return false;
			}

			const unsigned char code = buffer[0];
			if (code == 0xC1 || code == 0xC3)
			{
				if (buffer.size() < 2)
				{
					return false;
				}

				*out_size = buffer[1];
				return *out_size >= 3;
			}

			if (code == 0xC2 || code == 0xC4)
			{
				if (buffer.size() < 3)
				{
					return false;
				}

				*out_size = static_cast<size_t>((buffer[1] << 8) | buffer[2]);
				return *out_size >= 4;
			}

			return false;
		}

		void ParseConnectServerPacket(ConnectServerBootstrapState* state, const unsigned char* packet, size_t packet_size)
		{
			if (state == NULL || packet == NULL || packet_size < 4)
			{
				return;
			}

			const unsigned char code = packet[0];
			const size_t head_offset = (code == 0xC1 || code == 0xC3) ? 2u : 3u;
			if (packet_size <= head_offset + 1)
			{
				return;
			}

			const unsigned char head_code = packet[head_offset];
			const unsigned char sub_code = packet[head_offset + 1];
			if (head_code != 0xF4)
			{
				return;
			}

			if (sub_code == 0x06)
			{
				if (packet_size < head_offset + 3)
				{
					return;
				}

				const unsigned short total = static_cast<unsigned short>(packet[head_offset + 2] | (static_cast<unsigned short>(packet[head_offset + 3]) << 8));
				const size_t entry_offset = head_offset + 4;
				const size_t entry_size = 3;
				if (packet_size < entry_offset)
				{
					return;
				}

				const size_t available_entries = (packet_size - entry_offset) / entry_size;
				state->total_server_count = total;
				state->server_entries.clear();
				state->server_entries.reserve(available_entries);

				for (size_t index = 0; index < available_entries; ++index)
				{
					const unsigned char* entry = packet + entry_offset + (index * entry_size);
					ConnectServerEntry server_entry = {};
					server_entry.connect_index = ReadWord(entry);
					server_entry.load_percent = entry[2];
					state->server_entries.push_back(server_entry);
				}

				state->server_list_received = true;
				std::ostringstream stream;
				stream << "Server list recebida: " << state->server_entries.size();
				state->status_message = stream.str();
				return;
			}

			if (sub_code == 0x03)
			{
				if (packet_size < head_offset + 1 + 15 + 2)
				{
					return;
				}

				char host_buffer[16] = { 0 };
				memcpy(host_buffer, packet + head_offset + 2, 15);
				state->selected_game_server_host = host_buffer;
				state->selected_game_server_port = ReadWord(packet + head_offset + 17);
				state->server_address_received = true;

				std::ostringstream stream;
				stream << "GameServer " << state->selected_game_server_host << ":" << state->selected_game_server_port;
				state->status_message = stream.str();
				return;
			}

			if (sub_code == 0x05)
			{
				state->status_message = "ConnectServer ocupado";
			}
		}

		void ReceivePackets(ConnectServerBootstrapState* state)
		{
			if (state == NULL || state->status != ConnectServerBootstrapStatus_Connected || !state->socket_open)
			{
				return;
			}

			NativeSocket socket_value = FromStateSocket(state);
			if (socket_value == kInvalidSocket)
			{
				return;
			}

			unsigned char temp_buffer[1024];
			while (true)
			{
				const int received = recv(socket_value, reinterpret_cast<char*>(temp_buffer), sizeof(temp_buffer), 0);
				if (received == 0)
				{
					DisconnectConnectServerBootstrap(state, "ConnectServer fechou a conexao");
					return;
				}

				if (received < 0)
				{
					const int error_code = GetLastSocketErrorCode();
					if (IsConnectInProgressError(error_code))
					{
						break;
					}

					SetFailure(state, error_code, state->resolved_endpoint);
					return;
				}

				state->receive_buffer.insert(state->receive_buffer.end(), temp_buffer, temp_buffer + received);
			}

			while (!state->receive_buffer.empty())
			{
				size_t packet_size = 0;
				if (!ExtractPacketSize(state->receive_buffer, &packet_size))
				{
					if (!state->receive_buffer.empty() &&
						state->receive_buffer[0] != 0xC1 &&
						state->receive_buffer[0] != 0xC2 &&
						state->receive_buffer[0] != 0xC3 &&
						state->receive_buffer[0] != 0xC4)
					{
						state->receive_buffer.erase(state->receive_buffer.begin());
						continue;
					}
					break;
				}

				if (state->receive_buffer.size() < packet_size)
				{
					break;
				}

				ParseConnectServerPacket(state, &state->receive_buffer[0], packet_size);
				state->receive_buffer.erase(state->receive_buffer.begin(), state->receive_buffer.begin() + static_cast<long>(packet_size));
			}
		}
	}

	void InitializeConnectServerBootstrapState(ConnectServerBootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		state->configured = false;
		state->host.clear();
		state->port = 0;
		state->status = ConnectServerBootstrapStatus_Idle;
		state->resolved_endpoint.clear();
		state->status_message = "ConnectServer indisponivel";
		state->attempt_count = 0;
		state->connect_started_at_millis = 0;
		state->connect_timeout_millis = 6000;
		state->last_error_code = 0;
		state->socket_handle = 0;
		state->socket_open = false;
		state->server_list_requested = false;
		state->server_list_received = false;
		state->server_address_requested = false;
		state->server_address_received = false;
		state->requested_server_index = 0;
		state->total_server_count = 0;
		state->receive_buffer.clear();
		state->server_entries.clear();
		state->selected_game_server_host.clear();
		state->selected_game_server_port = 0;
	}

	void ShutdownConnectServerBootstrap(ConnectServerBootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		DisconnectConnectServerBootstrap(state, "Conexao encerrada");
		state->configured = false;
		state->host.clear();
		state->port = 0;
	}

	bool ConfigureConnectServerBootstrap(ConnectServerBootstrapState* state, const char* host, unsigned short port)
	{
		if (state == NULL)
		{
			return false;
		}

		DisconnectConnectServerBootstrap(state, NULL);
		state->configured = host != NULL && host[0] != '\0' && port != 0;
		state->host = host != NULL ? host : "";
		state->port = port;
		state->resolved_endpoint.clear();
		state->connect_started_at_millis = 0;
		state->last_error_code = 0;
		state->status = ConnectServerBootstrapStatus_Idle;
		state->server_list_requested = false;
		state->server_list_received = false;
		state->server_address_requested = false;
		state->server_address_received = false;
		state->requested_server_index = 0;
		state->total_server_count = 0;
		state->receive_buffer.clear();
		state->server_entries.clear();
		state->selected_game_server_host.clear();
		state->selected_game_server_port = 0;

		if (!state->configured)
		{
			state->status_message = "ConnectServer invalido";
			return false;
		}

		std::ostringstream stream;
		stream << "Servidor pronto " << state->host << ":" << state->port;
		state->status_message = stream.str();
		return true;
	}

	bool StartConnectServerBootstrap(ConnectServerBootstrapState* state)
	{
		if (state == NULL || !state->configured)
		{
			if (state != NULL)
			{
				state->status = ConnectServerBootstrapStatus_Failed;
				state->status_message = "ConnectServer nao configurado";
			}
			return false;
		}

		DisconnectConnectServerBootstrap(state, NULL);
		state->attempt_count += 1;
		state->connect_started_at_millis = GetTickMilliseconds();
		state->last_error_code = 0;
		state->resolved_endpoint.clear();
		state->server_list_requested = false;
		state->server_list_received = false;
		state->server_address_requested = false;
		state->server_address_received = false;
		state->requested_server_index = 0;
		state->total_server_count = 0;
		state->receive_buffer.clear();
		state->server_entries.clear();
		state->selected_game_server_host.clear();
		state->selected_game_server_port = 0;

		char port_buffer[16] = { 0 };
		snprintf(port_buffer, sizeof(port_buffer), "%u", static_cast<unsigned int>(state->port));

		addrinfo hints = {};
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		addrinfo* address_list = NULL;
		const int resolve_result = getaddrinfo(state->host.c_str(), port_buffer, &hints, &address_list);
		if (resolve_result != 0 || address_list == NULL)
		{
			const std::string endpoint_text = state->host + ":" + port_buffer;
			const int resolve_error = resolve_result != 0 ? resolve_result : GetLastSocketErrorCode();
			state->status = ConnectServerBootstrapStatus_Failed;
			state->last_error_code = resolve_error;
			state->resolved_endpoint = endpoint_text;
			state->status_message = std::string("Falha ao resolver ") + endpoint_text + " (" + FormatResolveError(resolve_error) + ")";
			return false;
		}

		bool started = false;
		int last_error_code = 0;
		std::string last_endpoint;
		for (addrinfo* current = address_list; current != NULL; current = current->ai_next)
		{
			NativeSocket socket_value = socket(current->ai_family, current->ai_socktype, current->ai_protocol);
			if (socket_value == kInvalidSocket)
			{
				last_error_code = GetLastSocketErrorCode();
				continue;
			}

			if (!SetSocketNonBlocking(socket_value))
			{
				last_error_code = GetLastSocketErrorCode();
				CloseNativeSocket(socket_value);
				continue;
			}

			last_endpoint = FormatAddress(current->ai_addr, static_cast<socklen_t>(current->ai_addrlen));
			const int connect_result = connect(socket_value, current->ai_addr, static_cast<socklen_t>(current->ai_addrlen));
			if (connect_result == 0)
			{
				AdoptSocket(
					state,
					socket_value,
					ConnectServerBootstrapStatus_Connected,
					last_endpoint,
					std::string("Conectado a ") + last_endpoint,
					0);
				started = true;
				break;
			}

			last_error_code = GetLastSocketErrorCode();
			if (IsConnectInProgressError(last_error_code))
			{
				AdoptSocket(
					state,
					socket_value,
					ConnectServerBootstrapStatus_Connecting,
					last_endpoint,
					std::string("Conectando em ") + last_endpoint,
					0);
				started = true;
				break;
			}

			CloseNativeSocket(socket_value);
		}

		freeaddrinfo(address_list);

		if (!started)
		{
			SetFailure(state, last_error_code, last_endpoint.empty() ? (state->host + ":" + port_buffer) : last_endpoint);
			return false;
		}

		return true;
	}

	void PollConnectServerBootstrap(ConnectServerBootstrapState* state)
	{
		if (state == NULL || !state->socket_open)
		{
			return;
		}

		if (state->status == ConnectServerBootstrapStatus_Connected)
		{
			ReceivePackets(state);
			return;
		}

		if (state->status != ConnectServerBootstrapStatus_Connecting)
		{
			return;
		}

		const NativeSocket socket_value = FromStateSocket(state);
		if (socket_value == kInvalidSocket)
		{
			SetFailure(state, 0, state->resolved_endpoint);
			return;
		}

		fd_set write_fds;
		fd_set error_fds;
		FD_ZERO(&write_fds);
		FD_ZERO(&error_fds);
		FD_SET(socket_value, &write_fds);
		FD_SET(socket_value, &error_fds);

		timeval timeout = {};
		const int select_result =
#if defined(_WIN32)
			select(0, NULL, &write_fds, &error_fds, &timeout);
#else
			select(socket_value + 1, NULL, &write_fds, &error_fds, &timeout);
#endif
		if (select_result <= 0)
		{
			if (select_result < 0)
			{
				SetFailure(state, GetLastSocketErrorCode(), state->resolved_endpoint);
				return;
			}

			if (state->connect_timeout_millis > 0 &&
				state->connect_started_at_millis > 0 &&
				(GetTickMilliseconds() - state->connect_started_at_millis) >= state->connect_timeout_millis)
			{
				SetFailure(state, GetConnectTimeoutErrorCode(), state->resolved_endpoint);
			}
			return;
		}

		int socket_error = 0;
#if defined(_WIN32)
		int option_length = sizeof(socket_error);
#else
		socklen_t option_length = sizeof(socket_error);
#endif
		if (getsockopt(socket_value, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&socket_error), &option_length) != 0)
		{
			socket_error = GetLastSocketErrorCode();
		}

		if (socket_error != 0 || FD_ISSET(socket_value, &error_fds))
		{
			SetFailure(state, socket_error != 0 ? socket_error : GetLastSocketErrorCode(), state->resolved_endpoint);
			return;
		}

		state->status = ConnectServerBootstrapStatus_Connected;
		state->last_error_code = 0;
		state->status_message = std::string("Conectado a ") + state->resolved_endpoint;
		ReceivePackets(state);
	}

	bool RequestConnectServerListBootstrap(ConnectServerBootstrapState* state)
	{
		static const unsigned char kPacket[] = { 0xC1, 0x04, 0xF4, 0x06 };
		if (state == NULL || !SendPacket(state, kPacket, sizeof(kPacket)))
		{
			return false;
		}

		state->server_list_requested = true;
		state->status_message = "Solicitando server list";
		return true;
	}

	bool RequestConnectServerAddressBootstrap(ConnectServerBootstrapState* state, unsigned short connect_index)
	{
		unsigned char packet[6] = { 0xC1, 0x06, 0xF4, 0x03, 0x00, 0x00 };
		packet[4] = static_cast<unsigned char>(connect_index & 0xFF);
		packet[5] = static_cast<unsigned char>((connect_index >> 8) & 0xFF);
		if (state == NULL || !SendPacket(state, packet, sizeof(packet)))
		{
			return false;
		}

		state->server_address_requested = true;
		state->server_address_received = false;
		state->requested_server_index = connect_index;
		std::ostringstream stream;
		stream << "Solicitando endereco do servidor " << static_cast<unsigned int>(connect_index + 1);
		state->status_message = stream.str();
		return true;
	}

	void DisconnectConnectServerBootstrap(ConnectServerBootstrapState* state, const char* reason)
	{
		if (state == NULL)
		{
			return;
		}

		const NativeSocket socket_value = FromStateSocket(state);
		if (socket_value != kInvalidSocket)
		{
			CloseNativeSocket(socket_value);
		}

		state->socket_handle = 0;
		state->socket_open = false;
		state->connect_started_at_millis = 0;
		state->last_error_code = 0;
		state->status = ConnectServerBootstrapStatus_Idle;
		state->server_list_requested = false;
		state->server_list_received = false;
		state->server_address_requested = false;
		state->server_address_received = false;
		state->requested_server_index = 0;
		state->total_server_count = 0;
		state->receive_buffer.clear();
		state->server_entries.clear();
		state->selected_game_server_host.clear();
		state->selected_game_server_port = 0;
		if (reason != NULL && reason[0] != '\0')
		{
			state->status_message = reason;
		}
	}

	const char* GetConnectServerBootstrapStatusLabel(ConnectServerBootstrapStatus status)
	{
		switch (status)
		{
		case ConnectServerBootstrapStatus_Idle:
			return "idle";
		case ConnectServerBootstrapStatus_Connecting:
			return "connecting";
		case ConnectServerBootstrapStatus_Connected:
			return "connected";
		case ConnectServerBootstrapStatus_Failed:
			return "failed";
		default:
			return "unknown";
		}
	}
}

#include "Platform/GameServerBootstrap.h"

#include "PacketManager.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
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

		const size_t kLoginAccountSize = 10;
		const size_t kLoginPasswordSize = 20;
		const size_t kLoginVersionSize = 5;
		const size_t kLoginSerialSize = 16;
		const size_t kCharacterNameSize = 10;
		const size_t kCharacterListEntrySize = 34;
		const unsigned char kBuxCode[3] = { 0xFC, 0xCF, 0xAB };
		const unsigned char kStreamPacketXorFilter[32] =
		{
			0xE7, 0x6D, 0x3A, 0x89, 0xBC, 0xB2, 0x9F, 0x73,
			0x23, 0xA8, 0xFE, 0xB6, 0x49, 0x5D, 0x39, 0x5D,
			0x8A, 0xCB, 0x63, 0x8D, 0xEA, 0x7D, 0x2B, 0x5F,
			0xC3, 0xB1, 0xE9, 0x83, 0x29, 0x51, 0xE8, 0x56
		};

		bool SendAll(NativeSocket socket_value, const unsigned char* data, size_t size);
		void SetFailure(GameServerBootstrapState* state, int error_code, const std::string& endpoint_hint);
		void AppendPacketBytes(
			std::vector<unsigned char>* packet,
			const unsigned char* data,
			size_t size,
			bool apply_xor);

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

		NativeSocket FromStateSocket(const GameServerBootstrapState* state)
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

		const char* DescribeLoginResult(unsigned char result)
		{
			switch (result)
			{
			case 0x00: return "Senha incorreta";
			case 0x01: return "Login OK";
			case 0x02: return "Conta inexistente";
			case 0x03: return "Conta ja conectada";
			case 0x04: return "Servidor cheio";
			case 0x05: return "Conta bloqueada";
			case 0x06: return "Versao invalida";
			case 0x08: return "Falha de login";
			case 0x09: return "Sem informacao de pagamento";
			case 0x20: return "Login OK";
			default: return "Falha de conexao";
			}
		}

		bool SendEncryptedF1Packet(GameServerBootstrapState* state, unsigned char sub_code, const unsigned char* payload, size_t payload_size)
		{
			if (state == NULL || !state->socket_open)
			{
				return false;
			}

			std::vector<unsigned char> plain_packet;
			plain_packet.reserve(4 + payload_size);

			const unsigned char header_code = 0xC1;
			const unsigned char packet_size_placeholder = 0;
			const unsigned char head_code = 0xF1;

			AppendPacketBytes(&plain_packet, &header_code, sizeof(header_code), false);
			AppendPacketBytes(&plain_packet, &packet_size_placeholder, sizeof(packet_size_placeholder), false);
			AppendPacketBytes(&plain_packet, &head_code, sizeof(head_code), false);
			AppendPacketBytes(&plain_packet, &sub_code, sizeof(sub_code), true);
			if (payload != NULL && payload_size > 0)
			{
				AppendPacketBytes(&plain_packet, payload, payload_size, true);
			}

			if (plain_packet.size() > 0xFF)
			{
				state->status = GameServerBootstrapStatus_Failed;
				state->status_message = "Pacote F1 excedeu o limite C1";
				return false;
			}

			plain_packet[1] = static_cast<unsigned char>(plain_packet.size());

			unsigned char send_packet[2048] = { 0 };
			std::vector<unsigned char> encrypted_source = plain_packet;
			encrypted_source[1] = state->send_packet_serial++;
			int encrypted_size = gPacketManager.Encrypt(
				&send_packet[2],
				&encrypted_source[1],
				static_cast<int>(encrypted_source.size() - 1));
			if (encrypted_size <= 0)
			{
				state->status = GameServerBootstrapStatus_Failed;
				state->status_message = "Falha ao criptografar pacote F1";
				return false;
			}

			encrypted_size += 2;
			send_packet[0] = 0xC3;
			send_packet[1] = static_cast<unsigned char>(encrypted_size & 0xFF);

			if (!SendAll(FromStateSocket(state), send_packet, static_cast<size_t>(encrypted_size)))
			{
				SetFailure(state, GetLastSocketErrorCode(), state->resolved_endpoint);
				return false;
			}

			return true;
		}

		bool RequestDuplicateLoginDisconnect(GameServerBootstrapState* state)
		{
			const unsigned char logout_flag = 2;
			if (!SendEncryptedF1Packet(state, 0x02, &logout_flag, sizeof(logout_flag)))
			{
				return false;
			}

			state->duplicate_login_disconnect_pending = true;
			state->status_message = "Conta ja conectada, solicitando desconexao";
			return true;
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
			GameServerBootstrapState* state,
			NativeSocket socket_value,
			GameServerBootstrapStatus status,
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

		void SetFailure(GameServerBootstrapState* state, int error_code, const std::string& endpoint_hint)
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
			state->status = GameServerBootstrapStatus_Failed;
			state->resolved_endpoint = endpoint_hint;
			state->last_error_code = error_code;
			state->login_pending = false;

			const std::uint64_t elapsed_millis =
				state->connect_started_at_millis > 0 ? (GetTickMilliseconds() - state->connect_started_at_millis) : 0;
			std::ostringstream stream;
			if (error_code == GetConnectTimeoutErrorCode())
			{
				stream << "Tempo esgotado ao conectar GameServer";
			}
			else
			{
				stream << "Falha ao conectar GameServer";
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

		size_t ReadPacketSize(const std::vector<unsigned char>& buffer)
		{
			if (buffer.empty())
			{
				return 0;
			}

			const unsigned char code = buffer[0];
			if (code == 0xC1 || code == 0xC3)
			{
				return (buffer.size() >= 2) ? static_cast<size_t>(buffer[1]) : 0u;
			}

			if (code == 0xC2 || code == 0xC4)
			{
				return (buffer.size() >= 3) ? static_cast<size_t>((buffer[1] << 8) | buffer[2]) : 0u;
			}

			return 0;
		}

			std::string BuildJoinVersionString(const unsigned char* buffer, size_t size)
			{
				if (buffer == NULL || size == 0)
				{
				return std::string();
			}

			std::string version;
			version.reserve(size);
			for (size_t index = 0; index < size; ++index)
			{
				if (buffer[index] == 0)
				{
					break;
				}
				version.push_back(static_cast<char>(buffer[index]));
				}
				return version;
			}

			void AppendPacketBytes(
				std::vector<unsigned char>* packet,
				const unsigned char* data,
				size_t size,
				bool apply_xor)
			{
				if (packet == NULL || data == NULL || size == 0)
				{
					return;
				}

				const size_t start = packet->size();
				packet->insert(packet->end(), data, data + size);

				if (!apply_xor)
				{
					return;
				}

				for (size_t index = start; index < packet->size(); ++index)
				{
					if (index == 0)
					{
						continue;
					}

					(*packet)[index] ^= ((*packet)[index - 1] ^ kStreamPacketXorFilter[index % 32]);
				}
			}

		void ParseGameServerPacket(GameServerBootstrapState* state, const unsigned char* packet, size_t packet_size)
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

			if (head_code == 0xF1)
			{
				if (sub_code == 0x00)
				{
					if (packet_size < head_offset + 8)
					{
						return;
					}

					state->join_server_received = true;
					state->join_server_result = packet[head_offset + 2];
					state->hero_key = static_cast<unsigned short>(
						(static_cast<unsigned short>(packet[head_offset + 3]) << 8) |
						static_cast<unsigned short>(packet[head_offset + 4]));
					state->join_server_version = BuildJoinVersionString(packet + head_offset + 5, std::min<size_t>(5u, packet_size - (head_offset + 5)));
					state->join_server_success = (state->join_server_result == 0x01);

					if (state->join_server_success)
					{
						state->status = GameServerBootstrapStatus_JoinReady;
						std::ostringstream stream;
						stream << "JoinServer OK hero=" << state->hero_key;
						if (!state->join_server_version.empty())
						{
							stream << " ver=" << state->join_server_version;
						}
						state->status_message = stream.str();
					}
					else
					{
						state->status = GameServerBootstrapStatus_Failed;
						std::ostringstream stream;
						stream << "JoinServer recusou result=" << static_cast<unsigned int>(state->join_server_result);
						state->status_message = stream.str();
					}
					return;
				}

				if (sub_code == 0x01)
				{
					if (packet_size < head_offset + 3)
					{
						return;
					}

					state->login_result_received = true;
					state->login_result = packet[head_offset + 2];
					if (state->login_result == 0x01 || state->login_result == 0x20)
					{
						state->status = GameServerBootstrapStatus_LoginSucceeded;
						state->status_message = "Login OK";
					}
					else
					{
						state->status = GameServerBootstrapStatus_LoginFailed;
						std::ostringstream stream;
						stream << DescribeLoginResult(state->login_result)
							   << " (result=" << static_cast<unsigned int>(state->login_result) << ")";
						state->status_message = stream.str();

						if (state->login_result == 0x03 &&
							!state->duplicate_login_disconnect_tried &&
							!state->account.empty() &&
							!state->password.empty())
						{
							state->duplicate_login_disconnect_tried = true;
							if (!RequestDuplicateLoginDisconnect(state))
							{
								std::ostringstream retry_stream;
								retry_stream << "Conta ja conectada, falha ao solicitar desconexao"
									<< " (result=" << static_cast<unsigned int>(state->login_result) << ")";
								state->status_message = retry_stream.str();
							}
						}
					}
					return;
				}

				if (sub_code == 0x02)
				{
					if (packet_size < head_offset + 3)
					{
						return;
					}

					const unsigned char logout_result = packet[head_offset + 2];
					if (state->duplicate_login_disconnect_pending)
					{
						state->duplicate_login_disconnect_pending = false;
						DisconnectGameServerBootstrap(state, NULL);
						if (!StartGameServerBootstrap(state))
						{
							std::ostringstream stream;
							stream << "Conta desconectada, falha ao reconectar"
								   << " (logout=" << static_cast<unsigned int>(logout_result) << ")";
							state->status = GameServerBootstrapStatus_Failed;
							state->status_message = stream.str();
							return;
						}

						state->login_pending = true;
						std::ostringstream stream;
						stream << "Conta desconectada, reconectando"
							   << " (logout=" << static_cast<unsigned int>(logout_result) << ")";
						state->status_message = stream.str();
						return;
					}

					return;
				}

				return;
			}

			if (head_code == 0xF3 && sub_code == 0x00)
			{
				if (packet_size < head_offset + 6)
				{
					return;
				}

				state->character_list_requested = true;
				state->character_list_received = true;
				state->character_class_code = packet[head_offset + 2];
				state->character_move_count = packet[head_offset + 3];
				const unsigned char character_count = packet[head_offset + 4];
				state->max_character_count = packet[head_offset + 5];
				state->characters.clear();

				size_t offset = head_offset + 6;
				for (unsigned int index = 0; index < character_count && (offset + kCharacterListEntrySize) <= packet_size; ++index)
				{
					const unsigned char* entry_data = packet + offset;
					GameServerCharacterEntry entry = {};
					entry.slot = entry_data[0];
					entry.name.assign(reinterpret_cast<const char*>(entry_data + 1), 10);
					const std::string::size_type zero_position = entry.name.find('\0');
					if (zero_position != std::string::npos)
					{
						entry.name.resize(zero_position);
					}
					entry.level = static_cast<std::uint16_t>(
						static_cast<std::uint16_t>(entry_data[12]) |
						(static_cast<std::uint16_t>(entry_data[13]) << 8));
					entry.ctl_code = entry_data[14];
					entry.class_code = entry_data[15];
					entry.guild_status = entry_data[33];
					std::memcpy(entry.char_set, entry_data + 15, sizeof(entry.char_set));
					state->characters.push_back(entry);
					offset += kCharacterListEntrySize;
				}

				state->status = GameServerBootstrapStatus_CharacterListReady;
				std::ostringstream stream;
				stream << "Lista de personagens " << state->characters.size() << "/" << static_cast<unsigned int>(state->max_character_count);
				if (!state->characters.empty())
				{
					stream << ": ";
					for (size_t index = 0; index < state->characters.size(); ++index)
					{
						if (index > 0)
						{
							stream << ", ";
						}
						stream << "[" << static_cast<unsigned int>(state->characters[index].slot) << "]"
							   << state->characters[index].name;
					}
				}
				state->status_message = stream.str();
				return;
			}

			if (head_code == 0xF3 && sub_code == 0x03)
			{
				if (packet_size < head_offset + 6)
				{
					return;
				}

				state->map_join_requested = true;
				state->map_join_received = true;
				state->joined_map = packet[head_offset + 2];
				state->joined_x = packet[head_offset + 3];
				state->joined_y = packet[head_offset + 4];
				state->joined_angle = packet[head_offset + 5];
				state->status = GameServerBootstrapStatus_MapJoinReady;

				std::ostringstream stream;
				stream << "JoinMap OK";
				if (!state->selected_character_name.empty())
				{
					stream << " " << state->selected_character_name;
				}
				stream << " map=" << static_cast<unsigned int>(state->joined_map)
					   << " x=" << static_cast<unsigned int>(state->joined_x)
					   << " y=" << static_cast<unsigned int>(state->joined_y);
				state->status_message = stream.str();
			}
		}

		void BuxConvert(unsigned char* buffer, size_t size)
		{
			if (buffer == NULL)
			{
				return;
			}

			for (size_t index = 0; index < size; ++index)
			{
				buffer[index] ^= kBuxCode[index % 3];
			}
		}

			std::string BuildPackedVersionString(const std::string& client_version)
			{
			std::string packed;
			packed.reserve(kLoginVersionSize);
			for (size_t index = 0; index < client_version.size() && packed.size() < kLoginVersionSize; ++index)
			{
				if (client_version[index] >= '0' && client_version[index] <= '9')
				{
					packed.push_back(client_version[index]);
				}
			}

			while (packed.size() < kLoginVersionSize)
			{
				packed.push_back('0');
			}

				return packed;
			}

			unsigned char ResolveLanguageCode(const std::string& language)
			{
				if (language == "POR" || language == "Por" || language == "por")
				{
					return 1;
				}

				if (language == "SPN" || language == "Spn" || language == "spn")
				{
					return 2;
				}

				return 0;
			}

			bool SendStreamPacket(
				GameServerBootstrapState* state,
				unsigned char head_code,
				const unsigned char* payload,
				size_t payload_size)
			{
				if (state == NULL || !state->socket_open || payload == NULL || payload_size == 0)
				{
					return false;
				}

				std::vector<unsigned char> packet;
				packet.reserve(3 + payload_size);

				const unsigned char header_code = 0xC1;
				const unsigned char packet_size_placeholder = 0;
				AppendPacketBytes(&packet, &header_code, sizeof(header_code), false);
				AppendPacketBytes(&packet, &packet_size_placeholder, sizeof(packet_size_placeholder), false);
				AppendPacketBytes(&packet, &head_code, sizeof(head_code), false);
				AppendPacketBytes(&packet, payload, payload_size, true);

				if (packet.size() > 0xFF)
				{
					return false;
				}

				packet[1] = static_cast<unsigned char>(packet.size());
				return SendAll(FromStateSocket(state), &packet[0], packet.size());
			}

			bool RequestCharacterList(GameServerBootstrapState* state)
			{
				if (state == NULL || !state->socket_open)
				{
					return false;
				}

				const unsigned char payload[2] =
				{
					0x00,
					ResolveLanguageCode(state->language)
				};

				if (!SendStreamPacket(state, 0xF3, payload, sizeof(payload)))
				{
					return false;
				}

				state->character_list_requested = true;
				state->status_message = "Solicitando lista de personagens";
				return true;
			}

			bool RequestJoinMapServer(GameServerBootstrapState* state, const char* character_name)
			{
				if (state == NULL || !state->socket_open || character_name == NULL || character_name[0] == '\0')
				{
					return false;
				}

				unsigned char payload[1 + kCharacterNameSize] = { 0 };
				payload[0] = 0x03;
				const size_t character_name_length = std::min(strlen(character_name), kCharacterNameSize);
				memcpy(payload + 1, character_name, character_name_length);

				if (!SendStreamPacket(state, 0xF3, payload, sizeof(payload)))
				{
					return false;
				}

				state->map_join_requested = true;
				state->map_join_received = false;
				state->joined_map = 0;
				state->joined_x = 0;
				state->joined_y = 0;
				state->joined_angle = 0;
				state->selected_character_name.assign(character_name, character_name_length);
				state->status_message = std::string("Entrando com ") + state->selected_character_name;
				return true;
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

			void ReceivePackets(GameServerBootstrapState* state)
			{
			if (state == NULL || !state->socket_open)
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
					DisconnectGameServerBootstrap(state, "GameServer fechou a conexao");
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
				if (state->receive_buffer[0] != 0xC1 &&
					state->receive_buffer[0] != 0xC2 &&
					state->receive_buffer[0] != 0xC3 &&
					state->receive_buffer[0] != 0xC4)
				{
					state->receive_buffer.erase(state->receive_buffer.begin());
					continue;
				}

				const size_t packet_size = ReadPacketSize(state->receive_buffer);
				if (packet_size == 0 || state->receive_buffer.size() < packet_size)
				{
					break;
				}

				ParseGameServerPacket(state, &state->receive_buffer[0], packet_size);
				state->receive_buffer.erase(
					state->receive_buffer.begin(),
					state->receive_buffer.begin() + static_cast<long>(packet_size));
			}
		}
	}

	void InitializeGameServerBootstrapState(GameServerBootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		state->configured = false;
		state->host.clear();
		state->port = 0;
		state->account.clear();
		state->password.clear();
		state->language.clear();
		state->client_version.clear();
		state->client_serial.clear();
		state->status = GameServerBootstrapStatus_Idle;
		state->resolved_endpoint.clear();
		state->status_message = "GameServer indisponivel";
		state->attempt_count = 0;
		state->connect_started_at_millis = 0;
		state->connect_timeout_millis = 6000;
		state->last_error_code = 0;
		state->socket_handle = 0;
		state->socket_open = false;
		state->join_server_received = false;
		state->join_server_success = false;
		state->join_server_result = 0xFF;
		state->hero_key = 0;
		state->join_server_version.clear();
		state->login_pending = false;
		state->login_requested = false;
		state->login_result_received = false;
		state->login_result = 0xFF;
		state->duplicate_login_disconnect_tried = false;
		state->duplicate_login_disconnect_pending = false;
		state->character_list_requested = false;
		state->character_list_received = false;
		state->character_class_code = 0;
		state->character_move_count = 0;
		state->max_character_count = 0;
		state->map_join_requested = false;
		state->map_join_received = false;
		state->selected_character_slot = 0xFF;
		state->selected_character_name.clear();
		state->joined_map = 0;
		state->joined_x = 0;
		state->joined_y = 0;
		state->joined_angle = 0;
		state->send_packet_serial = 0;
		state->characters.clear();
		state->receive_buffer.clear();
	}

	void ShutdownGameServerBootstrap(GameServerBootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		DisconnectGameServerBootstrap(state, "Conexao GameServer encerrada");
		state->configured = false;
		state->host.clear();
		state->port = 0;
		state->account.clear();
		state->password.clear();
		state->language.clear();
		state->client_version.clear();
		state->client_serial.clear();
	}

	bool ConfigureGameServerBootstrap(
		GameServerBootstrapState* state,
		const char* host,
		unsigned short port,
		const char* account,
		const char* password,
		const char* language,
		const char* client_version,
		const char* client_serial)
	{
		if (state == NULL)
		{
			return false;
		}

		DisconnectGameServerBootstrap(state, NULL);
		state->configured = host != NULL && host[0] != '\0' && port != 0;
		state->host = host != NULL ? host : "";
		state->port = port;
		state->account = account != NULL ? account : "";
		state->password = password != NULL ? password : "";
		state->language = language != NULL ? language : "";
		state->client_version = client_version != NULL ? client_version : "";
		state->client_serial = client_serial != NULL ? client_serial : "";
		state->status = GameServerBootstrapStatus_Idle;
		state->resolved_endpoint.clear();
		state->status_message = state->configured ? ("GameServer pronto " + state->host + ":" + std::to_string(state->port)) : "GameServer invalido";
		state->attempt_count = 0;
		state->connect_started_at_millis = 0;
		state->last_error_code = 0;
		state->join_server_received = false;
		state->join_server_success = false;
		state->join_server_result = 0xFF;
		state->hero_key = 0;
		state->join_server_version.clear();
		state->login_pending = false;
		state->login_requested = false;
		state->login_result_received = false;
		state->login_result = 0xFF;
		state->duplicate_login_disconnect_tried = false;
		state->duplicate_login_disconnect_pending = false;
		state->character_list_requested = false;
		state->character_list_received = false;
		state->character_class_code = 0;
		state->character_move_count = 0;
		state->max_character_count = 0;
		state->map_join_requested = false;
		state->map_join_received = false;
		state->selected_character_slot = 0xFF;
		state->selected_character_name.clear();
		state->joined_map = 0;
		state->joined_x = 0;
		state->joined_y = 0;
		state->joined_angle = 0;
		state->send_packet_serial = 0;
		state->characters.clear();
		state->receive_buffer.clear();
		return state->configured;
	}

	bool StartGameServerBootstrap(GameServerBootstrapState* state)
	{
		if (state == NULL || !state->configured)
		{
			if (state != NULL)
			{
				state->status = GameServerBootstrapStatus_Failed;
				state->status_message = "GameServer nao configurado";
			}
			return false;
		}

		DisconnectGameServerBootstrap(state, NULL);
		state->attempt_count += 1;
		state->connect_started_at_millis = GetTickMilliseconds();
		state->last_error_code = 0;
		state->resolved_endpoint.clear();
		state->join_server_received = false;
		state->join_server_success = false;
		state->join_server_result = 0xFF;
		state->hero_key = 0;
		state->join_server_version.clear();
		state->login_pending = false;
		state->login_requested = false;
		state->login_result_received = false;
		state->login_result = 0xFF;
		state->duplicate_login_disconnect_pending = false;
		state->character_list_requested = false;
		state->character_list_received = false;
		state->character_class_code = 0;
		state->character_move_count = 0;
		state->max_character_count = 0;
		state->map_join_requested = false;
		state->map_join_received = false;
		state->selected_character_slot = 0xFF;
		state->selected_character_name.clear();
		state->joined_map = 0;
		state->joined_x = 0;
		state->joined_y = 0;
		state->joined_angle = 0;
		state->send_packet_serial = 0;
		state->characters.clear();
		state->receive_buffer.clear();

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
			state->status = GameServerBootstrapStatus_Failed;
			state->last_error_code = resolve_error;
			state->resolved_endpoint = endpoint_text;
			state->status_message = std::string("Falha ao resolver GameServer ") + endpoint_text + " (" + FormatResolveError(resolve_error) + ")";
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
					GameServerBootstrapStatus_Connected,
					last_endpoint,
					std::string("Conectado ao GameServer ") + last_endpoint,
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
					GameServerBootstrapStatus_Connecting,
					last_endpoint,
					std::string("Conectando ao GameServer ") + last_endpoint,
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

		void PollGameServerBootstrap(GameServerBootstrapState* state)
		{
			if (state == NULL || !state->socket_open)
		{
			return;
		}

			if (state->status == GameServerBootstrapStatus_JoinReady &&
				state->login_pending &&
				!state->login_requested &&
				!state->login_result_received)
			{
				if (!RequestGameServerLoginBootstrap(state))
				{
					SetFailure(state, GetLastSocketErrorCode(), state->resolved_endpoint);
					return;
				}
			}

			if (state->status == GameServerBootstrapStatus_LoginSucceeded &&
				!state->character_list_requested)
			{
				if (!RequestCharacterList(state))
				{
					SetFailure(state, GetLastSocketErrorCode(), state->resolved_endpoint);
					return;
				}
			}

			if (state->status == GameServerBootstrapStatus_Connected ||
				state->status == GameServerBootstrapStatus_JoinReady ||
				state->status == GameServerBootstrapStatus_LoginSucceeded ||
				state->status == GameServerBootstrapStatus_CharacterListReady ||
				state->status == GameServerBootstrapStatus_MapJoinReady ||
				state->status == GameServerBootstrapStatus_LoginFailed)
			{
				ReceivePackets(state);
			return;
		}

		if (state->status != GameServerBootstrapStatus_Connecting)
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

		state->status = GameServerBootstrapStatus_Connected;
		state->last_error_code = 0;
		state->status_message = std::string("Conectado ao GameServer ") + state->resolved_endpoint;
		ReceivePackets(state);
	}

	bool RequestGameServerLoginBootstrap(GameServerBootstrapState* state)
	{
			if (state == NULL ||
			!state->socket_open ||
			!state->join_server_success ||
			state->account.empty() ||
			state->password.empty())
			{
				return false;
			}

			unsigned char account_buffer[kLoginAccountSize] = { 0 };
			unsigned char password_buffer[kLoginPasswordSize] = { 0 };
			unsigned char serial_buffer[kLoginSerialSize] = { 0 };
			memcpy(account_buffer, state->account.c_str(), std::min(state->account.size(), kLoginAccountSize));
			memcpy(password_buffer, state->password.c_str(), std::min(state->password.size(), kLoginPasswordSize));
			memcpy(serial_buffer, state->client_serial.c_str(), std::min(state->client_serial.size(), kLoginSerialSize));
			BuxConvert(account_buffer, sizeof(account_buffer));
			BuxConvert(password_buffer, sizeof(password_buffer));

			const std::uint32_t tick_value = static_cast<std::uint32_t>(GetTickMilliseconds() & 0xFFFFFFFFu);
			const std::string packed_version = BuildPackedVersionString(state->client_version);

			std::vector<unsigned char> plain_packet;
			plain_packet.reserve(3 + 1 + sizeof(account_buffer) + sizeof(password_buffer) + sizeof(tick_value) + kLoginVersionSize + sizeof(serial_buffer));

			const unsigned char header_code = 0xC1;
			const unsigned char packet_size_placeholder = 0;
			const unsigned char head_code = 0xF1;
			const unsigned char sub_code = 0x01;

			AppendPacketBytes(&plain_packet, &header_code, sizeof(header_code), false);
			AppendPacketBytes(&plain_packet, &packet_size_placeholder, sizeof(packet_size_placeholder), false);
			AppendPacketBytes(&plain_packet, &head_code, sizeof(head_code), false);
			AppendPacketBytes(&plain_packet, &sub_code, sizeof(sub_code), true);
			AppendPacketBytes(&plain_packet, account_buffer, sizeof(account_buffer), true);
			AppendPacketBytes(&plain_packet, password_buffer, sizeof(password_buffer), true);
			AppendPacketBytes(&plain_packet, reinterpret_cast<const unsigned char*>(&tick_value), sizeof(tick_value), true);
			AppendPacketBytes(&plain_packet, reinterpret_cast<const unsigned char*>(packed_version.data()), std::min(packed_version.size(), kLoginVersionSize), true);
			if (packed_version.size() < kLoginVersionSize)
			{
				const unsigned char zero = 0;
				for (size_t index = packed_version.size(); index < kLoginVersionSize; ++index)
				{
					AppendPacketBytes(&plain_packet, &zero, sizeof(zero), true);
				}
			}
			AppendPacketBytes(&plain_packet, serial_buffer, sizeof(serial_buffer), true);

			if (plain_packet.size() > 0xFF)
			{
				state->status = GameServerBootstrapStatus_Failed;
				state->status_message = "Pacote de login excedeu o limite C1";
				return false;
			}

			plain_packet[1] = static_cast<unsigned char>(plain_packet.size());

			unsigned char send_packet[2048] = { 0 };
			std::vector<unsigned char> encrypted_source = plain_packet;
			encrypted_source[1] = state->send_packet_serial++;
			int encrypted_size = gPacketManager.Encrypt(
				&send_packet[2],
				&encrypted_source[1],
				static_cast<int>(encrypted_source.size() - 1));
			if (encrypted_size <= 0)
			{
				state->status = GameServerBootstrapStatus_Failed;
			state->status_message = "Falha ao criptografar pacote de login";
			return false;
			}

			encrypted_size += 2;
			send_packet[0] = 0xC3;
			send_packet[1] = static_cast<unsigned char>(encrypted_size & 0xFF);

		if (!SendAll(FromStateSocket(state), send_packet, static_cast<size_t>(encrypted_size)))
		{
			SetFailure(state, GetLastSocketErrorCode(), state->resolved_endpoint);
			return false;
		}

		state->login_pending = false;
		state->login_requested = true;
		state->status_message = std::string("Login enviado para ") + state->resolved_endpoint;
		return true;
	}

	bool RequestJoinMapServerBootstrap(GameServerBootstrapState* state, const char* character_name)
	{
		if (state == NULL ||
			!state->socket_open ||
			!state->character_list_received ||
			character_name == NULL ||
			character_name[0] == '\0')
		{
			return false;
		}

		for (size_t index = 0; index < state->characters.size(); ++index)
		{
			if (state->characters[index].name == character_name)
			{
				state->selected_character_slot = state->characters[index].slot;
				break;
			}
		}

		return RequestJoinMapServer(state, character_name);
	}

	bool RequestJoinMapServerBySlotBootstrap(GameServerBootstrapState* state, unsigned char character_slot)
	{
		if (state == NULL || !state->character_list_received)
		{
			return false;
		}

		for (size_t index = 0; index < state->characters.size(); ++index)
		{
			if (state->characters[index].slot == character_slot)
			{
				state->selected_character_slot = character_slot;
				return RequestJoinMapServer(state, state->characters[index].name.c_str());
			}
		}

		return false;
	}

	void DisconnectGameServerBootstrap(GameServerBootstrapState* state, const char* reason)
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
		state->status = GameServerBootstrapStatus_Idle;
		state->receive_buffer.clear();
		state->join_server_received = false;
		state->join_server_success = false;
		state->join_server_result = 0xFF;
		state->hero_key = 0;
			state->join_server_version.clear();
			state->login_pending = false;
			state->login_requested = false;
			state->login_result_received = false;
			state->login_result = 0xFF;
			state->duplicate_login_disconnect_pending = false;
			state->character_list_requested = false;
			state->character_list_received = false;
			state->character_class_code = 0;
			state->character_move_count = 0;
			state->max_character_count = 0;
			state->map_join_requested = false;
			state->map_join_received = false;
			state->selected_character_slot = 0xFF;
			state->selected_character_name.clear();
			state->joined_map = 0;
			state->joined_x = 0;
			state->joined_y = 0;
			state->joined_angle = 0;
			state->send_packet_serial = 0;
			state->characters.clear();
			if (reason != NULL && reason[0] != '\0')
			{
				state->status_message = reason;
		}
	}

	const char* GetGameServerBootstrapStatusLabel(GameServerBootstrapStatus status)
	{
		switch (status)
		{
		case GameServerBootstrapStatus_Idle:
			return "idle";
		case GameServerBootstrapStatus_Connecting:
			return "connecting";
		case GameServerBootstrapStatus_Connected:
			return "connected";
			case GameServerBootstrapStatus_JoinReady:
				return "join_ready";
			case GameServerBootstrapStatus_LoginSucceeded:
				return "login_ok";
			case GameServerBootstrapStatus_CharacterListReady:
				return "char_list";
			case GameServerBootstrapStatus_MapJoinReady:
				return "map_join";
			case GameServerBootstrapStatus_LoginFailed:
				return "login_failed";
		case GameServerBootstrapStatus_Failed:
			return "failed";
		default:
			return "unknown";
		}
	}
}

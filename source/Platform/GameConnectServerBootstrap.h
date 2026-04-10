#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace platform
{
	struct ConnectServerEntry
	{
		unsigned short connect_index;
		unsigned char load_percent;
	};

	enum ConnectServerBootstrapStatus
	{
		ConnectServerBootstrapStatus_Idle = 0,
		ConnectServerBootstrapStatus_Connecting,
		ConnectServerBootstrapStatus_Connected,
		ConnectServerBootstrapStatus_Failed,
	};

	struct ConnectServerBootstrapState
	{
		bool configured;
		std::string host;
		unsigned short port;
		ConnectServerBootstrapStatus status;
		std::string resolved_endpoint;
		std::string status_message;
		unsigned int attempt_count;
		std::uint64_t connect_started_at_millis;
		std::uint32_t connect_timeout_millis;
		int last_error_code;
		intptr_t socket_handle;
		bool socket_open;
		bool server_list_requested;
		bool server_list_received;
		bool server_address_requested;
		bool server_address_received;
		unsigned short requested_server_index;
		unsigned short total_server_count;
		std::vector<unsigned char> receive_buffer;
		std::vector<ConnectServerEntry> server_entries;
		std::string selected_game_server_host;
		unsigned short selected_game_server_port;
	};

	void InitializeConnectServerBootstrapState(ConnectServerBootstrapState* state);
	void ShutdownConnectServerBootstrap(ConnectServerBootstrapState* state);
	bool ConfigureConnectServerBootstrap(ConnectServerBootstrapState* state, const char* host, unsigned short port);
	bool StartConnectServerBootstrap(ConnectServerBootstrapState* state);
	void PollConnectServerBootstrap(ConnectServerBootstrapState* state);
	bool RequestConnectServerListBootstrap(ConnectServerBootstrapState* state);
	bool RequestConnectServerAddressBootstrap(ConnectServerBootstrapState* state, unsigned short connect_index);
	void DisconnectConnectServerBootstrap(ConnectServerBootstrapState* state, const char* reason);
	const char* GetConnectServerBootstrapStatusLabel(ConnectServerBootstrapStatus status);
}

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace platform
{
	enum GameServerBootstrapStatus
	{
		GameServerBootstrapStatus_Idle = 0,
		GameServerBootstrapStatus_Connecting,
		GameServerBootstrapStatus_Connected,
		GameServerBootstrapStatus_JoinReady,
		GameServerBootstrapStatus_LoginSucceeded,
		GameServerBootstrapStatus_CharacterListReady,
		GameServerBootstrapStatus_MapJoinReady,
		GameServerBootstrapStatus_LoginFailed,
		GameServerBootstrapStatus_Failed,
	};

	struct GameServerCharacterEntry
	{
		std::uint8_t slot;
		std::string name;
		std::uint16_t level;
		std::uint8_t ctl_code;
		std::uint8_t class_code;
		std::uint8_t guild_status;
		std::uint8_t char_set[18];
	};

	struct GameServerBootstrapState
	{
		bool configured;
		std::string host;
		unsigned short port;
		std::string account;
		std::string password;
		std::string language;
		std::string client_version;
		std::string client_serial;
		GameServerBootstrapStatus status;
		std::string resolved_endpoint;
		std::string status_message;
		unsigned int attempt_count;
		std::uint64_t connect_started_at_millis;
		std::uint32_t connect_timeout_millis;
		int last_error_code;
		intptr_t socket_handle;
		bool socket_open;
		bool join_server_received;
		bool join_server_success;
		unsigned char join_server_result;
		unsigned short hero_key;
		std::string join_server_version;
		bool login_pending;
		bool login_requested;
		bool login_result_received;
		unsigned char login_result;
		bool duplicate_login_disconnect_tried;
		bool duplicate_login_disconnect_pending;
		bool character_list_requested;
		bool character_list_received;
		unsigned char character_class_code;
		unsigned char character_move_count;
		unsigned char max_character_count;
		bool create_character_pending;
		unsigned char create_character_result;
		bool delete_character_pending;
		unsigned char delete_character_result;
		bool map_join_requested;
		bool map_join_received;
		unsigned char selected_character_slot;
		std::string selected_character_name;
		unsigned char joined_map;
		unsigned char joined_x;
		unsigned char joined_y;
		unsigned char joined_angle;
		unsigned char send_packet_serial;
		std::vector<GameServerCharacterEntry> characters;
		std::vector<unsigned char> receive_buffer;
	};

	void InitializeGameServerBootstrapState(GameServerBootstrapState* state);
	void ShutdownGameServerBootstrap(GameServerBootstrapState* state);
	bool ConfigureGameServerBootstrap(
		GameServerBootstrapState* state,
		const char* host,
		unsigned short port,
		const char* account,
		const char* password,
		const char* language,
		const char* client_version,
		const char* client_serial);
	bool StartGameServerBootstrap(GameServerBootstrapState* state);
	void PollGameServerBootstrap(GameServerBootstrapState* state);
	bool RequestGameServerLoginBootstrap(GameServerBootstrapState* state);
	bool RequestJoinMapServerBootstrap(GameServerBootstrapState* state, const char* character_name);
	bool RequestJoinMapServerBySlotBootstrap(GameServerBootstrapState* state, unsigned char character_slot);
	bool RequestCreateCharacterBootstrap(GameServerBootstrapState* state, const char* name, unsigned char character_class, unsigned char skin);
	bool RequestDeleteCharacterBootstrap(GameServerBootstrapState* state, unsigned char character_slot, const char* resident_id);
	unsigned char ConsumeCreateCharacterResult(GameServerBootstrapState* state);
	unsigned char ConsumeDeleteCharacterResult(GameServerBootstrapState* state);
	void DisconnectGameServerBootstrap(GameServerBootstrapState* state, const char* reason);
	const char* GetGameServerBootstrapStatusLabel(GameServerBootstrapStatus status);
}

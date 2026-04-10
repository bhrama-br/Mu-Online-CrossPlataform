#pragma once

#include <string>

namespace platform
{
	struct ClientRuntimeConfigState
	{
		bool found;
		bool used_defaults;
		bool has_auto_login_user_override;
		bool has_auto_login_password_override;
		std::string source_description;
		std::string player_id;
		int sound_on;
		int music_on;
		int resolution;
		int color_depth;
		int render_text_type;
		int chat_input_type;
		bool window_mode;
		std::string language;
		std::string auto_login_user;
		std::string auto_login_password;
		std::string connect_server_host;
		unsigned short connect_server_port;
		std::string game_server_host;
		unsigned short game_server_port;
		unsigned int window_width;
		unsigned int window_height;
		float screen_rate_x;
		float screen_rate_y;
	};

	bool InitializeClientRuntimeConfig(const char* config_path, ClientRuntimeConfigState* out_state);
}

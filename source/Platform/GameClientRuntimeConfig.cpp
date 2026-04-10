#include "Platform/GameClientRuntimeConfig.h"

#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace platform
{
	namespace
	{
		void ResetState(ClientRuntimeConfigState* state)
		{
			if (state == NULL)
			{
				return;
			}

			state->found = false;
			state->used_defaults = true;
			state->has_auto_login_user_override = false;
			state->has_auto_login_password_override = false;
			state->source_description.clear();
			state->player_id.clear();
			state->sound_on = 1;
			state->music_on = 0;
			state->resolution = 1;
			state->color_depth = 0;
			state->render_text_type = 0;
			state->chat_input_type = 1;
			state->window_mode = false;
			state->language = "Eng";
			state->auto_login_user.clear();
			state->auto_login_password.clear();
			state->connect_server_host.clear();
			state->connect_server_port = 0;
			state->game_server_host.clear();
			state->game_server_port = 0;
			state->window_width = 800;
			state->window_height = 600;
			state->screen_rate_x = 1.25f;
			state->screen_rate_y = 1.25f;
		}

		std::string TrimString(const std::string& input)
		{
			size_t begin = 0;
			while (begin < input.size() && (input[begin] == ' ' || input[begin] == '\t' || input[begin] == '\r' || input[begin] == '\n'))
			{
				++begin;
			}

			size_t end = input.size();
			while (end > begin && (input[end - 1] == ' ' || input[end - 1] == '\t' || input[end - 1] == '\r' || input[end - 1] == '\n'))
			{
				--end;
			}

			return input.substr(begin, end - begin);
		}

		bool FileExists(const char* file_path)
		{
			if (file_path == NULL || file_path[0] == '\0')
			{
				return false;
			}

			FILE* file = fopen(file_path, "rb");
			if (file == NULL)
			{
				return false;
			}

			fclose(file);
			return true;
		}

		int ParseInt(const std::string& value, int fallback_value)
		{
			if (value.empty())
			{
				return fallback_value;
			}

			char* end_ptr = NULL;
			const long parsed = strtol(value.c_str(), &end_ptr, 10);
			if (end_ptr == value.c_str())
			{
				return fallback_value;
			}

			return static_cast<int>(parsed);
		}

		bool ParseBool(const std::string& value, bool fallback_value)
		{
			if (value.empty())
			{
				return fallback_value;
			}

			const int parsed = ParseInt(value, fallback_value ? 1 : 0);
			return parsed != 0;
		}

		void ApplyResolutionMetrics(ClientRuntimeConfigState* state)
		{
			if (state == NULL)
			{
				return;
			}

			if (state->resolution < 0)
			{
				state->resolution = 1;
			}

			switch (state->resolution)
			{
			case 0: state->window_width = 640; state->window_height = 480; break;
			case 1: state->window_width = 800; state->window_height = 600; break;
			case 2: state->window_width = 1024; state->window_height = 768; break;
			case 3: state->window_width = 1280; state->window_height = 960; break;
			case 4: state->window_width = 1360; state->window_height = 768; break;
			case 5: state->window_width = 1440; state->window_height = 900; break;
			case 6: state->window_width = 1600; state->window_height = 900; break;
			case 7: state->window_width = 1680; state->window_height = 1050; break;
			case 8: state->window_width = 1920; state->window_height = 1080; break;
			default:
				state->resolution = 1;
				state->window_width = 800;
				state->window_height = 600;
				break;
			}

			if (state->resolution > 3)
			{
				state->screen_rate_x = 1.6f;
				state->screen_rate_y = 1.6f;
			}
			else
			{
				const float rate = static_cast<float>(state->window_height) / 480.0f;
				state->screen_rate_x = rate;
				state->screen_rate_y = rate;
			}
		}

		void ApplyIniValues(const std::map<std::string, std::map<std::string, std::string> >& values, ClientRuntimeConfigState* state)
		{
			if (state == NULL)
			{
				return;
			}

			std::map<std::string, std::string> runtime_values;
			const std::map<std::string, std::map<std::string, std::string> >::const_iterator explicit_runtime = values.find("Runtime");
			if (explicit_runtime != values.end())
			{
				runtime_values = explicit_runtime->second;
			}
			const std::map<std::string, std::map<std::string, std::string> >::const_iterator root_runtime = values.find("");
			if (root_runtime != values.end())
			{
				for (std::map<std::string, std::string>::const_iterator it = root_runtime->second.begin(); it != root_runtime->second.end(); ++it)
				{
					runtime_values[it->first] = it->second;
				}
			}

			if (runtime_values.find("ID") != runtime_values.end())
			{
				state->player_id = runtime_values["ID"];
			}
			state->sound_on = ParseInt(runtime_values["SoundOnOff"], state->sound_on);
			state->music_on = ParseInt(runtime_values["MusicOnOff"], state->music_on);
			state->resolution = ParseInt(runtime_values["Resolution"], state->resolution);
			state->color_depth = ParseInt(runtime_values["ColorDepth"], state->color_depth);
			state->render_text_type = ParseInt(runtime_values["TextOut"], state->render_text_type);
			state->chat_input_type = ParseInt(runtime_values["ChatInputType"], state->chat_input_type);
			state->window_mode = ParseBool(runtime_values["WindowMode"], state->window_mode);
			if (runtime_values.find("LangSelection") != runtime_values.end() && !runtime_values["LangSelection"].empty())
			{
				state->language = runtime_values["LangSelection"];
			}
			const std::map<std::string, std::string>::const_iterator auto_login_user_it = runtime_values.find("AutoLoginUser");
			const std::map<std::string, std::string>::const_iterator login_user_it = runtime_values.find("LoginUser");
			if (auto_login_user_it != runtime_values.end())
			{
				state->has_auto_login_user_override = true;
				state->auto_login_user = auto_login_user_it->second;
			}
			else if (login_user_it != runtime_values.end())
			{
				state->has_auto_login_user_override = true;
				state->auto_login_user = login_user_it->second;
			}

			const std::map<std::string, std::string>::const_iterator auto_login_password_it = runtime_values.find("AutoLoginPassword");
			const std::map<std::string, std::string>::const_iterator login_password_it = runtime_values.find("LoginPassword");
			if (auto_login_password_it != runtime_values.end())
			{
				state->has_auto_login_password_override = true;
				state->auto_login_password = auto_login_password_it->second;
			}
			else if (login_password_it != runtime_values.end())
			{
				state->has_auto_login_password_override = true;
				state->auto_login_password = login_password_it->second;
			}

			if (runtime_values.find("ConnectServerHost") != runtime_values.end() && !runtime_values["ConnectServerHost"].empty())
			{
				state->connect_server_host = runtime_values["ConnectServerHost"];
			}
			else if (runtime_values.find("ConnectServerIp") != runtime_values.end() && !runtime_values["ConnectServerIp"].empty())
			{
				state->connect_server_host = runtime_values["ConnectServerIp"];
			}
			else if (runtime_values.find("ConnectServerIP") != runtime_values.end() && !runtime_values["ConnectServerIP"].empty())
			{
				state->connect_server_host = runtime_values["ConnectServerIP"];
			}

			const int parsed_connect_port = ParseInt(runtime_values["ConnectServerPort"], state->connect_server_port);
			if (parsed_connect_port > 0 && parsed_connect_port <= 65535)
			{
				state->connect_server_port = static_cast<unsigned short>(parsed_connect_port);
			}

			if (runtime_values.find("GameServerHost") != runtime_values.end() && !runtime_values["GameServerHost"].empty())
			{
				state->game_server_host = runtime_values["GameServerHost"];
			}
			else if (runtime_values.find("GameServerIp") != runtime_values.end() && !runtime_values["GameServerIp"].empty())
			{
				state->game_server_host = runtime_values["GameServerIp"];
			}
			else if (runtime_values.find("GameServerIP") != runtime_values.end() && !runtime_values["GameServerIP"].empty())
			{
				state->game_server_host = runtime_values["GameServerIP"];
			}

			const int parsed_game_port = ParseInt(runtime_values["GameServerPort"], state->game_server_port);
			if (parsed_game_port > 0 && parsed_game_port <= 65535)
			{
				state->game_server_port = static_cast<unsigned short>(parsed_game_port);
			}
		}

		bool LoadFromIniFile(const char* config_path, ClientRuntimeConfigState* state)
		{
			if (state == NULL || config_path == NULL || config_path[0] == '\0' || !FileExists(config_path))
			{
				return false;
			}

			FILE* file = fopen(config_path, "rb");
			if (file == NULL)
			{
				return false;
			}

			std::string current_section;
			std::map<std::string, std::map<std::string, std::string> > values;
			char line_buffer[512];
			while (fgets(line_buffer, sizeof(line_buffer), file) != NULL)
			{
				std::string line = TrimString(line_buffer);
				if (line.empty() || line[0] == ';' || line[0] == '#')
				{
					continue;
				}

				if (line.size() >= 2 && line[0] == '[' && line[line.size() - 1] == ']')
				{
					current_section = TrimString(line.substr(1, line.size() - 2));
					continue;
				}

				const size_t equals = line.find('=');
				if (equals == std::string::npos)
				{
					continue;
				}

				const std::string key = TrimString(line.substr(0, equals));
				const std::string value = TrimString(line.substr(equals + 1));
				values[current_section][key] = value;
			}

			fclose(file);

			ApplyIniValues(values, state);
			state->found = true;
			state->used_defaults = false;
			state->source_description = config_path;
			return true;
		}

#if defined(_WIN32)
		bool ReadRegistryString(HKEY key, const char* value_name, std::string* out_value)
		{
			if (out_value == NULL)
			{
				return false;
			}

			char buffer[64] = { 0 };
			DWORD size = sizeof(buffer);
			if (RegQueryValueExA(key, value_name, 0, NULL, reinterpret_cast<LPBYTE>(buffer), &size) != ERROR_SUCCESS)
			{
				return false;
			}

			out_value->assign(buffer);
			return true;
		}

		bool ReadRegistryInt(HKEY key, const char* value_name, int* out_value)
		{
			if (out_value == NULL)
			{
				return false;
			}

			DWORD value = 0;
			DWORD size = sizeof(value);
			if (RegQueryValueExA(key, value_name, 0, NULL, reinterpret_cast<LPBYTE>(&value), &size) != ERROR_SUCCESS)
			{
				return false;
			}

			*out_value = static_cast<int>(value);
			return true;
		}

		bool LoadFromRegistry(ClientRuntimeConfigState* state)
		{
			if (state == NULL)
			{
				return false;
			}

			HKEY key = NULL;
			DWORD disposition = 0;
			if (RegCreateKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Webzen\\Mu\\Config", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &disposition) != ERROR_SUCCESS)
			{
				return false;
			}

			std::string value_string;
			int value_int = 0;
			if (ReadRegistryString(key, "ID", &value_string))
			{
				state->player_id = value_string;
			}
			if (ReadRegistryInt(key, "SoundOnOff", &value_int))
			{
				state->sound_on = value_int;
			}
			if (ReadRegistryInt(key, "MusicOnOff", &value_int))
			{
				state->music_on = value_int;
			}
			if (ReadRegistryInt(key, "Resolution", &value_int))
			{
				state->resolution = value_int;
			}
			if (ReadRegistryInt(key, "ColorDepth", &value_int))
			{
				state->color_depth = value_int;
			}
			if (ReadRegistryInt(key, "TextOut", &value_int))
			{
				state->render_text_type = value_int;
			}
			if (ReadRegistryInt(key, "WindowMode", &value_int))
			{
				state->window_mode = (value_int != 0);
			}
			if (ReadRegistryString(key, "LangSelection", &value_string) && !value_string.empty())
			{
				state->language = value_string;
			}

			RegCloseKey(key);
			state->found = true;
			state->used_defaults = false;
			state->source_description = "registry:HKCU\\SOFTWARE\\Webzen\\Mu\\Config";
			return true;
		}
#endif
	}

	bool InitializeClientRuntimeConfig(const char* config_path, ClientRuntimeConfigState* out_state)
	{
		ResetState(out_state);
		if (out_state == NULL)
		{
			return false;
		}

#if defined(_WIN32)
		bool loaded = false;
		if (config_path != NULL && config_path[0] != '\0')
		{
			loaded = LoadFromIniFile(config_path, out_state);
		}
		if (!loaded)
		{
			loaded = LoadFromRegistry(out_state);
		}
#else
		const bool loaded = LoadFromIniFile(config_path, out_state);
#endif

		ApplyResolutionMetrics(out_state);
		if (!loaded && out_state->source_description.empty())
		{
			out_state->source_description = config_path != NULL && config_path[0] != '\0' ? config_path : "defaults";
		}

		return loaded;
	}
}

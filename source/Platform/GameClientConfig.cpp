#include "Platform/GameClientConfig.h"

#include <cstdio>
#include <map>
#include <string>

namespace platform
{
	namespace
	{
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
	}

	bool LoadClientIniConfig(const char* config_path, ClientIniConfigState* out_state)
	{
		if (out_state != NULL)
		{
			out_state->found = false;
			out_state->config_path = config_path != NULL ? config_path : "";
			out_state->login_version.clear();
			out_state->test_version.clear();
			out_state->auto_login_user.clear();
			out_state->auto_login_password.clear();
		}

		if (config_path == NULL || config_path[0] == '\0')
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

		if (out_state != NULL)
		{
			out_state->found = true;
			out_state->login_version = values["LOGIN"]["Version"];
			out_state->test_version = values["LOGIN"]["TestVersion"];
			out_state->auto_login_user = values["AutoLogin"]["User"];
			out_state->auto_login_password = values["AutoLogin"]["Password"];
		}

		return true;
	}
}

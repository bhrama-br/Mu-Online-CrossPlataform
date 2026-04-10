#pragma once

#include <string>

namespace platform
{
	struct ClientIniConfigState
	{
		bool found;
		std::string config_path;
		std::string login_version;
		std::string test_version;
		std::string auto_login_user;
		std::string auto_login_password;
	};

	bool LoadClientIniConfig(const char* config_path, ClientIniConfigState* out_state);
}

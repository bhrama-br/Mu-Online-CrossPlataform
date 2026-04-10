#pragma once

#include <string>

#include "Protect.h"

namespace platform
{
	enum CoreGameBootstrapError
	{
		CoreGameBootstrapError_None = 0,
		CoreGameBootstrapError_InvalidProtectObject,
		CoreGameBootstrapError_MissingCoreDataFile,
		CoreGameBootstrapError_ReadMainConfigFailed,
		CoreGameBootstrapError_ReadCustomJewelFailed,
	};

	struct CoreGameBootstrapState
	{
		CoreGameBootstrapError error;
		std::string error_message;
		std::string asset_root;
		std::string main_file_path;
		std::string custom_jewel_path;
		std::string enc1_path;
		std::string dec2_path;
		std::string window_name;
		std::string ip_address;
		std::string client_version;
		std::string client_serial;
		unsigned short ip_address_port;
		unsigned long main_file_crc;
	};

	bool InitializeCoreGameProtect(CProtect* protect, CoreGameBootstrapState* out_state);
}

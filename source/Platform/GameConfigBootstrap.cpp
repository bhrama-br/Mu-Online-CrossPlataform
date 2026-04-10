#include "Platform/GameConfigBootstrap.h"

#include <cstdio>
#include <cstring>

#include "Platform/GameAssetPath.h"

namespace platform
{
	namespace
	{
		bool FileExists(const std::string& path)
		{
			if (path.empty())
			{
				return false;
			}

			FILE* file = fopen(path.c_str(), "rb");
			if (file == NULL)
			{
				return false;
			}

			fclose(file);
			return true;
		}

		void ResetState(CoreGameBootstrapState* state)
		{
			if (state == NULL)
			{
				return;
			}

			state->error = CoreGameBootstrapError_None;
			state->error_message.clear();
			state->asset_root.clear();
			state->main_file_path.clear();
			state->custom_jewel_path.clear();
			state->enc1_path.clear();
			state->dec2_path.clear();
			state->window_name.clear();
			state->ip_address.clear();
			state->client_version.clear();
			state->client_serial.clear();
			state->ip_address_port = 0;
			state->main_file_crc = 0;
		}

		void SetError(CoreGameBootstrapState* state, CoreGameBootstrapError error, const char* message)
		{
			if (state == NULL)
			{
				return;
			}

			state->error = error;
			state->error_message = message != NULL ? message : "";
		}

		std::string ReadFixedString(const char* buffer, size_t size)
		{
			if (buffer == NULL || size == 0)
			{
				return std::string();
			}

			size_t length = 0;
			while (length < size && buffer[length] != '\0')
			{
				++length;
			}

			return std::string(buffer, length);
		}
	}

	bool InitializeCoreGameProtect(CProtect* protect, CoreGameBootstrapState* out_state)
	{
		ResetState(out_state);

		if (protect == NULL)
		{
			SetError(out_state, CoreGameBootstrapError_InvalidProtectObject, "Invalid protect object.");
			return false;
		}

		InitializeGameAssetRoot();

		if (out_state != NULL)
		{
			out_state->asset_root = GetGameAssetRoot();
			out_state->main_file_path = ResolveGameAssetPath("Data\\Configs\\Configs.xtm");
			out_state->custom_jewel_path = ResolveGameAssetPath("Data\\Configs\\CustomJewel.xtm");
			out_state->enc1_path = ResolveGameAssetPath("Data\\Enc1.dat");
			out_state->dec2_path = ResolveGameAssetPath("Data\\Dec2.dat");
		}

		const std::string main_file_path = ResolveGameAssetPath("Data\\Configs\\Configs.xtm");
		const std::string custom_jewel_path = ResolveGameAssetPath("Data\\Configs\\CustomJewel.xtm");
		const std::string enc1_path = ResolveGameAssetPath("Data\\Enc1.dat");
		const std::string dec2_path = ResolveGameAssetPath("Data\\Dec2.dat");

		if (!FileExists(main_file_path) || !FileExists(custom_jewel_path) || !FileExists(enc1_path) || !FileExists(dec2_path))
		{
			SetError(out_state, CoreGameBootstrapError_MissingCoreDataFile, "Missing core game data files in the resolved Data root.");
			return false;
		}

		if (protect->ReadMainFile(main_file_path.c_str()) == 0)
		{
			SetError(out_state, CoreGameBootstrapError_ReadMainConfigFailed, "Failed to read Configs.xtm.");
			return false;
		}

		if (protect->ReadCustomJewelConfig(custom_jewel_path.c_str()) == 0)
		{
			SetError(out_state, CoreGameBootstrapError_ReadCustomJewelFailed, "Failed to read CustomJewel.xtm.");
			return false;
		}

		if (out_state != NULL)
		{
			out_state->window_name = ReadFixedString(protect->m_MainInfo.WindowName, sizeof(protect->m_MainInfo.WindowName));
			out_state->ip_address = ReadFixedString(protect->m_MainInfo.IpAddress, sizeof(protect->m_MainInfo.IpAddress));
			out_state->client_version = ReadFixedString(protect->m_MainInfo.ClientVersion, sizeof(protect->m_MainInfo.ClientVersion));
			out_state->client_serial = ReadFixedString(protect->m_MainInfo.ClientSerial, sizeof(protect->m_MainInfo.ClientSerial));
			out_state->ip_address_port = protect->m_MainInfo.IpAddressPort;
			out_state->main_file_crc = protect->m_ClientFileCRC;
		}

		return true;
	}
}

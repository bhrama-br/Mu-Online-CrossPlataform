#pragma once

#include <string>

namespace platform
{
	struct PacketCryptoBootstrapState
	{
		bool encryption_key_loaded;
		bool decryption_key_loaded;
		bool roundtrip_ok;
		int encrypted_size;
		int decrypted_size;
		std::string enc1_path;
		std::string dec2_path;
		std::string error_message;
	};

	bool InitializePacketCryptoBootstrap(const char* enc1_path, const char* dec2_path, PacketCryptoBootstrapState* out_state);
}

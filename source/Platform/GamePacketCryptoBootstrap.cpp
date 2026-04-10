#include "Platform/GamePacketCryptoBootstrap.h"

#include <cstring>

#include "PacketManager.h"

namespace platform
{
	namespace
	{
		const int kPacketPlainBlockSize = 8;
		const int kPacketEncryptedBlockSize = 11;

		void ResetState(PacketCryptoBootstrapState* state)
		{
			if (state == NULL)
			{
				return;
			}

			state->encryption_key_loaded = false;
			state->decryption_key_loaded = false;
			state->roundtrip_ok = false;
			state->encrypted_size = 0;
			state->decrypted_size = 0;
			state->enc1_path.clear();
			state->dec2_path.clear();
			state->error_message.clear();
		}

		void SetError(PacketCryptoBootstrapState* state, const char* message)
		{
			if (state == NULL)
			{
				return;
			}

			state->error_message = message != NULL ? message : "";
		}
	}

	bool InitializePacketCryptoBootstrap(const char* enc1_path, const char* dec2_path, PacketCryptoBootstrapState* out_state)
	{
		ResetState(out_state);

		if (out_state != NULL)
		{
			out_state->enc1_path = enc1_path != NULL ? enc1_path : "";
			out_state->dec2_path = dec2_path != NULL ? dec2_path : "";
		}

		if (enc1_path == NULL || dec2_path == NULL || enc1_path[0] == '\0' || dec2_path[0] == '\0')
		{
			SetError(out_state, "Invalid packet crypto key paths.");
			return false;
		}

		gPacketManager.Init();
		const bool encryption_key_loaded = gPacketManager.LoadEncryptionKey(enc1_path);
		const bool decryption_key_loaded = gPacketManager.LoadDecryptionKey(dec2_path);

		if (out_state != NULL)
		{
			out_state->encryption_key_loaded = encryption_key_loaded;
			out_state->decryption_key_loaded = decryption_key_loaded;
		}

		if (!encryption_key_loaded || !decryption_key_loaded)
		{
			SetError(out_state, "Failed to load packet encryption/decryption keys.");
			return false;
		}

		// Enc1.dat and Dec2.dat are directional keys. They are both needed by the client,
		// but they are not a valid encrypt/decrypt roundtrip pair for the same sample buffer.
		BYTE plain_block[kPacketPlainBlockSize] = { 0x01, 0x7F, 0x02, 0x80, 0x11, 0x22, 0x33, 0x44 };
		BYTE encrypted_block[kPacketEncryptedBlockSize] = { 0 };
		const int encrypted_size = gPacketManager.Encrypt(encrypted_block, plain_block, kPacketPlainBlockSize);
		const bool encryption_smoke_ok = (encrypted_size == kPacketEncryptedBlockSize);

		if (out_state != NULL)
		{
			out_state->encrypted_size = encrypted_size;
			out_state->decrypted_size = 0;
			out_state->roundtrip_ok = encryption_smoke_ok;
		}

		if (!encryption_smoke_ok)
		{
			SetError(out_state, "Packet encryption smoke validation failed.");
			return false;
		}

		return true;
	}
}

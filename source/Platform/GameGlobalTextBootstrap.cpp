#include "Platform/GameGlobalTextBootstrap.h"

#include <cstdio>
#include <vector>

namespace platform
{
	namespace
	{
#pragma pack(push, 1)
		struct GlobalTextHeader
		{
			unsigned short signature;
			unsigned int number_of_text;
		};

		struct GlobalTextStringHeader
		{
			unsigned int key;
			unsigned int size_of_string;
		};
#pragma pack(pop)

		void ResetState(GlobalTextBootstrapState* state)
		{
			if (state == NULL)
			{
				return;
			}

			state->loaded = false;
			state->source_path.clear();
			state->loaded_entry_count = 0;
			state->entries.clear();
			state->error_message.clear();
		}

		void DecodeBuxBuffer(std::vector<unsigned char>* buffer)
		{
			if (buffer == NULL)
			{
				return;
			}

			static const unsigned char bux_code[3] = { 0xfc, 0xcf, 0xab };
			for (size_t index = 0; index < buffer->size(); ++index)
			{
				(*buffer)[index] ^= bux_code[index % 3];
			}
		}

		std::string Latin1ToUtf8(const std::vector<unsigned char>& bytes)
		{
			std::string utf8;
			utf8.reserve(bytes.size());
			for (size_t index = 0; index < bytes.size(); ++index)
			{
				const unsigned char value = bytes[index];
				if (value == 0)
				{
					break;
				}

				if (value < 0x80)
				{
					utf8.push_back(static_cast<char>(value));
				}
				else
				{
					utf8.push_back(static_cast<char>(0xc0 | (value >> 6)));
					utf8.push_back(static_cast<char>(0x80 | (value & 0x3f)));
				}
			}

			return utf8;
		}
	}

	bool InitializeGlobalTextBootstrap(const char* text_bmd_path, GlobalTextBootstrapState* out_state)
	{
		ResetState(out_state);
		if (out_state == NULL)
		{
			return false;
		}

		if (text_bmd_path == NULL || text_bmd_path[0] == '\0')
		{
			out_state->error_message = "Missing Text_*.bmd path";
			return false;
		}

		out_state->source_path = text_bmd_path;

		FILE* file = fopen(text_bmd_path, "rb");
		if (file == NULL)
		{
			out_state->error_message = "Unable to open Text_*.bmd";
			return false;
		}

		GlobalTextHeader header = {};
		if (fread(&header, sizeof(header), 1, file) != 1)
		{
			fclose(file);
			out_state->error_message = "Unable to read global text header";
			return false;
		}

		if (header.signature != 0x5447)
		{
			fclose(file);
			out_state->error_message = "Invalid global text signature";
			return false;
		}

		for (unsigned int index = 0; index < header.number_of_text; ++index)
		{
			GlobalTextStringHeader string_header = {};
			if (fread(&string_header, sizeof(string_header), 1, file) != 1)
			{
				fclose(file);
				out_state->entries.clear();
				out_state->loaded_entry_count = 0;
				out_state->error_message = "Unable to read global text entry header";
				return false;
			}

			std::vector<unsigned char> encoded_text(string_header.size_of_string);
			if (!encoded_text.empty() &&
				fread(&encoded_text[0], sizeof(unsigned char), string_header.size_of_string, file) != string_header.size_of_string)
			{
				fclose(file);
				out_state->entries.clear();
				out_state->loaded_entry_count = 0;
				out_state->error_message = "Unable to read global text entry payload";
				return false;
			}

			DecodeBuxBuffer(&encoded_text);
			out_state->entries[static_cast<int>(string_header.key)] = Latin1ToUtf8(encoded_text);
		}

		fclose(file);

		out_state->loaded = true;
		out_state->loaded_entry_count = out_state->entries.size();
		return true;
	}

	const char* FindGlobalTextEntry(const GlobalTextBootstrapState* state, int key)
	{
		static const char* kEmpty = "";
		if (state == NULL)
		{
			return kEmpty;
		}

		const std::map<int, std::string>::const_iterator it = state->entries.find(key);
		if (it == state->entries.end())
		{
			return kEmpty;
		}

		return it->second.c_str();
	}
}

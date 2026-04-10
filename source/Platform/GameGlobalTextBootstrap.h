#pragma once

#include <map>
#include <string>

namespace platform
{
	struct GlobalTextBootstrapState
	{
		bool loaded;
		std::string source_path;
		size_t loaded_entry_count;
		std::map<int, std::string> entries;
		std::string error_message;
	};

	bool InitializeGlobalTextBootstrap(const char* text_bmd_path, GlobalTextBootstrapState* out_state);
	const char* FindGlobalTextEntry(const GlobalTextBootstrapState* state, int key);
}

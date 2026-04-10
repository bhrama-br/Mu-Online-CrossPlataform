#pragma once

#if defined(__ANDROID__)

#include <string>
#include <vector>

#include "Platform/GameConnectServerBootstrap.h"
#include "Platform/GameGlobalTextBootstrap.h"

namespace platform
{
	struct LegacyLoginServerGroupState
	{
		unsigned short group_index;
		int sequence;
		int position;
		int button_position;
		bool pvp_enabled;
		size_t room_count;
		std::string name;
		std::string description;
	};

	struct LegacyLoginServerEntryState
	{
		unsigned short connect_index;
		int display_index;
		int load_percent;
		unsigned char non_pvp;
		std::string label;
	};

	bool InitializeLegacyLoginServerListRuntime(const GlobalTextBootstrapState* global_text_state, std::string* out_error_message);
	void SyncLegacyLoginServerListRuntime(const ConnectServerBootstrapState* connect_server_state);
	bool CollectLegacyLoginServerGroups(std::vector<LegacyLoginServerGroupState>* out_groups);
	void CollectLegacyLoginServerEntries(int group_index, std::vector<LegacyLoginServerEntryState>* out_entries);
	bool TrySelectLegacyLoginServerEntry(int group_index, int room_slot, unsigned short* out_connect_index);
	int GetLegacyLoginPrimaryGroupIndex();
	const char* GetLegacyLoginPrimaryGroupLabel();
	const char* GetLegacyLoginSelectedServerName();
}

#endif

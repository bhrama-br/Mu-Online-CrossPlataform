#include "stdafx.h"

#if defined(__ANDROID__)

#include "Platform/LegacyLoginServerListRuntime.h"

#include "GlobalText.h"
#include "GameCensorship.h"
#include "Platform/GameAssetPath.h"
#include "ServerGroup.h"
#include "ServerInfo.h"
#include "ServerListManager.h"

#include <android/log.h>
#include <cstdio>

namespace
{
	const char* kLegacyLoginRuntimeLogTag = "MULegacyLoginRT";

	bool g_legacy_login_server_list_ready = false;
	std::string g_legacy_login_server_list_path;

	CServerGroup* FindServerGroupByIndex(int group_index)
	{
		type_mapServerGroup::iterator it = g_ServerListManager->m_mapServerGroup.begin();
		for (; it != g_ServerListManager->m_mapServerGroup.end(); ++it)
		{
			CServerGroup* group = it->second;
			if (group != NULL && group->m_iServerIndex == group_index)
			{
				return group;
			}
		}

		return NULL;
	}

	int ResolveLegacyCensorshipIndex(const CServerGroup* group, const CServerInfo* server_info)
	{
		int censorship_index = SEASON3A::CGameCensorship::STATE_12;
		if (group != NULL && group->m_bPvPServer)
		{
			censorship_index = SEASON3A::CGameCensorship::STATE_18;
		}
		else if (server_info != NULL && (0x01 & server_info->m_byNonPvP))
		{
			censorship_index = SEASON3A::CGameCensorship::STATE_15;
		}

		return censorship_index;
	}
}

CGlobalText GlobalText;

namespace platform
{
	bool InitializeLegacyLoginServerListRuntime(const GlobalTextBootstrapState* global_text_state, std::string* out_error_message)
	{
		if (out_error_message != NULL)
		{
			out_error_message->clear();
		}

		g_legacy_login_server_list_ready = false;
		g_legacy_login_server_list_path = ResolveGameAssetPath("Data/Local/ServerList.bmd");
		g_ServerListManager->Release();

		GlobalText.RemoveAll();
		if (global_text_state != NULL)
		{
			for (std::map<int, std::string>::const_iterator it = global_text_state->entries.begin();
				it != global_text_state->entries.end();
				++it)
			{
				GlobalText.Add(it->first, it->second.c_str());
			}
		}

		FILE* probe = fopen(g_legacy_login_server_list_path.c_str(), "rb");
		if (probe == NULL)
		{
			if (out_error_message != NULL)
			{
				*out_error_message = "ServerList.bmd ausente";
			}
			__android_log_print(
				ANDROID_LOG_WARN,
				kLegacyLoginRuntimeLogTag,
				"Legacy server list runtime unavailable: %s",
				g_legacy_login_server_list_path.c_str());
			return false;
		}

		fclose(probe);
		g_ServerListManager->LoadServerListScript();
		g_ServerListManager->SetTotalServer(0);
		g_legacy_login_server_list_ready = true;

		__android_log_print(
			ANDROID_LOG_INFO,
			kLegacyLoginRuntimeLogTag,
			"Legacy server list runtime ready: %s",
			g_legacy_login_server_list_path.c_str());
		return true;
	}

	void SyncLegacyLoginServerListRuntime(const ConnectServerBootstrapState* connect_server_state)
	{
		if (!g_legacy_login_server_list_ready || connect_server_state == NULL)
		{
			return;
		}

		g_ServerListManager->Release();
		g_ServerListManager->SetTotalServer(static_cast<int>(connect_server_state->server_entries.size()));

		for (size_t index = 0; index < connect_server_state->server_entries.size(); ++index)
		{
			const ConnectServerEntry& entry = connect_server_state->server_entries[index];
			g_ServerListManager->InsertServerGroup(static_cast<int>(entry.connect_index), static_cast<int>(entry.load_percent));
		}
	}

	bool CollectLegacyLoginServerGroups(std::vector<LegacyLoginServerGroupState>* out_groups)
	{
		if (out_groups == NULL)
		{
			return false;
		}

		out_groups->clear();
		if (!g_legacy_login_server_list_ready)
		{
			return false;
		}

		type_mapServerGroup::iterator it = g_ServerListManager->m_mapServerGroup.begin();
		for (; it != g_ServerListManager->m_mapServerGroup.end(); ++it)
		{
			CServerGroup* group = it->second;
			if (group == NULL)
			{
				continue;
			}

			LegacyLoginServerGroupState state = {};
			state.group_index = static_cast<unsigned short>(group->m_iServerIndex);
			state.sequence = group->m_iSequence;
			state.position = group->m_iWidthPos;
			state.button_position = group->m_iBtnPos;
			state.pvp_enabled = group->m_bPvPServer;
			state.room_count = static_cast<size_t>(group->GetServerSize());
			state.name = group->m_szName;
			state.description = group->m_szDescription;
			out_groups->push_back(state);
		}

		return !out_groups->empty();
	}

	void CollectLegacyLoginServerEntries(int group_index, std::vector<LegacyLoginServerEntryState>* out_entries)
	{
		if (out_entries == NULL)
		{
			return;
		}

		out_entries->clear();
		if (!g_legacy_login_server_list_ready)
		{
			return;
		}

		CServerGroup* group = FindServerGroupByIndex(group_index);
		if (group == NULL)
		{
			return;
		}

		CServerInfo* server_info = NULL;
		group->SetFirst();
		while (group->GetNext(server_info))
		{
			if (server_info == NULL)
			{
				continue;
			}

			LegacyLoginServerEntryState state = {};
			state.connect_index = static_cast<unsigned short>(server_info->m_iConnectIndex);
			state.display_index = server_info->m_iIndex;
			state.load_percent = server_info->m_iPercent;
			state.non_pvp = server_info->m_byNonPvP;
			state.label = server_info->m_bName;
			out_entries->push_back(state);
		}
	}

	bool TrySelectLegacyLoginServerEntry(int group_index, int room_slot, unsigned short* out_connect_index)
	{
		CServerGroup* group = FindServerGroupByIndex(group_index);
		if (group == NULL)
		{
			return false;
		}

		CServerInfo* server_info = group->GetServerInfo(room_slot);
		if (server_info == NULL)
		{
			return false;
		}

		const int censorship_index = ResolveLegacyCensorshipIndex(group, server_info);
		const bool is_test_server = group->m_iSequence == 0;
		g_ServerListManager->SetSelectServerInfo(
			group->m_szName,
			server_info->m_iIndex,
			censorship_index,
			server_info->m_byNonPvP,
			is_test_server);

		if (out_connect_index != NULL)
		{
			*out_connect_index = static_cast<unsigned short>(server_info->m_iConnectIndex);
		}

		return true;
	}

	int GetLegacyLoginPrimaryGroupIndex()
	{
		if (!g_legacy_login_server_list_ready)
		{
			return -1;
		}

		type_mapServerGroup::iterator it = g_ServerListManager->m_mapServerGroup.begin();
		for (; it != g_ServerListManager->m_mapServerGroup.end(); ++it)
		{
			CServerGroup* group = it->second;
			if (group != NULL && group->m_iWidthPos == CServerGroup::SBP_CENTER)
			{
				return group->m_iServerIndex;
			}
		}

		it = g_ServerListManager->m_mapServerGroup.begin();
		if (it != g_ServerListManager->m_mapServerGroup.end() && it->second != NULL)
		{
			return it->second->m_iServerIndex;
		}

		return -1;
	}

	const char* GetLegacyLoginPrimaryGroupLabel()
	{
		if (!g_legacy_login_server_list_ready)
		{
			return "";
		}

		type_mapServerGroup::iterator it = g_ServerListManager->m_mapServerGroup.begin();
		for (; it != g_ServerListManager->m_mapServerGroup.end(); ++it)
		{
			CServerGroup* group = it->second;
			if (group != NULL && group->m_iWidthPos == CServerGroup::SBP_CENTER && group->m_szName[0] != '\0')
			{
				return group->m_szName;
			}
		}

		it = g_ServerListManager->m_mapServerGroup.begin();
		if (it != g_ServerListManager->m_mapServerGroup.end() &&
			it->second != NULL &&
			it->second->m_szName[0] != '\0')
		{
			return it->second->m_szName;
		}

		return "";
	}

	const char* GetLegacyLoginSelectedServerName()
	{
		if (!g_legacy_login_server_list_ready)
		{
			return "";
		}

		return g_ServerListManager->GetSelectServerName();
	}
}

#endif

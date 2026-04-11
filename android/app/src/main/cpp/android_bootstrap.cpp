#include <android/asset_manager.h>
#include <android/log.h>
#include <android/native_activity.h>
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vector>

#include "Platform/AndroidEglWindow.h"
#include "Platform/AndroidWin32Compat.h"
#include "Platform/GameAssetPath.h"
#include "Platform/GameBootstrapTexture.h"
#include "Platform/GameClientConfig.h"
#include "Platform/GameConnectServerBootstrap.h"
#include "Platform/GameClientRuntimeConfig.h"
#include "Platform/GameConfigBootstrap.h"
#include "Platform/GameGlobalTextBootstrap.h"
#include "Platform/GameLanguageAssetBootstrap.h"
#include "Platform/LegacyCharacterUiRuntime.h"
#include "Platform/LegacyClientRuntime.h"
#include "Platform/LegacyLoginUiRuntime.h"
#include "Platform/LegacyLoginServerListRuntime.h"
#include "Platform/GameMouseInput.h"
#include "Platform/GamePacketCryptoBootstrap.h"
#include "Platform/GameServerBootstrap.h"
#include "Platform/RenderBackend.h"
#include "Platform/AndroidGameInit.h"
#include "Platform/AndroidNetworkPollCompat.h"
#include "_define.h"

// Forward declarations for original game loop (avoids pulling in stdafx.h/WSclient.h)
class CWsctlc;
extern CWsctlc SocketClient;
extern void ProtocolCompiler(CWsctlc* pSocketClient, int iTranslation, int iParam);
extern bool CheckRenderNextFrame();
extern void RenderScene(HDC hDC);
extern HDC g_hDC;
extern int SceneFlag;

namespace
{
	const char* kLogTag = "MUAndroidBootstrap";
	const char* kPackagedDataAssetRoot = "Data";
	const char* kPackagedConfigProbeAsset = "Data/Configs/Configs.xtm";
	const char* kPackagedManifestAsset = "Data/mu_asset_manifest.txt";
	const char* kPackagedClientConfigAsset = "config.ini";
	const char* kPackagedRuntimeConfigAsset = "client_runtime.ini";
	const char* kExtractReadyMarker = ".__mu_android_assets_ready";
	const int kPreviewServerSlotCount = 16;
	const int kPreviewServerGroupSlotCount = 21;
	const int kPreviewCharacterSlotCount = 10;
	const int kPreviewServerListScriptMaxNameLength = 32;
	const int kPreviewServerListScriptMaxServerCount = 15;
	const float kLegacyPreviewWidth = 640.0f;
	const float kLegacyPreviewHeight = 480.0f;

	enum PreviewHitTarget
	{
		PreviewHitTarget_None = 0,
		PreviewHitTarget_FieldAccount,
		PreviewHitTarget_FieldPassword,
		PreviewHitTarget_ServerGroup0,
		PreviewHitTarget_ServerGroup1,
		PreviewHitTarget_ServerGroup2,
		PreviewHitTarget_ServerGroup3,
		PreviewHitTarget_ServerGroup4,
		PreviewHitTarget_ServerGroup5,
		PreviewHitTarget_ServerGroup6,
		PreviewHitTarget_ServerGroup7,
		PreviewHitTarget_ServerGroup8,
		PreviewHitTarget_ServerGroup9,
		PreviewHitTarget_ServerGroup10,
		PreviewHitTarget_ServerGroup11,
		PreviewHitTarget_ServerGroup12,
		PreviewHitTarget_ServerGroup13,
		PreviewHitTarget_ServerGroup14,
		PreviewHitTarget_ServerGroup15,
		PreviewHitTarget_ServerGroup16,
		PreviewHitTarget_ServerGroup17,
		PreviewHitTarget_ServerGroup18,
		PreviewHitTarget_ServerGroup19,
		PreviewHitTarget_ServerGroup20,
		PreviewHitTarget_ServerEntry0,
		PreviewHitTarget_ServerEntry1,
		PreviewHitTarget_ServerEntry2,
		PreviewHitTarget_ServerEntry3,
		PreviewHitTarget_ServerEntry4,
		PreviewHitTarget_ServerEntry5,
		PreviewHitTarget_ServerEntry6,
		PreviewHitTarget_ServerEntry7,
		PreviewHitTarget_ServerEntry8,
		PreviewHitTarget_ServerEntry9,
		PreviewHitTarget_ServerEntry10,
		PreviewHitTarget_ServerEntry11,
		PreviewHitTarget_ServerEntry12,
		PreviewHitTarget_ServerEntry13,
		PreviewHitTarget_ServerEntry14,
		PreviewHitTarget_ServerEntry15,
		PreviewHitTarget_CharacterEntry0,
		PreviewHitTarget_CharacterEntry1,
		PreviewHitTarget_CharacterEntry2,
		PreviewHitTarget_CharacterEntry3,
		PreviewHitTarget_CharacterEntry4,
		PreviewHitTarget_CharacterEntry5,
		PreviewHitTarget_CharacterEntry6,
		PreviewHitTarget_CharacterEntry7,
		PreviewHitTarget_CharacterEntry8,
		PreviewHitTarget_CharacterEntry9,
		PreviewHitTarget_CharacterCreate,
		PreviewHitTarget_CharacterMenu,
		PreviewHitTarget_CharacterConnect,
		PreviewHitTarget_CharacterDelete,
		PreviewHitTarget_CharacterCreateConfirm,
		PreviewHitTarget_CharacterCreateCancel,
		PreviewHitTarget_ButtonLogin,
		PreviewHitTarget_ButtonCancel,
		PreviewHitTarget_ButtonMenu,
		PreviewHitTarget_ButtonCredit,
	};

		struct PreviewRect
		{
			float x = 0.0f;
			float y = 0.0f;
			float width = 0.0f;
			float height = 0.0f;
			float virtual_width = 0.0f;
			float virtual_height = 0.0f;
		};

		struct PreviewVisibleServerGroup
		{
			unsigned short group_index;
			size_t first_entry_index;
			size_t entry_count;
			std::string name;
			std::string description;
			unsigned char position;
			unsigned char sequence;
			int button_position;
		};

		struct PreviewVisibleServerRoom
		{
			unsigned short connect_index;
			size_t connect_entry_index;
			int legacy_room_slot;
			int display_index;
			unsigned char load_percent;
			unsigned char non_pvp;
			std::string label;
		};

		struct PreviewServerGroupScript
		{
			unsigned short group_index;
			std::string name;
			unsigned char position;
			unsigned char sequence;
			unsigned char non_pvp[kPreviewServerListScriptMaxServerCount];
			std::string description;
		};

		struct PreviewServerListScriptState
		{
			bool loaded;
			std::string source_path;
			std::string error_message;
			std::vector<PreviewServerGroupScript> groups;
		};

		struct BootstrapState
		{
		platform::AndroidEglWindow egl_window;
		bool has_focus;
		bool has_game_data;
		bool has_core_bootstrap;
		bool has_client_config;
		bool has_client_ini_bootstrap;
			bool has_runtime_config;
			bool has_language_assets;
			bool has_global_text_bootstrap;
			bool has_server_list_script;
		bool has_interface_preview;
		bool legacy_login_ui_ready;
		bool legacy_character_ui_ready;
		bool has_packet_crypto_bootstrap;
		bool touch_active;
		bool multi_touch_active;
		float touch_x;
		float touch_y;
		int touch_pointer_id;
		int preview_pressed_target;
		int preview_focused_target;
		unsigned int touch_move_log_counter;
		unsigned int touch_event_counter;
		unsigned int preview_action_count;
		platform::GameMouseState game_mouse_state;
		platform::GameMouseMetrics game_mouse_metrics;
		platform::ClientRuntimeConfigState runtime_config;
		platform::CoreGameBootstrapState core_bootstrap_state;
		platform::ConnectServerBootstrapState connect_server_bootstrap;
		platform::GameServerBootstrapState game_server_bootstrap;
		platform::LanguageAssetBootstrapState language_assets;
			platform::GlobalTextBootstrapState global_text;
			PreviewServerListScriptState server_list_script;
		std::string auto_login_user;
		std::string preview_account_value;
		std::string preview_password_value;
		std::string preview_status_message;
		bool interface_background_uses_legacy_login;
		bool interface_uses_object95_scene;
		platform::BootstrapTexture interface_background_left_texture;
		platform::BootstrapTexture interface_background_right_texture;
		platform::BootstrapTexture interface_scene_chrome_texture;
		platform::BootstrapTexture interface_scene_swirl_texture;
		platform::BootstrapTexture interface_scene_leaf_texture;
		platform::BootstrapTexture interface_scene_leaf_alt_texture;
		platform::BootstrapTexture interface_scene_wave_texture;
		platform::BootstrapTexture interface_scene_cloud_texture;
		platform::BootstrapTexture interface_scene_horizon_texture;
		platform::BootstrapTexture interface_scene_cloud_band_texture;
		platform::BootstrapTexture interface_scene_water_texture;
		platform::BootstrapTexture interface_scene_water_detail_texture;
		platform::BootstrapTexture interface_scene_land_texture;
		platform::BootstrapTexture interface_title_texture;
		platform::BootstrapTexture interface_title_glow_texture;
		platform::BootstrapTexture interface_login_window_texture;
		platform::BootstrapTexture interface_medium_button_texture;
		platform::BootstrapTexture interface_server_group_button_texture;
		platform::BootstrapTexture interface_server_button_texture;
		platform::BootstrapTexture interface_server_gauge_texture;
		platform::BootstrapTexture interface_server_deco_texture;
		platform::BootstrapTexture interface_server_desc_fill_texture;
		platform::BootstrapTexture interface_server_desc_cap_texture;
		platform::BootstrapTexture interface_server_desc_corner_texture;
		platform::BootstrapTexture interface_credit_logo_texture;
		platform::BootstrapTexture interface_bottom_deco_texture;
		platform::BootstrapTexture interface_menu_button_texture;
		platform::BootstrapTexture interface_credit_button_texture;
		PreviewRect preview_server_group_rects[kPreviewServerGroupSlotCount];
		PreviewRect preview_server_entry_rects[kPreviewServerSlotCount];
		PreviewRect preview_character_entry_rects[kPreviewCharacterSlotCount];
		PreviewRect preview_character_create_window_rect;
		PreviewRect preview_character_create_rect;
		PreviewRect preview_character_menu_rect;
		PreviewRect preview_character_connect_rect;
		PreviewRect preview_character_delete_rect;
		PreviewRect preview_character_create_ok_rect;
		PreviewRect preview_character_create_cancel_rect;
		PreviewRect preview_account_rect;
		PreviewRect preview_password_rect;
		PreviewRect preview_login_button_rect;
		PreviewRect preview_cancel_button_rect;
		PreviewRect preview_menu_button_rect;
		PreviewRect preview_credit_button_rect;
		int preview_selected_server_group;
		int preview_selected_server_slot;
		unsigned int preview_key_event_count;
		int preview_connect_logged_status;
		unsigned int preview_connect_logged_attempt;
		size_t preview_connect_logged_server_count;
		std::string preview_connect_logged_message;
		int preview_game_logged_status;
		unsigned int preview_game_logged_attempt;
		std::string preview_game_logged_message;
		float phase;
		bool original_game_initialized;
	};

	bool CollectPreviewCharacterEntries(
		const BootstrapState* state,
		std::vector<platform::LegacyCharacterUiEntryState>* out_entries);

	void LogInfo(const char* message)
	{
		__android_log_write(ANDROID_LOG_INFO, kLogTag, message);
	}

	const char* GetMotionActionName(int32_t action_mask)
	{
		switch (action_mask)
		{
		case AMOTION_EVENT_ACTION_DOWN:
			return "down";
		case AMOTION_EVENT_ACTION_UP:
			return "up";
		case AMOTION_EVENT_ACTION_MOVE:
			return "move";
		case AMOTION_EVENT_ACTION_CANCEL:
			return "cancel";
		case AMOTION_EVENT_ACTION_POINTER_DOWN:
			return "pointer_down";
		case AMOTION_EVENT_ACTION_POINTER_UP:
			return "pointer_up";
		default:
			return "motion";
		}
	}

	void RefreshGameMouseMetrics(BootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		const int surface_width = state->egl_window.IsReady() ? state->egl_window.GetWidth() : static_cast<int>(state->runtime_config.window_width);
		const int surface_height = state->egl_window.IsReady() ? state->egl_window.GetHeight() : static_cast<int>(state->runtime_config.window_height);
		float pointer_scale = state->runtime_config.screen_rate_y;
		if (surface_height > 0)
		{
			pointer_scale = static_cast<float>(surface_height) / kLegacyPreviewHeight;
		}

		state->game_mouse_metrics = platform::CreateGameMouseMetrics(
			surface_width,
			surface_height,
			pointer_scale);
	}

	void LogTouchEvent(const BootstrapState* state, const char* action_name, int pointer_id, float x, float y, size_t pointer_count)
	{
		__android_log_print(
			ANDROID_LOG_INFO,
			kLogTag,
			"Touch %s id=%d x=%.1f y=%.1f gx=%d gy=%d l=%s push=%s pop=%s pointers=%zu active=%s multi=%s event=%u",
			action_name != NULL ? action_name : "motion",
			pointer_id,
			x,
			y,
			state != NULL ? state->game_mouse_state.x : 0,
			state != NULL ? state->game_mouse_state.y : 0,
			(state != NULL && state->game_mouse_state.left_button) ? "down" : "up",
			(state != NULL && state->game_mouse_state.left_button_push) ? "yes" : "no",
			(state != NULL && state->game_mouse_state.left_button_pop) ? "yes" : "no",
			pointer_count,
			(state != NULL && state->touch_active) ? "yes" : "no",
			(state != NULL && state->multi_touch_active) ? "yes" : "no",
			state != NULL ? state->touch_event_counter : 0u);
	}

	bool PointInsideRect(float x, float y, float left, float top, float width, float height)
	{
		return width > 0.0f &&
			height > 0.0f &&
			x >= left &&
			x <= (left + width) &&
			y >= top &&
			y <= (top + height);
	}

	void ClearPreviewRect(PreviewRect* rect)
	{
		if (rect == NULL)
		{
			return;
		}

		rect->x = 0.0f;
		rect->y = 0.0f;
		rect->width = 0.0f;
		rect->height = 0.0f;
		rect->virtual_width = 0.0f;
		rect->virtual_height = 0.0f;
	}

	void ResetPreviewHitAreas(BootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		ClearPreviewRect(&state->preview_account_rect);
		ClearPreviewRect(&state->preview_password_rect);
		ClearPreviewRect(&state->preview_login_button_rect);
		ClearPreviewRect(&state->preview_cancel_button_rect);
		ClearPreviewRect(&state->preview_menu_button_rect);
		ClearPreviewRect(&state->preview_credit_button_rect);
		for (int index = 0; index < kPreviewServerGroupSlotCount; ++index)
		{
			ClearPreviewRect(&state->preview_server_group_rects[index]);
		}
		for (int index = 0; index < kPreviewServerSlotCount; ++index)
		{
			ClearPreviewRect(&state->preview_server_entry_rects[index]);
		}
		for (int index = 0; index < kPreviewCharacterSlotCount; ++index)
		{
			ClearPreviewRect(&state->preview_character_entry_rects[index]);
		}
		ClearPreviewRect(&state->preview_character_create_window_rect);
		ClearPreviewRect(&state->preview_character_create_rect);
		ClearPreviewRect(&state->preview_character_menu_rect);
		ClearPreviewRect(&state->preview_character_connect_rect);
		ClearPreviewRect(&state->preview_character_delete_rect);
		ClearPreviewRect(&state->preview_character_create_ok_rect);
		ClearPreviewRect(&state->preview_character_create_cancel_rect);
	}

	float ConvertWindowYToRenderY(const BootstrapState* state, float window_y)
	{
		if (state == NULL || !state->egl_window.IsReady())
		{
			return window_y;
		}

		return static_cast<float>(state->egl_window.GetHeight()) - window_y;
	}

	bool PointInsidePreviewRect(float x, float render_y, const PreviewRect& rect)
	{
		return PointInsideRect(x, render_y, rect.x, rect.y, rect.width, rect.height);
	}

	PreviewHitTarget HitTestPreviewTarget(const BootstrapState* state, float window_x, float window_y)
	{
		if (state == NULL || !state->has_interface_preview)
		{
			return PreviewHitTarget_None;
		}

		const float render_y = ConvertWindowYToRenderY(state, window_y);
		if (PointInsidePreviewRect(window_x, render_y, state->preview_login_button_rect))
		{
			return PreviewHitTarget_ButtonLogin;
		}
		if (PointInsidePreviewRect(window_x, render_y, state->preview_cancel_button_rect))
		{
			return PreviewHitTarget_ButtonCancel;
		}
		if (PointInsidePreviewRect(window_x, render_y, state->preview_menu_button_rect))
		{
			return PreviewHitTarget_ButtonMenu;
		}
		if (PointInsidePreviewRect(window_x, render_y, state->preview_credit_button_rect))
		{
			return PreviewHitTarget_ButtonCredit;
		}
		for (int index = 0; index < kPreviewServerGroupSlotCount; ++index)
		{
			if (PointInsidePreviewRect(window_x, render_y, state->preview_server_group_rects[index]))
			{
				return static_cast<PreviewHitTarget>(PreviewHitTarget_ServerGroup0 + index);
			}
		}
		for (int index = 0; index < kPreviewServerSlotCount; ++index)
		{
			if (PointInsidePreviewRect(window_x, render_y, state->preview_server_entry_rects[index]))
			{
				return static_cast<PreviewHitTarget>(PreviewHitTarget_ServerEntry0 + index);
			}
		}
		for (int index = 0; index < kPreviewCharacterSlotCount; ++index)
		{
			if (PointInsidePreviewRect(window_x, render_y, state->preview_character_entry_rects[index]))
			{
				return static_cast<PreviewHitTarget>(PreviewHitTarget_CharacterEntry0 + index);
			}
		}
		if (PointInsidePreviewRect(window_x, render_y, state->preview_character_create_ok_rect))
		{
			return PreviewHitTarget_CharacterCreateConfirm;
		}
		if (PointInsidePreviewRect(window_x, render_y, state->preview_character_create_cancel_rect))
		{
			return PreviewHitTarget_CharacterCreateCancel;
		}
		if (PointInsidePreviewRect(window_x, render_y, state->preview_character_create_rect))
		{
			return PreviewHitTarget_CharacterCreate;
		}
		if (PointInsidePreviewRect(window_x, render_y, state->preview_character_menu_rect))
		{
			return PreviewHitTarget_CharacterMenu;
		}
		if (PointInsidePreviewRect(window_x, render_y, state->preview_character_connect_rect))
		{
			return PreviewHitTarget_CharacterConnect;
		}
		if (PointInsidePreviewRect(window_x, render_y, state->preview_character_delete_rect))
		{
			return PreviewHitTarget_CharacterDelete;
		}
		if (PointInsidePreviewRect(window_x, render_y, state->preview_character_create_window_rect))
		{
			return PreviewHitTarget_None;
		}
		if (PointInsidePreviewRect(window_x, render_y, state->preview_account_rect))
		{
			return PreviewHitTarget_FieldAccount;
		}
		if (PointInsidePreviewRect(window_x, render_y, state->preview_password_rect))
		{
			return PreviewHitTarget_FieldPassword;
		}

		return PreviewHitTarget_None;
	}

	bool IsPreviewFieldTarget(int target)
	{
		return target == PreviewHitTarget_FieldAccount || target == PreviewHitTarget_FieldPassword;
	}

	bool IsPreviewButtonTarget(int target)
	{
		return target == PreviewHitTarget_ButtonLogin ||
			target == PreviewHitTarget_ButtonCancel ||
			target == PreviewHitTarget_ButtonMenu ||
			target == PreviewHitTarget_ButtonCredit ||
			target == PreviewHitTarget_CharacterCreate ||
			target == PreviewHitTarget_CharacterMenu ||
			target == PreviewHitTarget_CharacterConnect ||
			target == PreviewHitTarget_CharacterDelete ||
			target == PreviewHitTarget_CharacterCreateConfirm ||
			target == PreviewHitTarget_CharacterCreateCancel ||
			(target >= PreviewHitTarget_ServerGroup0 && target <= PreviewHitTarget_ServerGroup20) ||
			(target >= PreviewHitTarget_ServerEntry0 && target <= PreviewHitTarget_ServerEntry15) ||
			(target >= PreviewHitTarget_CharacterEntry0 && target <= PreviewHitTarget_CharacterEntry9);
	}

	int GetPreviewServerGroupSlotFromTarget(int target)
	{
		if (target < PreviewHitTarget_ServerGroup0 || target > PreviewHitTarget_ServerGroup20)
		{
			return -1;
		}

		return target - PreviewHitTarget_ServerGroup0;
	}

	int GetPreviewServerSlotFromTarget(int target)
	{
		if (target < PreviewHitTarget_ServerEntry0 || target > PreviewHitTarget_ServerEntry15)
		{
			return -1;
		}

		return target - PreviewHitTarget_ServerEntry0;
	}

	int GetPreviewCharacterSlotFromTarget(int target)
	{
		if (target < PreviewHitTarget_CharacterEntry0 || target > PreviewHitTarget_CharacterEntry9)
		{
			return -1;
		}

		return target - PreviewHitTarget_CharacterEntry0;
	}

	const char* GetPreviewTargetLabel(const BootstrapState* state, int target)
	{
		if (state != NULL && state->has_global_text_bootstrap)
		{
			switch (target)
			{
			case PreviewHitTarget_FieldAccount:
				return platform::FindGlobalTextEntry(&state->global_text, 450);
			case PreviewHitTarget_FieldPassword:
				return platform::FindGlobalTextEntry(&state->global_text, 451);
			case PreviewHitTarget_ButtonLogin:
				return platform::FindGlobalTextEntry(&state->global_text, 452);
			case PreviewHitTarget_ButtonCancel:
				return platform::FindGlobalTextEntry(&state->global_text, 453);
			default:
				break;
			}
		}

		switch (target)
		{
		case PreviewHitTarget_FieldAccount:
			return "account";
		case PreviewHitTarget_FieldPassword:
			return "password";
		case PreviewHitTarget_ButtonLogin:
			return "login";
		case PreviewHitTarget_ButtonCancel:
			return "cancel";
		case PreviewHitTarget_ButtonMenu:
			return "menu";
		case PreviewHitTarget_ButtonCredit:
			return "credits";
		case PreviewHitTarget_CharacterCreate:
			return "char-create";
		case PreviewHitTarget_CharacterMenu:
			return "char-menu";
		case PreviewHitTarget_CharacterConnect:
			return "char-connect";
		case PreviewHitTarget_CharacterDelete:
			return "char-delete";
		case PreviewHitTarget_CharacterCreateConfirm:
			return "char-create-ok";
		case PreviewHitTarget_CharacterCreateCancel:
			return "char-create-cancel";
		default:
			break;
		}

		if (target >= PreviewHitTarget_ServerGroup0 && target <= PreviewHitTarget_ServerGroup20)
		{
			return "server-group";
		}
		if (target >= PreviewHitTarget_ServerEntry0 && target <= PreviewHitTarget_ServerEntry15)
		{
			return "server";
		}
		if (target >= PreviewHitTarget_CharacterEntry0 && target <= PreviewHitTarget_CharacterEntry9)
		{
			return "character";
		}
		return "none";
	}

		void SetPreviewStatusMessage(BootstrapState* state, const std::string& message)
		{
			if (state == NULL)
		{
			return;
		}

			state->preview_status_message = message;
		}

		void BuxConvertPreviewServerList(unsigned char* buffer, size_t size)
		{
			static const unsigned char kBuxCode[3] = { 0xFC, 0xCF, 0xAB };
			if (buffer == NULL)
			{
				return;
			}

			for (size_t index = 0; index < size; ++index)
			{
				buffer[index] ^= kBuxCode[index % 3];
			}
		}

		const PreviewServerGroupScript* FindPreviewServerGroupScript(const BootstrapState* state, unsigned short group_index)
		{
			if (state == NULL || !state->has_server_list_script)
			{
				return NULL;
			}

			for (size_t index = 0; index < state->server_list_script.groups.size(); ++index)
			{
				if (state->server_list_script.groups[index].group_index == group_index)
				{
					return &state->server_list_script.groups[index];
				}
			}

			return NULL;
		}

		std::string BuildPreviewServerEntryLabel(const BootstrapState* state, const platform::ConnectServerEntry& entry);

		void CollectPreviewVisibleServerGroups(const BootstrapState* state, std::vector<PreviewVisibleServerGroup>* groups)
		{
			if (groups == NULL)
			{
				return;
			}

			groups->clear();
			if (state == NULL)
			{
				return;
			}

			std::vector<platform::LegacyLoginServerGroupState> legacy_groups;
			if (platform::CollectLegacyLoginServerGroups(&legacy_groups) && !legacy_groups.empty())
			{
				for (size_t index = 0; index < legacy_groups.size(); ++index)
				{
					const platform::LegacyLoginServerGroupState& legacy_group = legacy_groups[index];
					PreviewVisibleServerGroup group = {};
					group.group_index = legacy_group.group_index;
					group.first_entry_index = 0u;
					group.entry_count = legacy_group.room_count;
					group.name = legacy_group.name;
					group.description = legacy_group.description;
					group.position = static_cast<unsigned char>(legacy_group.position);
					group.sequence = static_cast<unsigned char>(legacy_group.sequence);
					group.button_position = legacy_group.button_position;
					groups->push_back(group);
				}

				std::sort(
					groups->begin(),
					groups->end(),
					[](const PreviewVisibleServerGroup& left, const PreviewVisibleServerGroup& right) -> bool
					{
						if (left.position != right.position)
						{
							if (left.position == 2u)
							{
								return true;
							}
							if (right.position == 2u)
							{
								return false;
							}
							return left.position < right.position;
						}
						if (left.sequence != right.sequence)
						{
							return left.sequence < right.sequence;
						}
						return left.group_index < right.group_index;
					});
				return;
			}

			for (size_t entry_index = 0; entry_index < state->connect_server_bootstrap.server_entries.size(); ++entry_index)
			{
				const platform::ConnectServerEntry& entry = state->connect_server_bootstrap.server_entries[entry_index];
				const unsigned short group_index = static_cast<unsigned short>(entry.connect_index / 20);
				PreviewVisibleServerGroup* existing_group = NULL;
				for (size_t group_slot = 0; group_slot < groups->size(); ++group_slot)
				{
					if ((*groups)[group_slot].group_index == group_index)
					{
						existing_group = &(*groups)[group_slot];
						break;
					}
				}

				if (existing_group != NULL)
				{
					existing_group->entry_count += 1u;
					continue;
				}

				PreviewVisibleServerGroup group = {};
				group.group_index = group_index;
				group.first_entry_index = entry_index;
				group.entry_count = 1u;
				const PreviewServerGroupScript* group_script = FindPreviewServerGroupScript(state, group_index);
				if (group_script != NULL)
				{
					group.name = group_script->name;
					group.description = group_script->description;
					group.position = group_script->position;
					group.sequence = group_script->sequence;
					group.button_position = -1;
				}
				else
				{
					group.position = 2u;
					group.sequence = static_cast<unsigned char>(groups->size());
					group.button_position = -1;
				}
				if (group.name.empty())
				{
					std::ostringstream fallback_name;
					fallback_name << "Server " << static_cast<unsigned int>(group_index + 1u);
					group.name = fallback_name.str();
				}
				groups->push_back(group);
			}

			std::sort(
				groups->begin(),
				groups->end(),
				[](const PreviewVisibleServerGroup& left, const PreviewVisibleServerGroup& right) -> bool
				{
					if (left.position != right.position)
					{
						if (left.position == 2u)
						{
							return true;
						}
						if (right.position == 2u)
						{
							return false;
						}
						return left.position < right.position;
					}
					if (left.sequence != right.sequence)
					{
						return left.sequence < right.sequence;
					}
					return left.group_index < right.group_index;
				});
		}

		int FindPreviewVisibleServerGroupSlot(const std::vector<PreviewVisibleServerGroup>& groups, int group_index)
		{
			for (size_t index = 0; index < groups.size(); ++index)
			{
				if (static_cast<int>(groups[index].group_index) == group_index)
				{
					return static_cast<int>(index);
				}
			}

			return -1;
		}

		int GetPreviewServerGroupDrawSlot(const PreviewVisibleServerGroup& group)
		{
			if (group.position == 2u)
			{
				return 0;
			}
			if (group.position == 0u)
			{
				return 1 + static_cast<int>(group.sequence);
			}
			if (group.position == 1u)
			{
				return 11 + static_cast<int>(group.sequence);
			}

			return 1 + static_cast<int>(group.sequence);
		}

		void CollectPreviewVisibleServerRooms(const BootstrapState* state, int group_index, std::vector<PreviewVisibleServerRoom>* rooms)
		{
			if (rooms == NULL)
			{
				return;
			}

			rooms->clear();
			if (state == NULL)
			{
				return;
			}

			std::vector<platform::LegacyLoginServerEntryState> legacy_entries;
			platform::CollectLegacyLoginServerEntries(group_index, &legacy_entries);
			if (!legacy_entries.empty())
			{
				for (size_t legacy_index = 0; legacy_index < legacy_entries.size(); ++legacy_index)
				{
					const platform::LegacyLoginServerEntryState& legacy_entry = legacy_entries[legacy_index];
					PreviewVisibleServerRoom room = {};
					room.connect_index = legacy_entry.connect_index;
					room.connect_entry_index = static_cast<size_t>(-1);
					room.legacy_room_slot = static_cast<int>(legacy_index);
					room.display_index = legacy_entry.display_index;
					room.load_percent = static_cast<unsigned char>(legacy_entry.load_percent);
					room.non_pvp = legacy_entry.non_pvp;
					room.label = legacy_entry.label;

					for (size_t entry_index = 0; entry_index < state->connect_server_bootstrap.server_entries.size(); ++entry_index)
					{
						if (state->connect_server_bootstrap.server_entries[entry_index].connect_index == legacy_entry.connect_index)
						{
							room.connect_entry_index = entry_index;
							room.load_percent = state->connect_server_bootstrap.server_entries[entry_index].load_percent;
							break;
						}
					}

					rooms->push_back(room);
				}

				return;
			}

			for (size_t entry_index = 0; entry_index < state->connect_server_bootstrap.server_entries.size(); ++entry_index)
			{
				const platform::ConnectServerEntry& entry = state->connect_server_bootstrap.server_entries[entry_index];
				if (group_index >= 0 && static_cast<int>(entry.connect_index / 20) != group_index)
				{
					continue;
				}

				PreviewVisibleServerRoom room = {};
				room.connect_index = entry.connect_index;
				room.connect_entry_index = entry_index;
				room.legacy_room_slot = static_cast<int>(rooms->size());
				room.display_index = static_cast<int>(entry.connect_index % 20) + 1;
				room.load_percent = entry.load_percent;
				room.non_pvp = 0;
				room.label = BuildPreviewServerEntryLabel(state, entry);
				rooms->push_back(room);
			}
		}

		void SyncPreviewSelectionFromLegacyRuntime(BootstrapState* state, int default_room_slot = -1)
		{
			if (state == NULL || !state->legacy_login_ui_ready)
			{
				return;
			}

			const int legacy_selected_group = platform::GetLegacySelectedServerGroupIndex();
			if (legacy_selected_group >= 0)
			{
				state->preview_selected_server_group = legacy_selected_group;
			}

			const int legacy_selected_room = platform::GetLegacySelectedServerRoomSlot();
			if (legacy_selected_room >= 0)
			{
				state->preview_selected_server_slot = legacy_selected_room;
			}
			else if (default_room_slot >= 0)
			{
				state->preview_selected_server_slot = default_room_slot;
			}
		}

		bool TryHandleLegacyServerGroupTarget(BootstrapState* state, int target)
		{
			if (state == NULL || !state->legacy_login_ui_ready)
			{
				return false;
			}

			std::vector<PreviewVisibleServerGroup> visible_groups;
			CollectPreviewVisibleServerGroups(state, &visible_groups);
			const int group_slot = GetPreviewServerGroupSlotFromTarget(target);
			if (group_slot < 0 || static_cast<size_t>(group_slot) >= visible_groups.size())
			{
				return false;
			}

			const PreviewVisibleServerGroup& group = visible_groups[group_slot];
			if (group.button_position < 0)
			{
				return false;
			}

			const bool handled = platform::HandleLegacyServerGroupAction(group.button_position);
			if (handled)
			{
				SyncPreviewSelectionFromLegacyRuntime(state, 0);
			}
			return handled;
		}

		bool TryHandleLegacyServerRoomTarget(BootstrapState* state, int target)
		{
			if (state == NULL || !state->legacy_login_ui_ready)
			{
				return false;
			}

			const int slot = GetPreviewServerSlotFromTarget(target);
			if (slot < 0)
			{
				return false;
			}

			std::vector<PreviewVisibleServerRoom> visible_rooms;
			CollectPreviewVisibleServerRooms(state, state->preview_selected_server_group, &visible_rooms);
			if (static_cast<size_t>(slot) >= visible_rooms.size())
			{
				return false;
			}

			const PreviewVisibleServerRoom& room = visible_rooms[slot];
			const int legacy_room_slot = room.legacy_room_slot >= 0 ? room.legacy_room_slot : slot;
			const bool handled = platform::HandleLegacyServerRoomAction(legacy_room_slot);
			if (handled)
			{
				SyncPreviewSelectionFromLegacyRuntime(state);
			}
			return handled;
		}

		void NormalizePreviewServerSelection(BootstrapState* state)
		{
			if (state == NULL)
			{
				return;
			}

			std::vector<PreviewVisibleServerGroup> groups;
			CollectPreviewVisibleServerGroups(state, &groups);
			if (groups.empty())
			{
				state->preview_selected_server_group = -1;
				state->preview_selected_server_slot = -1;
				return;
			}

			int preferred_group_index = -1;
			if (state->legacy_login_ui_ready)
			{
				preferred_group_index = platform::GetLegacyLoginPrimaryGroupIndex();
			}

			if (state->preview_selected_server_group < 0 ||
				FindPreviewVisibleServerGroupSlot(groups, state->preview_selected_server_group) < 0)
			{
				if (preferred_group_index >= 0 &&
					FindPreviewVisibleServerGroupSlot(groups, preferred_group_index) >= 0)
				{
					state->preview_selected_server_group = preferred_group_index;
				}
				else
				{
					state->preview_selected_server_group = static_cast<int>(groups[0].group_index);
				}
			}

			std::vector<PreviewVisibleServerRoom> rooms;
			CollectPreviewVisibleServerRooms(state, state->preview_selected_server_group, &rooms);
			if (rooms.empty())
			{
				state->preview_selected_server_slot = -1;
				return;
			}

			if (state->preview_selected_server_slot < 0 ||
				static_cast<size_t>(state->preview_selected_server_slot) >= rooms.size())
			{
				state->preview_selected_server_slot = 0;
			}
		}

		const char* GetPreviewServerLoadText(const BootstrapState* state, unsigned char load_percent)
		{
			if (load_percent >= 128)
			{
				return "Lotado";
			}

			if (load_percent >= 100)
			{
				return "Ocupado";
			}

			(void)state;
			return "Normal";
		}

	std::string BuildPreviewServerEntryLabel(const BootstrapState* state, const platform::ConnectServerEntry& entry)
	{
		std::vector<platform::LegacyLoginServerEntryState> legacy_entries;
		platform::CollectLegacyLoginServerEntries(static_cast<int>(entry.connect_index / 20), &legacy_entries);
		for (size_t index = 0; index < legacy_entries.size(); ++index)
		{
			if (legacy_entries[index].connect_index == entry.connect_index && !legacy_entries[index].label.empty())
			{
				return legacy_entries[index].label;
			}
		}

		std::ostringstream stream;
		const unsigned short room_index = static_cast<unsigned short>(entry.connect_index + 1);
		const unsigned short group_index = static_cast<unsigned short>(entry.connect_index / 20);
			const PreviewServerGroupScript* group_script = FindPreviewServerGroupScript(state, group_index);

			if (group_script != NULL && !group_script->name.empty())
			{
				stream << group_script->name << "-" << room_index;
				const size_t group_slot = static_cast<size_t>(entry.connect_index % 20);
				if (group_slot < kPreviewServerListScriptMaxServerCount)
				{
					switch (group_script->non_pvp[group_slot])
					{
					case 1:
						stream << " (Non-PVP)";
						break;
					case 2:
						stream << " (Gold PVP)";
						break;
					case 3:
						stream << " (Gold)";
						break;
					default:
						break;
					}
				}
			}
			else
			{
				stream << "Sala-" << room_index;
			}
			return stream.str();
	}

	void ClearPreviewSelectedServerAddress(BootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		state->connect_server_bootstrap.server_address_requested = false;
		state->connect_server_bootstrap.server_address_received = false;
		state->connect_server_bootstrap.requested_server_index = 0;
		state->connect_server_bootstrap.selected_game_server_host.clear();
		state->connect_server_bootstrap.selected_game_server_port = 0;
	}

	void SyncSelectedCharacterFromLegacyRuntime(BootstrapState* state)
	{
		if (state == NULL || !state->legacy_character_ui_ready)
		{
			return;
		}

		const int selected_slot = platform::GetLegacyCharacterUiSelectedSlot();
		if (selected_slot < 0)
		{
			state->game_server_bootstrap.selected_character_slot = 0xFF;
			state->game_server_bootstrap.selected_character_name.clear();
			return;
		}

		state->game_server_bootstrap.selected_character_slot = static_cast<unsigned char>(selected_slot);
		std::vector<platform::LegacyCharacterUiEntryState> entries;
		if (CollectPreviewCharacterEntries(state, &entries))
		{
			for (size_t index = 0; index < entries.size(); ++index)
			{
				const platform::LegacyCharacterUiEntryState& entry = entries[index];
				if (entry.slot == state->game_server_bootstrap.selected_character_slot)
				{
					state->game_server_bootstrap.selected_character_name = entry.name;
					return;
				}
			}
		}

		state->game_server_bootstrap.selected_character_name.clear();
	}

	bool CollectPreviewCharacterEntries(
		const BootstrapState* state,
		std::vector<platform::LegacyCharacterUiEntryState>* out_entries)
	{
		if (out_entries == NULL)
		{
			return false;
		}

		out_entries->clear();
		if (state != NULL &&
			state->legacy_character_ui_ready &&
			platform::CollectLegacyCharacterUiEntries(out_entries) &&
			!out_entries->empty())
		{
			return true;
		}

		if (state == NULL)
		{
			return false;
		}

		for (size_t index = 0; index < state->game_server_bootstrap.characters.size(); ++index)
		{
			const platform::GameServerCharacterEntry& source_entry = state->game_server_bootstrap.characters[index];
			platform::LegacyCharacterUiEntryState entry = {};
			entry.visible = true;
			entry.slot = source_entry.slot;
			entry.level = source_entry.level;
			entry.selected =
				state->game_server_bootstrap.selected_character_slot != 0xFF &&
				state->game_server_bootstrap.selected_character_slot == source_entry.slot;
			entry.name = source_entry.name;
			out_entries->push_back(entry);
		}

		return !out_entries->empty();
	}

	std::string BuildConnectServerSummary(const BootstrapState* state)
	{
		if (state == NULL || !state->connect_server_bootstrap.configured)
		{
			return std::string("ConnectServer offline");
		}

		std::ostringstream stream;
		stream << state->connect_server_bootstrap.host << ":" << state->connect_server_bootstrap.port;
		return stream.str();
	}

		std::string BuildGameServerSummary(const BootstrapState* state)
		{
		if (state == NULL || !state->connect_server_bootstrap.server_address_received)
		{
			return std::string();
		}

		const bool has_override =
			!state->runtime_config.game_server_host.empty() || state->runtime_config.game_server_port != 0;
		const std::string effective_host =
			!state->runtime_config.game_server_host.empty()
				? state->runtime_config.game_server_host
				: state->connect_server_bootstrap.selected_game_server_host;
		const unsigned short effective_port =
			state->runtime_config.game_server_port != 0
				? state->runtime_config.game_server_port
				: state->connect_server_bootstrap.selected_game_server_port;

		std::ostringstream stream;
		stream << "GS " << effective_host << ":" << effective_port;
		if (has_override)
		{
			stream << " (srv "
				   << state->connect_server_bootstrap.selected_game_server_host
				   << ":" << state->connect_server_bootstrap.selected_game_server_port
				   << ")";
			}
			return stream.str();
		}

		bool IsPreviewCharacterStage(const BootstrapState* state)
		{
			if (state == NULL)
			{
				return false;
			}

			return state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_CharacterListReady ||
				state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_MapJoinReady;
		}

		bool IsPreviewLoginStage(const BootstrapState* state)
		{
			if (state == NULL || IsPreviewCharacterStage(state))
			{
				return false;
			}

			return state->connect_server_bootstrap.server_address_received ||
				state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_Connecting ||
				state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_Connected ||
				state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_JoinReady ||
				state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_LoginSucceeded ||
				state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_LoginFailed ||
				state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_Failed;
		}

		bool IsPreviewServerBrowserStage(const BootstrapState* state)
		{
			if (state == NULL || IsPreviewCharacterStage(state) || IsPreviewLoginStage(state))
			{
				return false;
			}

			return state->connect_server_bootstrap.status == platform::ConnectServerBootstrapStatus_Connected &&
				state->connect_server_bootstrap.server_list_received;
		}

		const char* GetPreviewSelectServerButtonLabel(const BootstrapState* state)
		{
			const char* legacy_primary_label = platform::GetLegacyLoginPrimaryGroupLabel();
			if (legacy_primary_label != NULL && legacy_primary_label[0] != '\0')
			{
				return legacy_primary_label;
			}

			if (state != NULL && state->has_server_list_script && !state->server_list_script.groups.empty())
			{
				for (size_t index = 0; index < state->server_list_script.groups.size(); ++index)
				{
					if (state->server_list_script.groups[index].position == 2u &&
						!state->server_list_script.groups[index].name.empty())
					{
						return state->server_list_script.groups[index].name.c_str();
					}
				}

				if (!state->server_list_script.groups[0].name.empty())
				{
					return state->server_list_script.groups[0].name.c_str();
				}
			}

			return "Switch Servers";
		}

		const char* GetPreviewPrimaryButtonLabel(const BootstrapState* state)
		{
			if (IsPreviewLoginStage(state))
			{
				return GetPreviewTargetLabel(state, PreviewHitTarget_ButtonLogin);
			}

			return GetPreviewSelectServerButtonLabel(state);
		}

		const char* GetPreviewActionTargetLabel(const BootstrapState* state, int target)
		{
			if (target == PreviewHitTarget_ButtonLogin)
			{
				return GetPreviewPrimaryButtonLabel(state);
			}

			return GetPreviewTargetLabel(state, target);
		}

	void RefreshPreviewStatusFromGameServer(BootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		switch (state->game_server_bootstrap.status)
		{
			case platform::GameServerBootstrapStatus_Connecting:
			case platform::GameServerBootstrapStatus_Connected:
			case platform::GameServerBootstrapStatus_JoinReady:
			case platform::GameServerBootstrapStatus_LoginSucceeded:
			case platform::GameServerBootstrapStatus_CharacterListReady:
			case platform::GameServerBootstrapStatus_MapJoinReady:
			case platform::GameServerBootstrapStatus_LoginFailed:
			case platform::GameServerBootstrapStatus_Failed:
				SetPreviewStatusMessage(state, state->game_server_bootstrap.status_message);
			break;
		default:
			break;
		}
	}

	void RefreshPreviewStatusFromConnectServer(BootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		switch (state->connect_server_bootstrap.status)
		{
		case platform::ConnectServerBootstrapStatus_Connecting:
		case platform::ConnectServerBootstrapStatus_Connected:
		case platform::ConnectServerBootstrapStatus_Failed:
			SetPreviewStatusMessage(state, state->connect_server_bootstrap.status_message);
			break;
		default:
			break;
		}
	}

	void PollPreviewConnectServer(BootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		const bool suspend_connect_server_poll =
			state->connect_server_bootstrap.server_address_received ||
			state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_Connecting ||
			state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_Connected ||
			state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_JoinReady ||
			state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_LoginSucceeded ||
			state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_CharacterListReady ||
			state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_MapJoinReady;
		if (suspend_connect_server_poll)
		{
			return;
		}

		platform::PollConnectServerBootstrap(&state->connect_server_bootstrap);
			if (state->connect_server_bootstrap.status == platform::ConnectServerBootstrapStatus_Connected &&
				!state->connect_server_bootstrap.server_list_requested)
			{
				if (platform::RequestConnectServerListBootstrap(&state->connect_server_bootstrap))
			{
					RefreshPreviewStatusFromConnectServer(state);
				}
			}
		platform::SyncLegacyLoginServerListRuntime(&state->connect_server_bootstrap);
		NormalizePreviewServerSelection(state);

		const int current_status = static_cast<int>(state->connect_server_bootstrap.status);
		const unsigned int current_attempt = state->connect_server_bootstrap.attempt_count;
		const size_t current_server_count = state->connect_server_bootstrap.server_entries.size();
		const std::string current_message = state->connect_server_bootstrap.status_message;
		if (current_status == state->preview_connect_logged_status &&
			current_attempt == state->preview_connect_logged_attempt &&
			current_server_count == state->preview_connect_logged_server_count &&
			current_message == state->preview_connect_logged_message)
		{
			return;
		}

		state->preview_connect_logged_status = current_status;
		state->preview_connect_logged_attempt = current_attempt;
		state->preview_connect_logged_server_count = current_server_count;
		state->preview_connect_logged_message = current_message;
		RefreshPreviewStatusFromConnectServer(state);
		const std::string endpoint_text =
			state->connect_server_bootstrap.resolved_endpoint.empty() ? BuildConnectServerSummary(state) : state->connect_server_bootstrap.resolved_endpoint;
		__android_log_print(
			ANDROID_LOG_INFO,
			kLogTag,
			"ConnectBootstrap status=%s attempt=%u endpoint=%s socket=%s servers=%zu address=%s:%u message=%s",
			platform::GetConnectServerBootstrapStatusLabel(state->connect_server_bootstrap.status),
			state->connect_server_bootstrap.attempt_count,
			endpoint_text.c_str(),
			state->connect_server_bootstrap.socket_open ? "open" : "closed",
			state->connect_server_bootstrap.server_entries.size(),
			state->connect_server_bootstrap.selected_game_server_host.empty() ? "-" : state->connect_server_bootstrap.selected_game_server_host.c_str(),
			static_cast<unsigned int>(state->connect_server_bootstrap.selected_game_server_port),
			state->connect_server_bootstrap.status_message.c_str());
	}

	void EnsurePreviewGameServer(BootstrapState* state)
	{
		if (state == NULL || !state->connect_server_bootstrap.server_address_received)
		{
			return;
		}

		const std::string effective_host =
			!state->runtime_config.game_server_host.empty()
				? state->runtime_config.game_server_host
				: state->connect_server_bootstrap.selected_game_server_host;
		const unsigned short effective_port =
			state->runtime_config.game_server_port != 0
				? state->runtime_config.game_server_port
				: state->connect_server_bootstrap.selected_game_server_port;

		if (effective_host.empty() || effective_port == 0)
		{
			return;
		}

			const bool needs_reconfigure =
				!state->game_server_bootstrap.configured ||
				state->game_server_bootstrap.host != effective_host ||
				state->game_server_bootstrap.port != effective_port ||
				state->game_server_bootstrap.account != state->preview_account_value ||
				state->game_server_bootstrap.password != state->preview_password_value ||
				state->game_server_bootstrap.language != state->runtime_config.language ||
				state->game_server_bootstrap.client_version != state->core_bootstrap_state.client_version ||
				state->game_server_bootstrap.client_serial != state->core_bootstrap_state.client_serial;

		if (needs_reconfigure)
		{
			if (platform::ConfigureGameServerBootstrap(
					&state->game_server_bootstrap,
					effective_host.c_str(),
						effective_port,
						state->preview_account_value.c_str(),
						state->preview_password_value.c_str(),
						state->runtime_config.language.c_str(),
						state->core_bootstrap_state.client_version.c_str(),
						state->core_bootstrap_state.client_serial.c_str()))
			{
				state->preview_game_logged_status = static_cast<int>(state->game_server_bootstrap.status);
				state->preview_game_logged_attempt = state->game_server_bootstrap.attempt_count;
				state->preview_game_logged_message = state->game_server_bootstrap.status_message;
				__android_log_print(
					ANDROID_LOG_INFO,
					kLogTag,
					"GameServerBootstrap configured=%s target=%s account=%s pass_len=%zu",
					"yes",
					BuildGameServerSummary(state).c_str(),
					state->preview_account_value.empty() ? "-" : state->preview_account_value.c_str(),
					state->preview_password_value.size());
				SetPreviewStatusMessage(state, state->game_server_bootstrap.status_message);
			}
		}
	}

	void PollPreviewGameServer(BootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		EnsurePreviewGameServer(state);

		// Auto-login: if credentials are pre-filled and GameServer is configured but idle, start login
		if (state->game_server_bootstrap.configured &&
			state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_Idle &&
			!state->auto_login_user.empty() &&
			!state->preview_password_value.empty())
		{
			__android_log_print(ANDROID_LOG_INFO, kLogTag, "AutoLogin: triggering login for user=%s", state->auto_login_user.c_str());
			if (state->legacy_login_ui_ready)
			{
				platform::HandleLegacyLoginConfirmAction();
			}
			if (!platform::RequestGameServerLoginBootstrap(&state->game_server_bootstrap))
			{
				__android_log_print(ANDROID_LOG_WARN, kLogTag, "AutoLogin: failed - %s", state->game_server_bootstrap.status_message.c_str());
			}
			state->auto_login_user.clear(); // prevent repeated attempts
		}

		platform::PollGameServerBootstrap(&state->game_server_bootstrap);

		const int current_status = static_cast<int>(state->game_server_bootstrap.status);
		const unsigned int current_attempt = state->game_server_bootstrap.attempt_count;
		const std::string current_message = state->game_server_bootstrap.status_message;
		if (current_status == state->preview_game_logged_status &&
			current_attempt == state->preview_game_logged_attempt &&
			current_message == state->preview_game_logged_message)
		{
			return;
		}

		state->preview_game_logged_status = current_status;
		state->preview_game_logged_attempt = current_attempt;
		state->preview_game_logged_message = current_message;
		RefreshPreviewStatusFromGameServer(state);
		const std::string endpoint_text =
			state->game_server_bootstrap.resolved_endpoint.empty() ? BuildGameServerSummary(state) : state->game_server_bootstrap.resolved_endpoint;
		__android_log_print(
			ANDROID_LOG_INFO,
			kLogTag,
			"GameServerBootstrap status=%s attempt=%u endpoint=%s socket=%s hero=%u join=%s login=%s message=%s",
			platform::GetGameServerBootstrapStatusLabel(state->game_server_bootstrap.status),
			state->game_server_bootstrap.attempt_count,
			endpoint_text.c_str(),
			state->game_server_bootstrap.socket_open ? "open" : "closed",
			static_cast<unsigned int>(state->game_server_bootstrap.hero_key),
			state->game_server_bootstrap.join_server_received ? (state->game_server_bootstrap.join_server_success ? "ok" : "fail") : "pending",
			state->game_server_bootstrap.login_result_received ? "done" : "pending",
			state->game_server_bootstrap.status_message.c_str());
	}

	void ShowSoftKeyboard(android_app* app)
	{
		if (app == NULL || app->activity == NULL)
		{
			return;
		}

		ANativeActivity_showSoftInput(app->activity, ANATIVEACTIVITY_SHOW_SOFT_INPUT_IMPLICIT);
	}

	void HideSoftKeyboard(android_app* app)
	{
		if (app == NULL || app->activity == NULL)
		{
			return;
		}

		ANativeActivity_hideSoftInput(app->activity, ANATIVEACTIVITY_HIDE_SOFT_INPUT_NOT_ALWAYS);
	}

	void SetPreviewFocus(BootstrapState* state, android_app* app, PreviewHitTarget target)
	{
		if (state == NULL)
		{
			return;
		}

		if (!IsPreviewFieldTarget(target))
		{
			target = PreviewHitTarget_None;
		}

		if (state->preview_focused_target == target)
		{
			if (target == PreviewHitTarget_None)
			{
				HideSoftKeyboard(app);
			}
			else
			{
				ShowSoftKeyboard(app);
			}
			return;
		}

		if (state->legacy_login_ui_ready && IsPreviewLoginStage(state))
		{
			if (target == PreviewHitTarget_FieldAccount)
			{
				platform::FocusLegacyLoginAccountField();
			}
			else if (target == PreviewHitTarget_FieldPassword)
			{
				platform::FocusLegacyLoginPasswordField();
			}
			else
			{
				platform::ClearLegacyLoginFieldFocus();
			}
		}

		state->preview_focused_target = target;
		if (target == PreviewHitTarget_None)
		{
			HideSoftKeyboard(app);
			if (IsPreviewLoginStage(state))
			{
				SetPreviewStatusMessage(state, "Toque nos campos para editar");
			}
			else if (state->connect_server_bootstrap.server_list_received)
			{
				SetPreviewStatusMessage(state, "Selecione uma sala");
			}
			return;
		}

		ShowSoftKeyboard(app);
		SetPreviewStatusMessage(state, std::string("Editando ") + GetPreviewTargetLabel(state, target));
	}

	std::string BuildPreviewPasswordMask(const std::string& value)
	{
		return std::string(value.size(), '*');
	}

	bool IsShiftPressed(int32_t meta_state)
	{
		return (meta_state & (AMETA_SHIFT_ON | AMETA_SHIFT_LEFT_ON | AMETA_SHIFT_RIGHT_ON)) != 0;
	}

	bool IsCapsLockEnabled(int32_t meta_state)
	{
		return (meta_state & AMETA_CAPS_LOCK_ON) != 0;
	}

	char TranslateAndroidKeyCodeToAscii(int32_t key_code, int32_t meta_state)
	{
		if (key_code >= AKEYCODE_A && key_code <= AKEYCODE_Z)
		{
			const bool upper = IsShiftPressed(meta_state) ^ IsCapsLockEnabled(meta_state);
			const char base = upper ? 'A' : 'a';
			return static_cast<char>(base + (key_code - AKEYCODE_A));
		}

		if (key_code >= AKEYCODE_0 && key_code <= AKEYCODE_9)
		{
			static const char kShiftDigits[10] = { ')', '!', '@', '#', '$', '%', '^', '&', '*', '(' };
			const int index = key_code - AKEYCODE_0;
			return IsShiftPressed(meta_state) ? kShiftDigits[index] : static_cast<char>('0' + index);
		}

		switch (key_code)
		{
		case AKEYCODE_SPACE:
			return ' ';
		case AKEYCODE_PERIOD:
			return IsShiftPressed(meta_state) ? '>' : '.';
		case AKEYCODE_COMMA:
			return IsShiftPressed(meta_state) ? '<' : ',';
		case AKEYCODE_MINUS:
			return IsShiftPressed(meta_state) ? '_' : '-';
		case AKEYCODE_EQUALS:
			return IsShiftPressed(meta_state) ? '+' : '=';
		case AKEYCODE_SEMICOLON:
			return IsShiftPressed(meta_state) ? ':' : ';';
		case AKEYCODE_APOSTROPHE:
			return IsShiftPressed(meta_state) ? '"' : '\'';
		case AKEYCODE_SLASH:
			return IsShiftPressed(meta_state) ? '?' : '/';
		case AKEYCODE_BACKSLASH:
			return IsShiftPressed(meta_state) ? '|' : '\\';
		case AKEYCODE_LEFT_BRACKET:
			return IsShiftPressed(meta_state) ? '{' : '[';
		case AKEYCODE_RIGHT_BRACKET:
			return IsShiftPressed(meta_state) ? '}' : ']';
		case AKEYCODE_GRAVE:
			return IsShiftPressed(meta_state) ? '~' : '`';
		case AKEYCODE_AT:
			return '@';
		case AKEYCODE_POUND:
			return '#';
		default:
			return '\0';
		}
	}

	std::string* GetFocusedPreviewFieldValue(BootstrapState* state)
	{
		if (state == NULL)
		{
			return NULL;
		}

		if (state->preview_focused_target == PreviewHitTarget_FieldAccount)
		{
			return &state->preview_account_value;
		}
		if (state->preview_focused_target == PreviewHitTarget_FieldPassword)
		{
			return &state->preview_password_value;
		}

		return NULL;
	}

	bool AppendPreviewCharacter(BootstrapState* state, char value)
	{
		if (state != NULL && state->legacy_login_ui_ready && IsPreviewLoginStage(state))
		{
			return platform::AppendLegacyLoginFieldCharacter(value);
		}

		std::string* field_value = GetFocusedPreviewFieldValue(state);
		if (field_value == NULL)
		{
			return false;
		}

		if (value < 32 || value > 126)
		{
			return false;
		}

		const size_t max_length = 16;
		if (field_value->size() >= max_length)
		{
			return false;
		}

		field_value->push_back(value);
		return true;
	}

	bool ErasePreviewCharacter(BootstrapState* state)
	{
		if (state != NULL && state->legacy_login_ui_ready && IsPreviewLoginStage(state))
		{
			return platform::BackspaceLegacyLoginFieldCharacter();
		}

		std::string* field_value = GetFocusedPreviewFieldValue(state);
		if (field_value == NULL || field_value->empty())
		{
			return false;
		}

		field_value->erase(field_value->size() - 1);
		return true;
	}

	std::string FitPreviewText(const std::string& value, size_t max_characters, bool keep_tail)
	{
		if (value.size() <= max_characters || max_characters == 0)
		{
			return value;
		}

		if (max_characters <= 3)
		{
			return keep_tail ? value.substr(value.size() - max_characters) : value.substr(0, max_characters);
		}

		if (keep_tail)
		{
			return std::string("...") + value.substr(value.size() - (max_characters - 3));
		}

		return value.substr(0, max_characters - 3) + "...";
	}

	bool DirectoryExists(const char* path)
	{
		if (path == NULL || path[0] == '\0')
		{
			return false;
		}

		struct stat status = {};
		return stat(path, &status) == 0 && S_ISDIR(status.st_mode);
	}

	bool FileExists(const char* path)
	{
		if (path == NULL || path[0] == '\0')
		{
			return false;
		}

		FILE* file = fopen(path, "rb");
		if (file == NULL)
		{
			return false;
		}

		fclose(file);
		return true;
	}

	bool EnsureDirectoryExists(const char* path)
	{
		if (path == NULL || path[0] == '\0')
		{
			return false;
		}

		if (DirectoryExists(path))
		{
			return true;
		}

		std::string parent = path;
		while (!parent.empty() && (parent[parent.size() - 1] == '/' || parent[parent.size() - 1] == '\\'))
		{
			parent.erase(parent.size() - 1);
		}

		const size_t slash = parent.find_last_of("/\\");
		if (slash != std::string::npos)
		{
			const std::string parent_path = slash == 0 ? std::string("/") : parent.substr(0, slash);
			if (!parent_path.empty() && !DirectoryExists(parent_path.c_str()))
			{
				if (!EnsureDirectoryExists(parent_path.c_str()))
				{
					return false;
				}
			}
		}

		if (mkdir(path, 0755) == 0)
		{
			return true;
		}

		return errno == EEXIST && DirectoryExists(path);
	}

	std::string JoinPath(const char* left, const char* right)
	{
		if (left == NULL || left[0] == '\0')
		{
			return right != NULL ? std::string(right) : std::string();
		}

		std::string result = left;
		while (!result.empty() && (result[result.size() - 1] == '/' || result[result.size() - 1] == '\\'))
		{
			result.erase(result.size() - 1);
		}

		if (right != NULL && right[0] != '\0')
		{
			result += "/";
			result += right;
		}

		return result;
	}

	std::string GetParentDirectory(const std::string& path)
	{
		const size_t slash = path.find_last_of("/\\");
		if (slash == std::string::npos)
		{
			return std::string();
		}

		if (slash == 0)
		{
			return std::string("/");
		}

		return path.substr(0, slash);
	}

	bool AssetFileExists(AAssetManager* asset_manager, const char* asset_path)
	{
		if (asset_manager == NULL || asset_path == NULL || asset_path[0] == '\0')
		{
			return false;
		}

		AAsset* asset = AAssetManager_open(asset_manager, asset_path, AASSET_MODE_STREAMING);
		if (asset == NULL)
		{
			return false;
		}

		AAsset_close(asset);
		return true;
	}

	bool ReadTextAsset(AAssetManager* asset_manager, const char* asset_path, std::string* content)
	{
		if (content == NULL)
		{
			return false;
		}

		content->clear();
		if (asset_manager == NULL || asset_path == NULL || asset_path[0] == '\0')
		{
			return false;
		}

		AAsset* asset = AAssetManager_open(asset_manager, asset_path, AASSET_MODE_STREAMING);
		if (asset == NULL)
		{
			return false;
		}

		const off_t asset_length = AAsset_getLength(asset);
		if (asset_length > 0)
		{
			content->resize(static_cast<size_t>(asset_length));
			const int bytes_read = AAsset_read(asset, &(*content)[0], static_cast<size_t>(asset_length));
			if (bytes_read < 0)
			{
				content->clear();
				AAsset_close(asset);
				return false;
			}
			if (bytes_read >= 0 && static_cast<size_t>(bytes_read) < content->size())
			{
				content->resize(static_cast<size_t>(bytes_read));
			}
		}

		AAsset_close(asset);
		return true;
	}

	bool ReadTextFile(const char* file_path, std::string* content)
	{
		if (content == NULL)
		{
			return false;
		}

		content->clear();
		if (file_path == NULL || file_path[0] == '\0')
		{
			return false;
		}

		FILE* file = fopen(file_path, "rb");
		if (file == NULL)
		{
			return false;
		}

		if (fseek(file, 0, SEEK_END) != 0)
		{
			fclose(file);
			return false;
		}

		const long file_size = ftell(file);
		if (file_size < 0)
		{
			fclose(file);
			return false;
		}

		rewind(file);
		content->resize(static_cast<size_t>(file_size));
		if (file_size > 0)
		{
			const size_t bytes_read = fread(&(*content)[0], 1, static_cast<size_t>(file_size), file);
			if (bytes_read != static_cast<size_t>(file_size))
			{
				content->clear();
				fclose(file);
				return false;
			}
		}

		fclose(file);
		return true;
	}

	bool GetAssetDirectoryEntries(AAssetManager* asset_manager, const char* asset_dir, std::vector<std::string>* entries)
	{
		if (entries == NULL)
		{
			return false;
		}

		entries->clear();
		if (asset_manager == NULL || asset_dir == NULL || asset_dir[0] == '\0')
		{
			return false;
		}

		AAssetDir* directory = AAssetManager_openDir(asset_manager, asset_dir);
		if (directory == NULL)
		{
			return false;
		}

		const char* name = NULL;
		while ((name = AAssetDir_getNextFileName(directory)) != NULL)
		{
			entries->push_back(name);
		}

		AAssetDir_close(directory);
		return !entries->empty();
	}

	bool CopyAssetFileToPath(AAssetManager* asset_manager, const std::string& asset_path, const std::string& destination_path)
	{
		AAsset* asset = AAssetManager_open(asset_manager, asset_path.c_str(), AASSET_MODE_STREAMING);
		if (asset == NULL)
		{
			return false;
		}

		const std::string parent_directory = GetParentDirectory(destination_path);
		if (!parent_directory.empty() && !EnsureDirectoryExists(parent_directory.c_str()))
		{
			AAsset_close(asset);
			return false;
		}

		FILE* output = fopen(destination_path.c_str(), "wb");
		if (output == NULL)
		{
			AAsset_close(asset);
			return false;
		}

		char buffer[64 * 1024];
		bool success = true;
		for (;;)
		{
			const int bytes_read = AAsset_read(asset, buffer, sizeof(buffer));
			if (bytes_read < 0)
			{
				success = false;
				break;
			}
			if (bytes_read == 0)
			{
				break;
			}
			if (fwrite(buffer, 1, static_cast<size_t>(bytes_read), output) != static_cast<size_t>(bytes_read))
			{
				success = false;
				break;
			}
		}

		fclose(output);
		AAsset_close(asset);
		return success;
	}

	bool ExtractAssetsFromManifest(AAssetManager* asset_manager, const std::string& manifest_text, const std::string& destination_root)
	{
		if (asset_manager == NULL || manifest_text.empty() || destination_root.empty())
		{
			return false;
		}

		if (!EnsureDirectoryExists(destination_root.c_str()))
		{
			return false;
		}

		std::istringstream stream(manifest_text);
		std::string line;
		bool extracted_any = false;
		bool success = true;
		int file_count = 0;

		while (std::getline(stream, line))
		{
			if (line.empty())
			{
				continue;
			}

			const size_t separator = line.find('|');
			const std::string relative_path = separator == std::string::npos ? line : line.substr(0, separator);
			if (relative_path.empty())
			{
				continue;
			}

			const std::string asset_path = JoinPath(kPackagedDataAssetRoot, relative_path.c_str());
			const std::string destination_path = JoinPath(destination_root.c_str(), relative_path.c_str());
			if (!CopyAssetFileToPath(asset_manager, asset_path, destination_path))
			{
				__android_log_print(ANDROID_LOG_WARN, kLogTag, "Failed to extract asset: %s", asset_path.c_str());
				success = false;
				continue;
			}

			extracted_any = true;
			++file_count;
		}

		__android_log_print(ANDROID_LOG_INFO, kLogTag, "Manifest extraction copied %d files", file_count);

		return extracted_any && success;
	}

	bool ExtractAssetTree(AAssetManager* asset_manager, const std::string& asset_root, const std::string& destination_root)
	{
		std::vector<std::string> entries;
		if (!GetAssetDirectoryEntries(asset_manager, asset_root.c_str(), &entries))
		{
			return false;
		}

		if (!EnsureDirectoryExists(destination_root.c_str()))
		{
			return false;
		}

		bool success = true;
		for (size_t i = 0; i < entries.size(); ++i)
		{
			const std::string child_asset_path = JoinPath(asset_root.c_str(), entries[i].c_str());
			const std::string child_destination_path = JoinPath(destination_root.c_str(), entries[i].c_str());

			std::vector<std::string> child_entries;
			if (GetAssetDirectoryEntries(asset_manager, child_asset_path.c_str(), &child_entries))
			{
				if (!ExtractAssetTree(asset_manager, child_asset_path, child_destination_path))
				{
					success = false;
				}
				continue;
			}

			if (!CopyAssetFileToPath(asset_manager, child_asset_path, child_destination_path))
			{
				success = false;
			}
		}

		return success;
	}

	bool WriteMarkerFile(const std::string& marker_path)
	{
		const std::string parent_directory = GetParentDirectory(marker_path);
		if (!parent_directory.empty() && !EnsureDirectoryExists(parent_directory.c_str()))
		{
			return false;
		}

		FILE* file = fopen(marker_path.c_str(), "wb");
		if (file == NULL)
		{
			return false;
		}

		fputs("ready\n", file);
		fclose(file);
		return true;
	}

	void ConfigureGameDataRoot(BootstrapState* state, android_app* app)
	{
		if (state == NULL || app == NULL || app->activity == NULL)
		{
			return;
		}

		const std::string internal_data_root =
			(app->activity->internalDataPath != NULL && app->activity->internalDataPath[0] != '\0')
			? JoinPath(app->activity->internalDataPath, "Data")
			: std::string();
		const std::string external_data_root =
			(app->activity->externalDataPath != NULL && app->activity->externalDataPath[0] != '\0')
			? JoinPath(app->activity->externalDataPath, "Data")
			: std::string();
		const bool has_packaged_data = AssetFileExists(app->activity->assetManager, kPackagedConfigProbeAsset);

		std::string selected_root;
		if (has_packaged_data && !internal_data_root.empty())
		{
			const std::string marker_path = JoinPath(internal_data_root.c_str(), kExtractReadyMarker);
			const std::string extracted_config_path = JoinPath(internal_data_root.c_str(), "Configs/Configs.xtm");
			const std::string extracted_manifest_path = JoinPath(internal_data_root.c_str(), "mu_asset_manifest.txt");
			std::string packaged_manifest;
			std::string extracted_manifest;
			const bool has_packaged_manifest = ReadTextAsset(app->activity->assetManager, kPackagedManifestAsset, &packaged_manifest);
			__android_log_print(ANDROID_LOG_INFO, kLogTag, "Packaged manifest = %s", has_packaged_manifest ? "yes" : "no");
			bool extraction_needed = !FileExists(extracted_config_path.c_str());
			if (!extraction_needed)
			{
				if (has_packaged_manifest)
				{
					extraction_needed = !ReadTextFile(extracted_manifest_path.c_str(), &extracted_manifest)
						|| extracted_manifest != packaged_manifest;
				}
				else
				{
					extraction_needed = !FileExists(marker_path.c_str());
				}
			}

			if (extraction_needed)
			{
				__android_log_print(ANDROID_LOG_INFO, kLogTag, "Extracting packaged Data to %s", internal_data_root.c_str());
				bool extraction_ok = false;
				if (has_packaged_manifest)
				{
					__android_log_print(ANDROID_LOG_INFO, kLogTag, "Using packaged manifest for extraction");
					extraction_ok = ExtractAssetsFromManifest(app->activity->assetManager, packaged_manifest, internal_data_root);
				}
				else
				{
					__android_log_print(ANDROID_LOG_INFO, kLogTag, "Using directory walk fallback for extraction");
					extraction_ok = ExtractAssetTree(app->activity->assetManager, kPackagedDataAssetRoot, internal_data_root);
				}

				if (extraction_ok && FileExists(extracted_config_path.c_str()))
				{
					if (!has_packaged_manifest)
					{
						WriteMarkerFile(marker_path);
					}
				}
				else
				{
					__android_log_print(ANDROID_LOG_WARN, kLogTag, "Packaged Data extraction did not produce Configs.xtm");
				}
			}

			selected_root = internal_data_root;
		}

		std::vector<std::string> candidates;
		if (selected_root.empty() && !external_data_root.empty())
		{
			candidates.push_back(external_data_root);
		}
		if (selected_root.empty() && !internal_data_root.empty())
		{
			candidates.push_back(internal_data_root);
		}

		for (size_t i = 0; selected_root.empty() && i < candidates.size(); ++i)
		{
			if (DirectoryExists(candidates[i].c_str()))
			{
				selected_root = candidates[i];
			}
		}

		if (selected_root.empty())
		{
			if (!external_data_root.empty())
			{
				selected_root = external_data_root;
			}
			else if (!internal_data_root.empty())
			{
				selected_root = internal_data_root;
			}
			else
			{
				selected_root = "Data";
			}
		}

			platform::SetGameAssetRootOverride(selected_root.c_str());
			platform::InitializeGameAssetRoot();
			platform::SetLegacyClientAppDataRoot(app->activity != NULL ? app->activity->internalDataPath : NULL);

			const std::string configs_path = platform::ResolveGameAssetPath("Data/Configs/Configs.xtm");
		state->has_game_data = FileExists(configs_path.c_str());

		__android_log_print(ANDROID_LOG_INFO, kLogTag, "Game asset root = %s", platform::GetGameAssetRoot());
		__android_log_print(ANDROID_LOG_INFO, kLogTag, "Packaged Data = %s", has_packaged_data ? "yes" : "no");
		__android_log_print(ANDROID_LOG_INFO, kLogTag, "Configs.xtm = %s", state->has_game_data ? "found" : "missing");
	}

	std::string ExtractPackagedClientConfig(android_app* app)
	{
		if (app == NULL || app->activity == NULL || app->activity->internalDataPath == NULL)
		{
			return std::string();
		}

		if (!AssetFileExists(app->activity->assetManager, kPackagedClientConfigAsset))
		{
			__android_log_print(ANDROID_LOG_INFO, kLogTag, "Packaged config.ini = no");
			return std::string();
		}

		const std::string destination_path = JoinPath(app->activity->internalDataPath, "config.ini");
		if (!CopyAssetFileToPath(app->activity->assetManager, kPackagedClientConfigAsset, destination_path))
		{
			__android_log_print(ANDROID_LOG_WARN, kLogTag, "Failed to extract config.ini to %s", destination_path.c_str());
			return std::string();
		}

		__android_log_print(ANDROID_LOG_INFO, kLogTag, "Packaged config.ini = yes");
		__android_log_print(ANDROID_LOG_INFO, kLogTag, "config.ini path = %s", destination_path.c_str());
		return destination_path;
	}

	std::string ExtractPackagedRuntimeConfig(android_app* app)
	{
		if (app == NULL || app->activity == NULL || app->activity->internalDataPath == NULL)
		{
			return std::string();
		}

		const std::string destination_path = JoinPath(app->activity->internalDataPath, "client_runtime.ini");
		if (!AssetFileExists(app->activity->assetManager, kPackagedRuntimeConfigAsset))
		{
			return destination_path;
		}

		if (!CopyAssetFileToPath(app->activity->assetManager, kPackagedRuntimeConfigAsset, destination_path))
		{
			__android_log_print(ANDROID_LOG_WARN, kLogTag, "Failed to extract client_runtime.ini to %s", destination_path.c_str());
			return destination_path;
		}

		__android_log_print(ANDROID_LOG_INFO, kLogTag, "Packaged client_runtime.ini = yes");
		__android_log_print(ANDROID_LOG_INFO, kLogTag, "client_runtime.ini path = %s", destination_path.c_str());
		return destination_path;
	}

	void BootstrapClientIniConfig(BootstrapState* state, const std::string& config_path)
	{
		if (state == NULL)
		{
			return;
		}

		state->has_client_config = !config_path.empty();
		state->has_client_ini_bootstrap = false;
		if (config_path.empty())
		{
			return;
		}

		platform::ClientIniConfigState client_ini_state;
		if (!platform::LoadClientIniConfig(config_path.c_str(), &client_ini_state))
		{
			__android_log_print(ANDROID_LOG_WARN, kLogTag, "Client config bootstrap failed: %s", config_path.c_str());
			return;
		}

		state->has_client_ini_bootstrap = true;
		state->auto_login_user = client_ini_state.auto_login_user;
		state->preview_account_value = client_ini_state.auto_login_user;
		state->preview_password_value = client_ini_state.auto_login_password;
		platform::ApplyLegacyClientIniConfig(&client_ini_state);
		__android_log_print(ANDROID_LOG_INFO, kLogTag, "ConfigVersion = %s", client_ini_state.login_version.c_str());
		__android_log_print(ANDROID_LOG_INFO, kLogTag, "TestVersion = %s", client_ini_state.test_version.c_str());
		if (!client_ini_state.auto_login_user.empty())
		{
			__android_log_print(ANDROID_LOG_INFO, kLogTag, "ConfigAutoLoginUser = %s", client_ini_state.auto_login_user.c_str());
		}
	}

	void BootstrapClientRuntimeConfig(BootstrapState* state, const std::string& runtime_config_path)
	{
		if (state == NULL)
		{
			return;
		}

		state->has_runtime_config = platform::InitializeClientRuntimeConfig(
			runtime_config_path.empty() ? NULL : runtime_config_path.c_str(),
			&state->runtime_config);
		platform::ApplyLegacyClientRuntimeConfig(&state->runtime_config);
		if (state->runtime_config.has_auto_login_user_override)
		{
			state->auto_login_user = state->runtime_config.auto_login_user;
			state->preview_account_value = state->runtime_config.auto_login_user;
		}
		if (state->runtime_config.has_auto_login_password_override)
		{
			state->preview_password_value = state->runtime_config.auto_login_password;
		}
		RefreshGameMouseMetrics(state);
		platform::ResetGameMouseState(
			&state->game_mouse_state,
			state->game_mouse_metrics.max_mouse_x / 2,
			state->game_mouse_metrics.max_mouse_y / 2);
		platform::SyncLegacyClientMouseState(&state->game_mouse_state);
		__android_log_print(
			ANDROID_LOG_INFO,
			kLogTag,
			"RuntimeConfig = %s found=%s lang=%s res=%d window=%ux%u scale=%.3f window_mode=%s sound=%d music=%d connect_override=%s:%u",
			state->runtime_config.source_description.c_str(),
			state->runtime_config.found ? "yes" : "no",
			state->runtime_config.language.c_str(),
			state->runtime_config.resolution,
			state->runtime_config.window_width,
			state->runtime_config.window_height,
			state->runtime_config.screen_rate_y,
			state->runtime_config.window_mode ? "yes" : "no",
			state->runtime_config.sound_on,
			state->runtime_config.music_on,
			state->runtime_config.connect_server_host.empty() ? "-" : state->runtime_config.connect_server_host.c_str(),
			static_cast<unsigned int>(state->runtime_config.connect_server_port));
		if (state->runtime_config.has_auto_login_user_override || state->runtime_config.has_auto_login_password_override)
		{
			__android_log_print(
				ANDROID_LOG_INFO,
				kLogTag,
				"RuntimeAutoLoginOverride = user=%s pass_len=%zu",
				state->preview_account_value.empty() ? "-" : state->preview_account_value.c_str(),
				state->preview_password_value.size());
		}
		if (!state->runtime_config.game_server_host.empty() || state->runtime_config.game_server_port != 0)
		{
			__android_log_print(
				ANDROID_LOG_INFO,
				kLogTag,
				"RuntimeGameServerOverride = %s:%u",
				state->runtime_config.game_server_host.empty() ? "-" : state->runtime_config.game_server_host.c_str(),
				static_cast<unsigned int>(state->runtime_config.game_server_port));
		}
		if (!state->runtime_config.auto_login_user.empty() || !state->runtime_config.auto_login_password.empty())
		{
			__android_log_print(
				ANDROID_LOG_INFO,
				kLogTag,
				"RuntimeLoginOverride = user=%s pass_len=%zu",
				state->runtime_config.auto_login_user.empty() ? "-" : state->runtime_config.auto_login_user.c_str(),
				state->runtime_config.auto_login_password.size());
		}
	}

	void BootstrapLanguageAssets(BootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		state->has_language_assets = platform::InitializeLanguageAssetBootstrap(
			state->runtime_config.language.c_str(),
			&state->language_assets);

		if (state->has_language_assets)
		{
			if (!state->language_assets.resolved_language.empty())
			{
				state->runtime_config.language = state->language_assets.resolved_language;
			}

			__android_log_print(
				ANDROID_LOG_INFO,
				kLogTag,
				"LanguageAssets = request=%s resolved=%s fallback=%s text=%s dialog=%s item=%s",
				state->language_assets.requested_language.c_str(),
				state->language_assets.resolved_language.c_str(),
				state->language_assets.used_fallback ? "yes" : "no",
				state->language_assets.text_path.c_str(),
				state->language_assets.dialog_path.c_str(),
				state->language_assets.item_path.c_str());
			return;
		}

		__android_log_print(
			ANDROID_LOG_WARN,
			kLogTag,
			"LanguageAssets bootstrap failed: request=%s reason=%s",
			state->language_assets.requested_language.c_str(),
			state->language_assets.error_message.c_str());
	}

		void BootstrapGlobalText(BootstrapState* state)
		{
		if (state == NULL)
		{
			return;
		}

		state->has_global_text_bootstrap = false;
		if (!state->has_language_assets)
		{
			return;
		}

		state->has_global_text_bootstrap = platform::InitializeGlobalTextBootstrap(
			state->language_assets.text_path.c_str(),
			&state->global_text);

		if (!state->has_global_text_bootstrap)
		{
			__android_log_print(
				ANDROID_LOG_WARN,
				kLogTag,
				"GlobalText bootstrap failed: %s",
				state->global_text.error_message.c_str());
			return;
		}

		__android_log_print(
			ANDROID_LOG_INFO,
			kLogTag,
			"GlobalText = path=%s entries=%zu ok=%s cancel=%s close=%s recipient=%s",
				state->global_text.source_path.c_str(),
				state->global_text.loaded_entry_count,
				platform::FindGlobalTextEntry(&state->global_text, 228),
				platform::FindGlobalTextEntry(&state->global_text, 229),
				platform::FindGlobalTextEntry(&state->global_text, 1002),
				platform::FindGlobalTextEntry(&state->global_text, 1000));

			std::string legacy_runtime_error;
			const bool legacy_runtime_ready =
				platform::InitializeLegacyLoginServerListRuntime(&state->global_text, &legacy_runtime_error);
			__android_log_print(
				legacy_runtime_ready ? ANDROID_LOG_INFO : ANDROID_LOG_WARN,
				kLogTag,
				"LegacyLoginServerList = ready=%s reason=%s",
				legacy_runtime_ready ? "yes" : "no",
				legacy_runtime_error.empty() ? "-" : legacy_runtime_error.c_str());
		}

		void BootstrapServerListScript(BootstrapState* state)
		{
			if (state == NULL)
			{
				return;
			}

			state->has_server_list_script = false;
			state->server_list_script = PreviewServerListScriptState();
			state->server_list_script.loaded = false;
			state->server_list_script.source_path = platform::ResolveGameAssetPath("Data/Local/ServerList.bmd");

			FILE* file = fopen(state->server_list_script.source_path.c_str(), "rb");
			if (file == NULL)
			{
				state->server_list_script.error_message = "ServerList.bmd ausente";
				__android_log_print(
					ANDROID_LOG_WARN,
					kLogTag,
					"ServerListScript bootstrap failed: path=%s reason=%s",
					state->server_list_script.source_path.c_str(),
					state->server_list_script.error_message.c_str());
				return;
			}

#pragma pack(push, 1)
			struct PackedServerGroupInfo
			{
				unsigned short group_index;
				char name[kPreviewServerListScriptMaxNameLength];
				unsigned char position;
				unsigned char sequence;
				unsigned char non_pvp[kPreviewServerListScriptMaxServerCount];
				short description_length;
			};
#pragma pack(pop)

			PackedServerGroupInfo packed_group = {};
			while (fread(&packed_group, sizeof(packed_group), 1, file) == 1)
			{
				BuxConvertPreviewServerList(reinterpret_cast<unsigned char*>(&packed_group), sizeof(packed_group));
				if (packed_group.description_length < 0 || packed_group.description_length > 2048)
				{
					state->server_list_script.error_message = "Description length invalido";
					break;
				}

				std::vector<char> description(static_cast<size_t>(packed_group.description_length) + 1u, '\0');
				if (packed_group.description_length > 0)
				{
					if (fread(&description[0], static_cast<size_t>(packed_group.description_length), 1, file) != 1)
					{
						state->server_list_script.error_message = "Falha ao ler descricao";
						break;
					}

					BuxConvertPreviewServerList(reinterpret_cast<unsigned char*>(&description[0]), static_cast<size_t>(packed_group.description_length));
				}

				PreviewServerGroupScript group = {};
				group.group_index = packed_group.group_index;
				group.name.assign(packed_group.name, packed_group.name + kPreviewServerListScriptMaxNameLength);
				const std::string::size_type zero_position = group.name.find('\0');
				if (zero_position != std::string::npos)
				{
					group.name.resize(zero_position);
				}
				group.position = packed_group.position;
				group.sequence = packed_group.sequence;
				for (int index = 0; index < kPreviewServerListScriptMaxServerCount; ++index)
				{
					group.non_pvp[index] = packed_group.non_pvp[index];
				}
				group.description.assign(description.data());
				state->server_list_script.groups.push_back(group);
			}

			fclose(file);

			if (!state->server_list_script.error_message.empty())
			{
				__android_log_print(
					ANDROID_LOG_WARN,
					kLogTag,
					"ServerListScript bootstrap failed: path=%s reason=%s",
					state->server_list_script.source_path.c_str(),
					state->server_list_script.error_message.c_str());
				return;
			}

			state->has_server_list_script = !state->server_list_script.groups.empty();
			state->server_list_script.loaded = state->has_server_list_script;
			__android_log_print(
				ANDROID_LOG_INFO,
				kLogTag,
				"ServerListScript = path=%s groups=%zu first=%s",
				state->server_list_script.source_path.c_str(),
				state->server_list_script.groups.size(),
				state->server_list_script.groups.empty() ? "-" : state->server_list_script.groups[0].name.c_str());
		}

		void DestroyInterfacePreviewTextures(BootstrapState* state)
		{
		if (state == NULL)
		{
			return;
		}

		platform::DestroyBootstrapTexture(&state->interface_title_texture);
		platform::DestroyBootstrapTexture(&state->interface_title_glow_texture);
		platform::DestroyBootstrapTexture(&state->interface_background_left_texture);
		platform::DestroyBootstrapTexture(&state->interface_background_right_texture);
		platform::DestroyBootstrapTexture(&state->interface_scene_chrome_texture);
		platform::DestroyBootstrapTexture(&state->interface_scene_swirl_texture);
		platform::DestroyBootstrapTexture(&state->interface_scene_leaf_texture);
		platform::DestroyBootstrapTexture(&state->interface_scene_leaf_alt_texture);
		platform::DestroyBootstrapTexture(&state->interface_scene_wave_texture);
		platform::DestroyBootstrapTexture(&state->interface_scene_cloud_texture);
		platform::DestroyBootstrapTexture(&state->interface_scene_horizon_texture);
		platform::DestroyBootstrapTexture(&state->interface_scene_cloud_band_texture);
		platform::DestroyBootstrapTexture(&state->interface_scene_water_texture);
		platform::DestroyBootstrapTexture(&state->interface_scene_water_detail_texture);
		platform::DestroyBootstrapTexture(&state->interface_scene_land_texture);
		platform::DestroyBootstrapTexture(&state->interface_login_window_texture);
		platform::DestroyBootstrapTexture(&state->interface_medium_button_texture);
		platform::DestroyBootstrapTexture(&state->interface_server_group_button_texture);
		platform::DestroyBootstrapTexture(&state->interface_server_button_texture);
		platform::DestroyBootstrapTexture(&state->interface_server_gauge_texture);
		platform::DestroyBootstrapTexture(&state->interface_server_deco_texture);
		platform::DestroyBootstrapTexture(&state->interface_server_desc_fill_texture);
		platform::DestroyBootstrapTexture(&state->interface_server_desc_cap_texture);
		platform::DestroyBootstrapTexture(&state->interface_server_desc_corner_texture);
		platform::DestroyBootstrapTexture(&state->interface_credit_logo_texture);
		platform::DestroyBootstrapTexture(&state->interface_bottom_deco_texture);
		platform::DestroyBootstrapTexture(&state->interface_menu_button_texture);
		platform::DestroyBootstrapTexture(&state->interface_credit_button_texture);
		state->preview_pressed_target = PreviewHitTarget_None;
		state->preview_focused_target = PreviewHitTarget_None;
		state->interface_background_uses_legacy_login = false;
		state->interface_uses_object95_scene = false;
		ResetPreviewHitAreas(state);
		state->has_interface_preview = false;
	}

	void BootstrapInterfacePreview(BootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		state->has_interface_preview = false;
		state->preview_pressed_target = PreviewHitTarget_None;
		state->preview_focused_target = PreviewHitTarget_None;
		state->interface_background_uses_legacy_login = false;
		state->interface_uses_object95_scene = false;
		ResetPreviewHitAreas(state);
		DestroyInterfacePreviewTextures(state);

		int loaded_count = 0;

		auto load_texture_candidates =
			[&](const char* const* relative_paths,
				size_t relative_path_count,
				platform::BootstrapTexture* texture,
				const char* label,
				bool mark_legacy_background,
				bool mark_object95_scene) -> bool
		{
			std::string attempted_path;
			std::string attempted_error;
			for (size_t index = 0; index < relative_path_count; ++index)
			{
				const std::string resolved_path = platform::ResolveGameAssetPath(relative_paths[index]);
				attempted_path = resolved_path;
				if (platform::LoadBootstrapTextureFromFile(resolved_path.c_str(), texture))
				{
					loaded_count += 1;
					if (mark_legacy_background && index == 0)
					{
						state->interface_background_uses_legacy_login = true;
					}
					if (mark_object95_scene && index == 0)
					{
						state->interface_uses_object95_scene = true;
					}
					__android_log_print(
						ANDROID_LOG_INFO,
						kLogTag,
						"InterfacePreviewTexture = %s %dx%d path=%s",
						label,
						texture->width,
						texture->height,
						texture->source_path.c_str());
					return true;
				}
				attempted_error = texture->error_message;
			}

			__android_log_print(
				ANDROID_LOG_WARN,
				kLogTag,
				"InterfacePreviewTexture failed: %s reason=%s path=%s",
				label,
				attempted_error.c_str(),
				attempted_path.c_str());
			return false;
		};

		const char* background_left_candidates[] =
		{
			"Logo/Login_Back01.OZJ",
			"Logo/New_Login_Back01.OZJ",
		};
		const char* background_right_candidates[] =
		{
			"Logo/Login_Back02.OZJ",
			"Logo/New_Login_Back02.OZJ",
		};
		load_texture_candidates(background_left_candidates, 2, &state->interface_background_left_texture, "Login_Back01", true, false);
		load_texture_candidates(background_right_candidates, 2, &state->interface_background_right_texture, "Login_Back02", true, false);

		struct TextureRequest
		{
			const char* relative_path;
			platform::BootstrapTexture* texture;
			const char* label;
		};

		TextureRequest requests[] =
		{
			{ "World78/bg_b_05.OZJ", &state->interface_scene_chrome_texture, "bg_b_05" },
			{ "World78/bg_b_08.OZJ", &state->interface_scene_swirl_texture, "bg_b_08" },
			{ "Custom/NewInterface/login_back.ozj", &state->interface_login_window_texture, "login_back" },
			{ "Custom/NewInterface/btn_medium.ozj", &state->interface_medium_button_texture, "btn_medium" },
			{ "Interface/cha_bt.OZT", &state->interface_server_group_button_texture, "cha_bt" },
			{ "Interface/server_b2_all.OZT", &state->interface_server_button_texture, "server_b2_all" },
			{ "Interface/server_b2_loding.OZJ", &state->interface_server_gauge_texture, "server_b2_loding" },
			{ "Interface/server_deco_all.OZT", &state->interface_server_deco_texture, "server_deco_all" },
			{ "Interface/server_ex01.OZT", &state->interface_server_desc_fill_texture, "server_ex01" },
			{ "Interface/server_ex02.OZJ", &state->interface_server_desc_cap_texture, "server_ex02" },
			{ "Interface/server_ex03.OZT", &state->interface_server_desc_corner_texture, "server_ex03" },
			{ "Interface/cr_mu_lo.OZT", &state->interface_credit_logo_texture, "cr_mu_lo" },
			{ "Interface/deco.OZT", &state->interface_bottom_deco_texture, "deco" },
			{ "Interface/server_menu_b_all.OZT", &state->interface_menu_button_texture, "server_menu_b_all" },
			{ "Interface/server_credit_b_all.OZT", &state->interface_credit_button_texture, "server_credit_b_all" },
		};

		for (size_t index = 0; index < sizeof(requests) / sizeof(requests[0]); ++index)
		{
			const char* request_path = requests[index].relative_path;
			load_texture_candidates(&request_path, 1, requests[index].texture, requests[index].label, false, false);
		}

		const char* title_candidates[] =
		{
			"Logo/MU-logo.OZT",
			"Object95/logo.OZJ",
		};
		load_texture_candidates(title_candidates, 2, &state->interface_title_texture, "title_logo", false, true);

		const char* title_glow_candidates[] =
		{
			"Logo/MU-logo_g.OZJ",
			"Object95/logo2_R.OZJ",
			"Object95/logo2.OZJ",
		};
		load_texture_candidates(title_glow_candidates, 3, &state->interface_title_glow_texture, "title_glow", false, true);

		const char* legacy_wave_candidates[] =
		{
			"Object95/wave.ozj",
			"Object95/wave_R.ozj",
			"Logo/wave.OZJ",
		};
		load_texture_candidates(legacy_wave_candidates, 3, &state->interface_scene_wave_texture, "legacy_wave", false, true);

		const char* legacy_sky_candidates[] =
		{
			"Object95/sky0.OZJ",
			"Logo/sky0.OZJ",
			"Logo/sky5.OZJ",
			"Logo/cloud.OZJ",
		};
		load_texture_candidates(legacy_sky_candidates, 4, &state->interface_scene_cloud_texture, "legacy_sky", false, true);

		const char* legacy_horizon_candidates[] =
		{
			"Object95/sky5.OZJ",
			"Logo/sky5.OZJ",
			"Logo/Login_Back02.OZJ",
		};
		load_texture_candidates(legacy_horizon_candidates, 3, &state->interface_scene_horizon_texture, "legacy_horizon", false, true);

		const char* legacy_cloud_band_candidates[] =
		{
			"Object95/so_cloud03.OZJ",
			"Logo/cloud.OZJ",
		};
		load_texture_candidates(legacy_cloud_band_candidates, 2, &state->interface_scene_cloud_band_texture, "legacy_cloud_band", false, true);

		const char* legacy_water_candidates[] =
		{
			"Object95/so_see01.OZT",
			"Object95/wave.ozj",
		};
		load_texture_candidates(legacy_water_candidates, 2, &state->interface_scene_water_texture, "legacy_water", false, true);

		const char* legacy_water_detail_candidates[] =
		{
			"Object95/sosee02_R.OZJ",
			"Object95/sowater01_R.OZJ",
			"Object95/wave_R.ozj",
		};
		load_texture_candidates(legacy_water_detail_candidates, 3, &state->interface_scene_water_detail_texture, "legacy_water_detail", false, true);

		const char* legacy_land_candidates[] =
		{
			"Object95/so_land01.OZT",
		};
		load_texture_candidates(legacy_land_candidates, 1, &state->interface_scene_land_texture, "legacy_land", false, true);

		const std::string leaf_alpha_path = platform::ResolveGameAssetPath("World78/leaf01.OZT");
		const std::string leaf_color_path = platform::ResolveGameAssetPath("World78/leaf01.OZJ");
		if (platform::LoadBootstrapTextureFromColorAndAlphaFiles(leaf_color_path.c_str(), leaf_alpha_path.c_str(), &state->interface_scene_leaf_texture))
		{
			loaded_count += 1;
			__android_log_print(
				ANDROID_LOG_INFO,
				kLogTag,
				"InterfacePreviewTexture = %s %dx%d path=%s",
				"leaf01",
				state->interface_scene_leaf_texture.width,
				state->interface_scene_leaf_texture.height,
				state->interface_scene_leaf_texture.source_path.c_str());
		}
		else
		{
			__android_log_print(
				ANDROID_LOG_WARN,
				kLogTag,
				"InterfacePreviewTexture failed: %s reason=%s path=%s",
				"leaf01",
				state->interface_scene_leaf_texture.error_message.c_str(),
				leaf_color_path.c_str());
		}

		const std::string leaf_alt_color_path = platform::ResolveGameAssetPath("World78/leaf02.OZJ");
		if (platform::LoadBootstrapTextureFromColorAndAlphaFiles(leaf_alt_color_path.c_str(), leaf_alpha_path.c_str(), &state->interface_scene_leaf_alt_texture))
		{
			loaded_count += 1;
			__android_log_print(
				ANDROID_LOG_INFO,
				kLogTag,
				"InterfacePreviewTexture = %s %dx%d path=%s",
				"leaf02",
				state->interface_scene_leaf_alt_texture.width,
				state->interface_scene_leaf_alt_texture.height,
				state->interface_scene_leaf_alt_texture.source_path.c_str());
		}
		else
		{
			__android_log_print(
				ANDROID_LOG_WARN,
				kLogTag,
				"InterfacePreviewTexture failed: %s reason=%s path=%s",
				"leaf02",
				state->interface_scene_leaf_alt_texture.error_message.c_str(),
				leaf_alt_color_path.c_str());
		}

		state->has_interface_preview = loaded_count > 0;
	}

	void BootstrapCoreGameConfig(BootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		state->has_core_bootstrap = false;

		if (gProtect == NULL)
		{
			gProtect = new CProtect;
		}
		platform::CoreGameBootstrapState core_bootstrap_state;
		if (!platform::InitializeCoreGameProtect(gProtect, &core_bootstrap_state))
		{
			__android_log_print(ANDROID_LOG_WARN, kLogTag, "Core bootstrap failed: %s", core_bootstrap_state.error_message.c_str());
			return;
		}

		state->core_bootstrap_state = core_bootstrap_state;
		state->has_core_bootstrap = true;
		platform::ApplyLegacyClientCoreBootstrap(&state->core_bootstrap_state);
		const std::string connect_server_host =
			!state->runtime_config.connect_server_host.empty() ? state->runtime_config.connect_server_host : core_bootstrap_state.ip_address;
		const unsigned short connect_server_port =
			state->runtime_config.connect_server_port != 0 ? state->runtime_config.connect_server_port : core_bootstrap_state.ip_address_port;
		__android_log_print(ANDROID_LOG_INFO, kLogTag, "WindowName = %s", core_bootstrap_state.window_name.c_str());
		__android_log_print(ANDROID_LOG_INFO, kLogTag, "ConnectServer = %s:%u",
			connect_server_host.c_str(),
			static_cast<unsigned int>(connect_server_port));
		if (!state->runtime_config.connect_server_host.empty() || state->runtime_config.connect_server_port != 0)
		{
			__android_log_print(
				ANDROID_LOG_INFO,
				kLogTag,
				"ConnectServerOverride = host=%s port=%u base=%s:%u",
				state->runtime_config.connect_server_host.empty() ? "-" : state->runtime_config.connect_server_host.c_str(),
				static_cast<unsigned int>(state->runtime_config.connect_server_port),
				core_bootstrap_state.ip_address.c_str(),
				static_cast<unsigned int>(core_bootstrap_state.ip_address_port));
		}
		__android_log_print(ANDROID_LOG_INFO, kLogTag, "ClientVersion = %s", core_bootstrap_state.client_version.c_str());
		__android_log_print(ANDROID_LOG_INFO, kLogTag, "MainCRC = %lu", core_bootstrap_state.main_file_crc);
		if (platform::ConfigureConnectServerBootstrap(
				&state->connect_server_bootstrap,
				connect_server_host.c_str(),
				connect_server_port))
		{
			state->preview_connect_logged_status = static_cast<int>(state->connect_server_bootstrap.status);
			state->preview_connect_logged_attempt = state->connect_server_bootstrap.attempt_count;
			__android_log_print(
				ANDROID_LOG_INFO,
				kLogTag,
				"ConnectBootstrap configured=%s target=%s",
				"yes",
				BuildConnectServerSummary(state).c_str());
		}
	}

	void BootstrapPacketCrypto(BootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		state->has_packet_crypto_bootstrap = false;

		platform::PacketCryptoBootstrapState packet_crypto_state;
		if (!platform::InitializePacketCryptoBootstrap(
				platform::ResolveGameAssetPath("Data/Enc1.dat").c_str(),
				platform::ResolveGameAssetPath("Data/Dec2.dat").c_str(),
				&packet_crypto_state))
		{
			__android_log_print(ANDROID_LOG_WARN, kLogTag, "Packet crypto bootstrap failed: %s", packet_crypto_state.error_message.c_str());
			return;
		}

		state->has_packet_crypto_bootstrap = true;
		__android_log_print(ANDROID_LOG_INFO, kLogTag, "PacketEncKey = %s", packet_crypto_state.encryption_key_loaded ? "loaded" : "missing");
		__android_log_print(ANDROID_LOG_INFO, kLogTag, "PacketDecKey = %s", packet_crypto_state.decryption_key_loaded ? "loaded" : "missing");
		__android_log_print(ANDROID_LOG_INFO, kLogTag, "PacketEncryptSmoke = ok (%d -> %d)",
			8,
			packet_crypto_state.encrypted_size);
	}

	int FindPointerIndex(const AInputEvent* event, int pointer_id)
	{
		if (event == NULL)
		{
			return -1;
		}

		const size_t pointer_count = AMotionEvent_getPointerCount(event);
		for (size_t index = 0; index < pointer_count; ++index)
		{
			if (AMotionEvent_getPointerId(event, index) == pointer_id)
			{
				return static_cast<int>(index);
			}
		}

		return -1;
	}

	void DrawTouchOverlay(BootstrapState* state)
	{
		if (state == NULL || !state->touch_active || !state->egl_window.IsReady())
		{
			return;
		}

		const int width = state->egl_window.GetWidth();
		const int height = state->egl_window.GetHeight();
		if (width <= 0 || height <= 0)
		{
			return;
		}

		const float outer_size = state->multi_touch_active ? 28.0f : 22.0f;
		const float inner_size = state->multi_touch_active ? 12.0f : 9.0f;

		platform::QuadVertex2D outer[4] = {
			{ state->touch_x - outer_size, state->touch_y - outer_size, 0.0f, 0.0f, 0.10f, 0.85f, 1.00f, 0.30f },
			{ state->touch_x + outer_size, state->touch_y - outer_size, 0.0f, 0.0f, 0.10f, 0.85f, 1.00f, 0.30f },
			{ state->touch_x + outer_size, state->touch_y + outer_size, 0.0f, 0.0f, 0.10f, 0.85f, 1.00f, 0.30f },
			{ state->touch_x - outer_size, state->touch_y + outer_size, 0.0f, 0.0f, 0.10f, 0.85f, 1.00f, 0.30f },
		};
		platform::QuadVertex2D inner[4] = {
			{ state->touch_x - inner_size, state->touch_y - inner_size, 0.0f, 0.0f, 1.00f, 1.00f, 1.00f, 0.90f },
			{ state->touch_x + inner_size, state->touch_y - inner_size, 0.0f, 0.0f, 1.00f, 1.00f, 1.00f, 0.90f },
			{ state->touch_x + inner_size, state->touch_y + inner_size, 0.0f, 0.0f, 1.00f, 1.00f, 1.00f, 0.90f },
			{ state->touch_x - inner_size, state->touch_y + inner_size, 0.0f, 0.0f, 1.00f, 1.00f, 1.00f, 0.90f },
		};

		platform::RenderBackend& render_backend = platform::GetRenderBackend();
		render_backend.PushOrthoScene(width, height);
		render_backend.SetDepthTestEnabled(false);
		render_backend.SetCullFaceEnabled(false);
		render_backend.SetTextureEnabled(false);
		render_backend.SetAlphaTestEnabled(false);
		render_backend.SetBlendState(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		render_backend.DrawQuad2D(outer, false);
		render_backend.DrawQuad2D(inner, false);
		render_backend.PopScene();
	}

	void DrawTexturedQuad(platform::RenderBackend& render_backend, const platform::BootstrapTexture& texture, float x, float y, float width, float height, float alpha)
	{
		if (!texture.loaded || texture.texture_id == 0 || width <= 0.0f || height <= 0.0f)
		{
			return;
		}

		glBindTexture(GL_TEXTURE_2D, texture.texture_id);

		platform::QuadVertex2D vertices[4] = {
			{ x, y, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, alpha },
			{ x + width, y, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, alpha },
			{ x + width, y + height, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, alpha },
			{ x, y + height, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, alpha },
		};
		render_backend.DrawQuad2D(vertices, true);
	}

	void DrawTexturedQuadUv(
		platform::RenderBackend& render_backend,
		const platform::BootstrapTexture& texture,
		float x,
		float y,
		float width,
		float height,
		float u0,
		float v0,
		float u1,
		float v1,
		float alpha)
	{
		if (!texture.loaded || texture.texture_id == 0 || width <= 0.0f || height <= 0.0f)
		{
			return;
		}

		glBindTexture(GL_TEXTURE_2D, texture.texture_id);

		platform::QuadVertex2D vertices[4] = {
			{ x, y, u0, v0, 1.0f, 1.0f, 1.0f, alpha },
			{ x + width, y, u1, v0, 1.0f, 1.0f, 1.0f, alpha },
			{ x + width, y + height, u1, v1, 1.0f, 1.0f, 1.0f, alpha },
			{ x, y + height, u0, v1, 1.0f, 1.0f, 1.0f, alpha },
		};
		render_backend.DrawQuad2D(vertices, true);
	}

	void DrawTexturedQuadUvTint(
		platform::RenderBackend& render_backend,
		const platform::BootstrapTexture& texture,
		float x,
		float y,
		float width,
		float height,
		float u0,
		float v0,
		float u1,
		float v1,
		float red,
		float green,
		float blue,
		float alpha)
	{
		if (!texture.loaded || texture.texture_id == 0 || width <= 0.0f || height <= 0.0f || alpha <= 0.0f)
		{
			return;
		}

		glBindTexture(GL_TEXTURE_2D, texture.texture_id);

		platform::QuadVertex2D vertices[4] = {
			{ x, y, u0, v0, red, green, blue, alpha },
			{ x + width, y, u1, v0, red, green, blue, alpha },
			{ x + width, y + height, u1, v1, red, green, blue, alpha },
			{ x, y + height, u0, v1, red, green, blue, alpha },
		};
		render_backend.DrawQuad2D(vertices, true);
	}

	void DrawTexturedQuadRotatedUv(
		platform::RenderBackend& render_backend,
		const platform::BootstrapTexture& texture,
		float center_x,
		float center_y,
		float width,
		float height,
		float angle_radians,
		float u0,
		float v0,
		float u1,
		float v1,
		float red,
		float green,
		float blue,
		float alpha)
	{
		if (!texture.loaded || texture.texture_id == 0 || width <= 0.0f || height <= 0.0f || alpha <= 0.0f)
		{
			return;
		}

		glBindTexture(GL_TEXTURE_2D, texture.texture_id);

		const float half_width = width * 0.5f;
		const float half_height = height * 0.5f;
		const float cos_angle = cosf(angle_radians);
		const float sin_angle = sinf(angle_radians);
		const float corner_x[4] = { -half_width, half_width, half_width, -half_width };
		const float corner_y[4] = { -half_height, -half_height, half_height, half_height };
		const float uv_x[4] = { u0, u1, u1, u0 };
		const float uv_y[4] = { v0, v0, v1, v1 };
		platform::QuadVertex2D vertices[4] = {};
		for (int index = 0; index < 4; ++index)
		{
			const float rotated_x = corner_x[index] * cos_angle - corner_y[index] * sin_angle;
			const float rotated_y = corner_x[index] * sin_angle + corner_y[index] * cos_angle;
			vertices[index].x = center_x + rotated_x;
			vertices[index].y = center_y + rotated_y;
			vertices[index].u = uv_x[index];
			vertices[index].v = uv_y[index];
			vertices[index].r = red;
			vertices[index].g = green;
			vertices[index].b = blue;
			vertices[index].a = alpha;
		}
		render_backend.DrawQuad2D(vertices, true);
	}

	void DrawTexturedQuadRow(
		platform::RenderBackend& render_backend,
		const platform::BootstrapTexture& texture,
		float x,
		float y,
		float width,
		float height,
		int row_index,
		int row_count,
		float alpha)
	{
		if (row_count <= 0)
		{
			return;
		}

		const float row_height = 1.0f / static_cast<float>(row_count);
		const float v0 = row_height * static_cast<float>(row_index);
		const float v1 = v0 + row_height;
		DrawTexturedQuadUv(render_backend, texture, x, y, width, height, 0.0f, v0, 1.0f, v1, alpha);
	}

	int ResolveLegacyButtonRow(const platform::LegacyLoginUiButtonState* button_state, int row_count, int fallback_row)
	{
		if (row_count <= 0)
		{
			return 0;
		}

		int resolved_row = fallback_row;
		if (button_state != NULL && button_state->visible)
		{
			resolved_row = button_state->current_frame;
		}

		if (resolved_row < 0)
		{
			return 0;
		}
		if (resolved_row >= row_count)
		{
			return row_count - 1;
		}

		return resolved_row;
	}

	void DrawFilledRect(platform::RenderBackend& render_backend, float x, float y, float width, float height, float red, float green, float blue, float alpha)
	{
		if (width <= 0.0f || height <= 0.0f)
		{
			return;
		}

		platform::QuadVertex2D vertices[4] = {
			{ x, y, 0.0f, 0.0f, red, green, blue, alpha },
			{ x + width, y, 0.0f, 0.0f, red, green, blue, alpha },
			{ x + width, y + height, 0.0f, 0.0f, red, green, blue, alpha },
			{ x, y + height, 0.0f, 0.0f, red, green, blue, alpha },
		};
		render_backend.DrawQuad2D(vertices, false);
	}

	const unsigned char* GetBootstrapGlyphRows(char symbol)
	{
		static const unsigned char kSpace[7] = { 0, 0, 0, 0, 0, 0, 0 };
		static const unsigned char kQuestion[7] = { 0x0e, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04 };

		if (symbol >= 'a' && symbol <= 'z')
		{
			symbol = static_cast<char>(symbol - 'a' + 'A');
		}

		switch (symbol)
		{
		case 'A': { static const unsigned char rows[7] = { 0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 }; return rows; }
		case 'B': { static const unsigned char rows[7] = { 0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e }; return rows; }
		case 'C': { static const unsigned char rows[7] = { 0x0f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0f }; return rows; }
		case 'D': { static const unsigned char rows[7] = { 0x1e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1e }; return rows; }
		case 'E': { static const unsigned char rows[7] = { 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f }; return rows; }
		case 'F': { static const unsigned char rows[7] = { 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10 }; return rows; }
		case 'G': { static const unsigned char rows[7] = { 0x0f, 0x10, 0x10, 0x17, 0x11, 0x11, 0x0f }; return rows; }
		case 'H': { static const unsigned char rows[7] = { 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 }; return rows; }
		case 'I': { static const unsigned char rows[7] = { 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1f }; return rows; }
		case 'J': { static const unsigned char rows[7] = { 0x1f, 0x02, 0x02, 0x02, 0x12, 0x12, 0x0c }; return rows; }
		case 'K': { static const unsigned char rows[7] = { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 }; return rows; }
		case 'L': { static const unsigned char rows[7] = { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f }; return rows; }
		case 'M': { static const unsigned char rows[7] = { 0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11 }; return rows; }
		case 'N': { static const unsigned char rows[7] = { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 }; return rows; }
		case 'O': { static const unsigned char rows[7] = { 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e }; return rows; }
		case 'P': { static const unsigned char rows[7] = { 0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10 }; return rows; }
		case 'Q': { static const unsigned char rows[7] = { 0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d }; return rows; }
		case 'R': { static const unsigned char rows[7] = { 0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11 }; return rows; }
		case 'S': { static const unsigned char rows[7] = { 0x0f, 0x10, 0x10, 0x0e, 0x01, 0x01, 0x1e }; return rows; }
		case 'T': { static const unsigned char rows[7] = { 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 }; return rows; }
		case 'U': { static const unsigned char rows[7] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e }; return rows; }
		case 'V': { static const unsigned char rows[7] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04 }; return rows; }
		case 'W': { static const unsigned char rows[7] = { 0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0a }; return rows; }
		case 'X': { static const unsigned char rows[7] = { 0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11 }; return rows; }
		case 'Y': { static const unsigned char rows[7] = { 0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04 }; return rows; }
		case 'Z': { static const unsigned char rows[7] = { 0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f }; return rows; }
		case '0': { static const unsigned char rows[7] = { 0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e }; return rows; }
		case '1': { static const unsigned char rows[7] = { 0x04, 0x0c, 0x14, 0x04, 0x04, 0x04, 0x1f }; return rows; }
		case '2': { static const unsigned char rows[7] = { 0x0e, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1f }; return rows; }
		case '3': { static const unsigned char rows[7] = { 0x1e, 0x01, 0x01, 0x0e, 0x01, 0x01, 0x1e }; return rows; }
		case '4': { static const unsigned char rows[7] = { 0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02 }; return rows; }
		case '5': { static const unsigned char rows[7] = { 0x1f, 0x10, 0x10, 0x1e, 0x01, 0x01, 0x1e }; return rows; }
		case '6': { static const unsigned char rows[7] = { 0x0e, 0x10, 0x10, 0x1e, 0x11, 0x11, 0x0e }; return rows; }
		case '7': { static const unsigned char rows[7] = { 0x1f, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 }; return rows; }
		case '8': { static const unsigned char rows[7] = { 0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e }; return rows; }
		case '9': { static const unsigned char rows[7] = { 0x0e, 0x11, 0x11, 0x0f, 0x01, 0x01, 0x0e }; return rows; }
		case '.': { static const unsigned char rows[7] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c }; return rows; }
		case ',': { static const unsigned char rows[7] = { 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c, 0x08 }; return rows; }
		case ':': { static const unsigned char rows[7] = { 0x00, 0x0c, 0x0c, 0x00, 0x0c, 0x0c, 0x00 }; return rows; }
		case ';': { static const unsigned char rows[7] = { 0x00, 0x0c, 0x0c, 0x00, 0x0c, 0x0c, 0x08 }; return rows; }
		case '-': { static const unsigned char rows[7] = { 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00 }; return rows; }
		case '_': { static const unsigned char rows[7] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f }; return rows; }
		case '/': { static const unsigned char rows[7] = { 0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10 }; return rows; }
		case '\\': { static const unsigned char rows[7] = { 0x10, 0x10, 0x08, 0x04, 0x02, 0x01, 0x01 }; return rows; }
		case '[': { static const unsigned char rows[7] = { 0x0e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0e }; return rows; }
		case ']': { static const unsigned char rows[7] = { 0x0e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0e }; return rows; }
		case '(': { static const unsigned char rows[7] = { 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02 }; return rows; }
		case ')': { static const unsigned char rows[7] = { 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08 }; return rows; }
		case '+': { static const unsigned char rows[7] = { 0x00, 0x04, 0x04, 0x1f, 0x04, 0x04, 0x00 }; return rows; }
		case '=': { static const unsigned char rows[7] = { 0x00, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x00 }; return rows; }
		case '!': { static const unsigned char rows[7] = { 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04 }; return rows; }
		case '@': { static const unsigned char rows[7] = { 0x0e, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0e }; return rows; }
		case '*': { static const unsigned char rows[7] = { 0x00, 0x11, 0x0a, 0x1f, 0x0a, 0x11, 0x00 }; return rows; }
		case '|': { static const unsigned char rows[7] = { 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 }; return rows; }
		case ' ':
			return kSpace;
		default:
			return kQuestion;
		}
	}

	float MeasureBootstrapTextWidth(const std::string& text, float pixel_size)
	{
		if (text.empty() || pixel_size <= 0.0f)
		{
			return 0.0f;
		}

		return static_cast<float>(text.size() * 6u - 1u) * pixel_size;
	}

	float ComputeBootstrapPixelSizeForWidth(const std::string& text, float preferred_pixel_size, float max_width, float minimum_pixel_size)
	{
		if (text.empty() || preferred_pixel_size <= 0.0f || max_width <= 0.0f)
		{
			return preferred_pixel_size;
		}

		const float preferred_width = MeasureBootstrapTextWidth(text, preferred_pixel_size);
		if (preferred_width <= max_width)
		{
			return preferred_pixel_size;
		}

		const float fitted = max_width / static_cast<float>(text.size() * 6u - 1u);
		return fitted > minimum_pixel_size ? fitted : minimum_pixel_size;
	}

	void DrawBootstrapText(
		platform::RenderBackend& render_backend,
		const std::string& text,
		float x,
		float y,
		float pixel_size,
		float red,
		float green,
		float blue,
		float alpha)
	{
		if (text.empty() || pixel_size <= 0.0f || alpha <= 0.0f)
		{
			return;
		}

		float cursor_x = x;
		for (size_t index = 0; index < text.size(); ++index)
		{
			const unsigned char* rows = GetBootstrapGlyphRows(text[index]);
			for (int row = 0; row < 7; ++row)
			{
				const unsigned char bits = rows[row];
				for (int column = 0; column < 5; ++column)
				{
					if ((bits & (1u << (4 - column))) == 0)
					{
						continue;
					}

					DrawFilledRect(
						render_backend,
						cursor_x + static_cast<float>(column) * pixel_size,
						y + static_cast<float>(6 - row) * pixel_size,
						pixel_size,
						pixel_size,
						red,
						green,
						blue,
						alpha);
				}
			}

			cursor_x += 6.0f * pixel_size;
		}
	}

	void DrawBootstrapTextShadowed(
		platform::RenderBackend& render_backend,
		const std::string& text,
		float x,
		float y,
		float pixel_size,
		float red,
		float green,
		float blue,
		float alpha)
	{
		DrawBootstrapText(render_backend, text, x + pixel_size, y - pixel_size, pixel_size, 0.0f, 0.0f, 0.0f, alpha * 0.65f);
		DrawBootstrapText(render_backend, text, x, y, pixel_size, red, green, blue, alpha);
	}

	void DrawTexturedQuadCover(platform::RenderBackend& render_backend, const platform::BootstrapTexture& texture, float container_width, float container_height, float alpha)
	{
		if (!texture.loaded || texture.width <= 0 || texture.height <= 0 || container_width <= 0.0f || container_height <= 0.0f)
		{
			return;
		}

		const float texture_aspect = static_cast<float>(texture.width) / static_cast<float>(texture.height);
		const float container_aspect = container_width / container_height;
		float quad_width = container_width;
		float quad_height = container_height;
		if (texture_aspect > container_aspect)
		{
			quad_height = container_height;
			quad_width = quad_height * texture_aspect;
		}
		else
		{
			quad_width = container_width;
			quad_height = quad_width / texture_aspect;
		}

		DrawTexturedQuad(
			render_backend,
			texture,
			(container_width - quad_width) * 0.5f,
			(container_height - quad_height) * 0.5f,
			quad_width,
			quad_height,
			alpha);
	}

	void DrawTexturedQuadCoverRect(
		platform::RenderBackend& render_backend,
		const platform::BootstrapTexture& texture,
		const PreviewRect& rect,
		float alpha)
	{
		if (!texture.loaded || texture.width <= 0 || texture.height <= 0 || rect.width <= 0.0f || rect.height <= 0.0f)
		{
			return;
		}

		const float texture_aspect = static_cast<float>(texture.width) / static_cast<float>(texture.height);
		const float container_aspect = rect.width / rect.height;
		float quad_width = rect.width;
		float quad_height = rect.height;
		if (texture_aspect > container_aspect)
		{
			quad_height = rect.height;
			quad_width = quad_height * texture_aspect;
		}
		else
		{
			quad_width = rect.width;
			quad_height = quad_width / texture_aspect;
		}

		DrawTexturedQuad(
			render_backend,
			texture,
			rect.x + (rect.width - quad_width) * 0.5f,
			rect.y + (rect.height - quad_height) * 0.5f,
			quad_width,
			quad_height,
			alpha);
	}

	void DrawCombinedBackground(
		platform::RenderBackend& render_backend,
		const platform::BootstrapTexture& left_texture,
		const platform::BootstrapTexture& right_texture,
		float container_width,
		float container_height,
		float alpha)
	{
		if (!left_texture.loaded || !right_texture.loaded ||
			left_texture.width <= 0 || left_texture.height <= 0 ||
			right_texture.width <= 0 || right_texture.height <= 0 ||
			container_width <= 0.0f || container_height <= 0.0f)
		{
			return;
		}

		const float source_width = static_cast<float>(left_texture.width + right_texture.width);
		const float source_height = static_cast<float>(left_texture.height > right_texture.height ? left_texture.height : right_texture.height);
		if (source_width <= 0.0f || source_height <= 0.0f)
		{
			return;
		}

		const float scale_x = container_width / source_width;
		const float scale_y = container_height / source_height;
		const float scale = scale_x > scale_y ? scale_x : scale_y;
		const float combined_width = source_width * scale;
		const float combined_height = source_height * scale;
		const float start_x = (container_width - combined_width) * 0.5f;
		const float start_y = (container_height - combined_height) * 0.5f;
		const float left_width = static_cast<float>(left_texture.width) * scale;
		const float right_width = static_cast<float>(right_texture.width) * scale;

		DrawTexturedQuad(
			render_backend,
			left_texture,
			start_x,
			start_y,
			left_width,
			combined_height,
			alpha);
		DrawTexturedQuad(
			render_backend,
			right_texture,
			start_x + left_width,
			start_y,
			right_width,
			combined_height,
			alpha);
	}

	void DrawTexturedQuadPixels(
		platform::RenderBackend& render_backend,
		const platform::BootstrapTexture& texture,
		float x,
		float y,
		float width,
		float height,
		float source_x,
		float source_y,
		float source_width,
		float source_height,
		float alpha)
	{
		if (!texture.loaded || texture.width <= 0 || texture.height <= 0 || source_width <= 0.0f || source_height <= 0.0f)
		{
			return;
		}

		const float u0 = source_x / static_cast<float>(texture.width);
		const float v0 = source_y / static_cast<float>(texture.height);
		const float u1 = (source_x + source_width) / static_cast<float>(texture.width);
		const float v1 = (source_y + source_height) / static_cast<float>(texture.height);
		DrawTexturedQuadUv(render_backend, texture, x, y, width, height, u0, v0, u1, v1, alpha);
	}

	float GetLegacyPreviewVirtualWidth(int width, int height)
	{
		if (width <= 0 || height <= 0)
		{
			return kLegacyPreviewWidth;
		}

		return (static_cast<float>(width) / static_cast<float>(height)) * kLegacyPreviewHeight;
	}

	float GetLegacyPreviewRectVirtualWidth(const PreviewRect& scene_rect)
	{
		return scene_rect.virtual_width > 0.0f ? scene_rect.virtual_width : kLegacyPreviewWidth;
	}

	float GetLegacyPreviewRectVirtualHeight(const PreviewRect& scene_rect)
	{
		return scene_rect.virtual_height > 0.0f ? scene_rect.virtual_height : kLegacyPreviewHeight;
	}

	float GetLegacyPreviewCenteredBaseX(const PreviewRect& scene_rect, float x_on_640_base)
	{
		return ((GetLegacyPreviewRectVirtualWidth(scene_rect) - kLegacyPreviewWidth) * 0.5f) + x_on_640_base;
	}

	PreviewRect GetLegacyPreviewSceneRect(int width, int height)
	{
		PreviewRect rect = {};
		if (width <= 0 || height <= 0)
		{
			return rect;
		}

		rect.width = static_cast<float>(width);
		rect.height = static_cast<float>(height);
		rect.x = 0.0f;
		rect.y = 0.0f;
		rect.virtual_width = GetLegacyPreviewVirtualWidth(width, height);
		rect.virtual_height = kLegacyPreviewHeight;
		return rect;
	}

	float MapLegacyPreviewX(const PreviewRect& scene_rect, float virtual_x)
	{
		return scene_rect.x + (virtual_x / GetLegacyPreviewRectVirtualWidth(scene_rect)) * scene_rect.width;
	}

	float MapLegacyPreviewWidth(const PreviewRect& scene_rect, float virtual_width)
	{
		return (virtual_width / GetLegacyPreviewRectVirtualWidth(scene_rect)) * scene_rect.width;
	}

	float MapLegacyPreviewY(const PreviewRect& scene_rect, float virtual_top, float virtual_height)
	{
		return scene_rect.y + scene_rect.height - ((virtual_top + virtual_height) / GetLegacyPreviewRectVirtualHeight(scene_rect)) * scene_rect.height;
	}

	float MapLegacyPreviewHeight(const PreviewRect& scene_rect, float virtual_height)
	{
		return (virtual_height / GetLegacyPreviewRectVirtualHeight(scene_rect)) * scene_rect.height;
	}

	template <typename LegacyRectType>
	PreviewRect MapLegacyPreviewRect(const PreviewRect& scene_rect, const LegacyRectType& rect_state)
	{
		PreviewRect mapped = {};
		if (rect_state.width <= 0 || rect_state.height <= 0)
		{
			return mapped;
		}

		mapped.x = MapLegacyPreviewX(scene_rect, static_cast<float>(rect_state.x));
		mapped.y = MapLegacyPreviewY(scene_rect, static_cast<float>(rect_state.y), static_cast<float>(rect_state.height));
		mapped.width = MapLegacyPreviewWidth(scene_rect, static_cast<float>(rect_state.width));
		mapped.height = MapLegacyPreviewHeight(scene_rect, static_cast<float>(rect_state.height));
		return mapped;
	}

	void DrawCombinedBackgroundRect(
		platform::RenderBackend& render_backend,
		const platform::BootstrapTexture& left_texture,
		const platform::BootstrapTexture& right_texture,
		const PreviewRect& scene_rect,
		float alpha)
	{
		if (!left_texture.loaded || !right_texture.loaded ||
			scene_rect.width <= 0.0f || scene_rect.height <= 0.0f)
		{
			return;
		}

		const float source_width = static_cast<float>(left_texture.width + right_texture.width);
		const float source_height = static_cast<float>(left_texture.height > right_texture.height ? left_texture.height : right_texture.height);
		if (source_width <= 0.0f || source_height <= 0.0f)
		{
			return;
		}

		const float scale = std::max(scene_rect.width / source_width, scene_rect.height / source_height);
		const float combined_width = source_width * scale;
		const float combined_height = source_height * scale;
		const float start_x = scene_rect.x + (scene_rect.width - combined_width) * 0.5f;
		const float start_y = scene_rect.y + (scene_rect.height - combined_height) * 0.5f;
		const float left_width = static_cast<float>(left_texture.width) * scale;
		const float right_width = static_cast<float>(right_texture.width) * scale;

		DrawTexturedQuad(render_backend, left_texture, start_x, start_y, left_width, combined_height, alpha);
		DrawTexturedQuad(render_backend, right_texture, start_x + left_width, start_y, right_width, combined_height, alpha);
	}

	void DrawBootstrapSceneBars(platform::RenderBackend& render_backend, const PreviewRect& scene_rect, float screen_width, float screen_height)
	{
		DrawFilledRect(render_backend, 0.0f, 0.0f, screen_width, scene_rect.y, 0.0f, 0.0f, 0.0f, 1.0f);
		DrawFilledRect(render_backend, 0.0f, scene_rect.y + scene_rect.height, screen_width, screen_height - scene_rect.y - scene_rect.height, 0.0f, 0.0f, 0.0f, 1.0f);
		DrawFilledRect(render_backend, 0.0f, scene_rect.y, scene_rect.x, scene_rect.height, 0.0f, 0.0f, 0.0f, 1.0f);
		DrawFilledRect(render_backend, scene_rect.x + scene_rect.width, scene_rect.y, screen_width - scene_rect.x - scene_rect.width, scene_rect.height, 0.0f, 0.0f, 0.0f, 1.0f);
	}

	void DrawLegacySceneAtmosphere(
		platform::RenderBackend& render_backend,
		const BootstrapState* state,
		const PreviewRect& scene_rect)
	{
		if (state == NULL || scene_rect.width <= 0.0f || scene_rect.height <= 0.0f)
		{
			return;
		}

		if (state->interface_background_uses_legacy_login)
		{
			if (state->interface_uses_object95_scene)
			{
				const float horizon_scroll = fmodf(state->phase * 0.0065f, 1.0f);
				const float water_scroll = fmodf(state->phase * 0.0240f, 1.0f);
				const float shimmer_scroll = fmodf(state->phase * 0.0380f, 1.0f);

				if (state->interface_scene_horizon_texture.loaded)
				{
					DrawTexturedQuadUvTint(
						render_backend,
						state->interface_scene_horizon_texture,
						scene_rect.x,
						MapLegacyPreviewY(scene_rect, 108.0f, 196.0f),
						scene_rect.width,
						MapLegacyPreviewHeight(scene_rect, 196.0f),
						horizon_scroll * 0.08f,
						0.06f,
						1.0f + horizon_scroll * 0.08f,
						0.96f,
						1.0f,
						0.98f,
						0.94f,
						0.82f);
				}

				if (state->interface_scene_cloud_band_texture.loaded)
				{
					DrawTexturedQuadUvTint(
						render_backend,
						state->interface_scene_cloud_band_texture,
						scene_rect.x,
						MapLegacyPreviewY(scene_rect, 112.0f, 120.0f),
						scene_rect.width,
						MapLegacyPreviewHeight(scene_rect, 120.0f),
						horizon_scroll * 0.03f,
						0.08f,
						1.0f + horizon_scroll * 0.03f,
						0.92f,
						0.94f,
						0.92f,
						1.0f,
						0.28f);
				}

				if (state->interface_scene_land_texture.loaded)
				{
					const float land_width_virtual = 328.0f;
					const float land_height_virtual =
						land_width_virtual * (static_cast<float>(state->interface_scene_land_texture.height) / static_cast<float>(state->interface_scene_land_texture.width));
					DrawTexturedQuadUvTint(
						render_backend,
						state->interface_scene_land_texture,
						MapLegacyPreviewX(scene_rect, 320.0f - land_width_virtual * 0.5f),
						MapLegacyPreviewY(scene_rect, 202.0f, land_height_virtual),
						MapLegacyPreviewWidth(scene_rect, land_width_virtual),
						MapLegacyPreviewHeight(scene_rect, land_height_virtual),
						0.0f,
						0.0f,
						1.0f,
						1.0f,
						0.86f,
						0.88f,
						0.90f,
						0.58f);
				}

				DrawFilledRect(
					render_backend,
					scene_rect.x,
					MapLegacyPreviewY(scene_rect, 236.0f, 170.0f),
					scene_rect.width,
					MapLegacyPreviewHeight(scene_rect, 170.0f),
					0.02f,
					0.06f,
					0.10f,
					0.26f);

				DrawFilledRect(
					render_backend,
					scene_rect.x,
					MapLegacyPreviewY(scene_rect, 286.0f, 120.0f),
					scene_rect.width,
					MapLegacyPreviewHeight(scene_rect, 120.0f),
					0.01f,
					0.03f,
					0.05f,
					0.24f);

				if (state->interface_scene_water_texture.loaded)
				{
					DrawTexturedQuadUvTint(
						render_backend,
						state->interface_scene_water_texture,
						scene_rect.x,
						MapLegacyPreviewY(scene_rect, 236.0f, 64.0f),
						scene_rect.width,
						MapLegacyPreviewHeight(scene_rect, 64.0f),
						water_scroll,
						0.18f,
						water_scroll + 3.2f,
						0.96f,
						0.82f,
						0.90f,
						0.98f,
						0.16f);
				}

				if (state->interface_scene_water_detail_texture.loaded)
				{
					DrawTexturedQuadUvTint(
						render_backend,
						state->interface_scene_water_detail_texture,
						scene_rect.x,
						MapLegacyPreviewY(scene_rect, 246.0f, 124.0f),
						scene_rect.width,
						MapLegacyPreviewHeight(scene_rect, 124.0f),
						shimmer_scroll,
						0.0f,
						shimmer_scroll + 2.8f,
						1.0f,
						0.66f,
						0.78f,
						0.90f,
						0.14f);
				}

				if (state->interface_scene_wave_texture.loaded)
				{
					const float wave_alpha = 0.22f + 0.05f * sinf(state->phase * 0.65f);
					const float lower_wave_alpha = 0.18f + 0.04f * cosf(state->phase * 0.48f);
					DrawTexturedQuadUvTint(
						render_backend,
						state->interface_scene_wave_texture,
						scene_rect.x,
						MapLegacyPreviewY(scene_rect, 244.0f, 66.0f),
						scene_rect.width,
						MapLegacyPreviewHeight(scene_rect, 66.0f),
						water_scroll * 0.6f,
						0.05f,
						water_scroll * 0.6f + 4.0f,
						0.90f,
						0.78f,
						0.88f,
						1.0f,
						wave_alpha);
					DrawTexturedQuadUvTint(
						render_backend,
						state->interface_scene_wave_texture,
						scene_rect.x,
						MapLegacyPreviewY(scene_rect, 302.0f, 54.0f),
						scene_rect.width,
						MapLegacyPreviewHeight(scene_rect, 54.0f),
						0.12f + water_scroll * 0.5f,
						0.14f,
						3.6f + water_scroll * 0.5f,
						0.96f,
						0.72f,
						0.82f,
						0.96f,
						lower_wave_alpha);
				}

				DrawFilledRect(
					render_backend,
					scene_rect.x,
					MapLegacyPreviewY(scene_rect, 214.0f, 44.0f),
					scene_rect.width,
					MapLegacyPreviewHeight(scene_rect, 44.0f),
					0.08f,
					0.05f,
					0.04f,
					0.12f);
				return;
			}

			if (state->interface_scene_wave_texture.loaded)
			{
				const float wave_alpha = 0.38f + 0.08f * sinf(state->phase * 0.55f);
				const float lower_wave_alpha = 0.30f + 0.06f * cosf(state->phase * 0.41f);
				const float upper_wave_y = scene_rect.y + scene_rect.height - MapLegacyPreviewHeight(scene_rect, 146.0f);
				const float lower_wave_y = scene_rect.y + scene_rect.height - MapLegacyPreviewHeight(scene_rect, 104.0f);
				DrawTexturedQuadUv(
					render_backend,
					state->interface_scene_wave_texture,
					scene_rect.x,
					upper_wave_y,
					scene_rect.width,
					MapLegacyPreviewHeight(scene_rect, 86.0f),
					0.02f,
					0.02f,
					0.98f,
					0.98f,
					wave_alpha);
				DrawTexturedQuadUv(
					render_backend,
					state->interface_scene_wave_texture,
					scene_rect.x,
					lower_wave_y,
					scene_rect.width,
					MapLegacyPreviewHeight(scene_rect, 62.0f),
					0.08f,
					0.10f,
					0.92f,
					0.98f,
					lower_wave_alpha);
			}

			DrawFilledRect(
				render_backend,
				scene_rect.x,
				scene_rect.y + scene_rect.height - MapLegacyPreviewHeight(scene_rect, 134.0f),
				scene_rect.width,
				MapLegacyPreviewHeight(scene_rect, 134.0f),
				0.02f,
				0.07f,
				0.10f,
				0.26f);
			return;
		}

		if (state->interface_scene_chrome_texture.loaded)
		{
			const float pulse = 0.5f + 0.5f * sinf(state->phase * 0.55f);
			const float uv_margin_x = 0.020f + pulse * 0.030f;
			const float uv_margin_y = 0.014f + pulse * 0.020f;
			const float scroll_u = fmodf(state->phase * 0.018f, 1.0f) * 0.10f;
			const float scroll_v = fmodf(state->phase * 0.011f, 1.0f) * 0.08f;
			const float u0 = scroll_u + uv_margin_x;
			const float v0 = scroll_v + uv_margin_y;
			const float u1 = scroll_u + 1.0f - uv_margin_x;
			const float v1 = scroll_v + 1.0f - uv_margin_y;
			DrawTexturedQuadUv(
				render_backend,
				state->interface_scene_chrome_texture,
				scene_rect.x,
				scene_rect.y,
				scene_rect.width,
				scene_rect.height,
				u0,
				v0,
				u1,
				v1,
				0.26f + pulse * 0.14f);
		}

		if (state->interface_scene_swirl_texture.loaded)
		{
			const float pulse = 0.5f + 0.5f * sinf(state->phase * 1.45f);
			const float crop = 0.11f + pulse * 0.05f;
			const float glow_alpha = 0.10f + pulse * 0.22f;
			DrawTexturedQuadRotatedUv(
				render_backend,
				state->interface_scene_swirl_texture,
				scene_rect.x + scene_rect.width * 0.5f,
				scene_rect.y + scene_rect.height * 0.5f,
				MapLegacyPreviewWidth(scene_rect, 1150.0f),
				MapLegacyPreviewHeight(scene_rect, 1150.0f),
				state->phase * 0.18f,
				crop,
				crop,
				1.0f - crop,
				1.0f - crop,
				0.18f + pulse * 0.10f,
				0.18f + pulse * 0.10f,
				0.40f + pulse * 0.24f,
				glow_alpha);
		}
	}

	void DrawLegacySceneLeaves(
		platform::RenderBackend& render_backend,
		const BootstrapState* state,
		const PreviewRect& scene_rect)
	{
		if (state == NULL || scene_rect.width <= 0.0f || scene_rect.height <= 0.0f)
		{
			return;
		}

		if (state->interface_background_uses_legacy_login || state->interface_uses_object95_scene)
		{
			return;
		}

		const platform::BootstrapTexture* primary_texture = NULL;
		const platform::BootstrapTexture* secondary_texture = NULL;
		if (state->interface_scene_leaf_texture.loaded)
		{
			primary_texture = &state->interface_scene_leaf_texture;
		}
		if (state->interface_scene_leaf_alt_texture.loaded)
		{
			secondary_texture = &state->interface_scene_leaf_alt_texture;
		}
		if (primary_texture == NULL && secondary_texture == NULL)
		{
			return;
		}
		if (primary_texture == NULL)
		{
			primary_texture = secondary_texture;
		}

		for (int index = 0; index < 8; ++index)
		{
			const platform::BootstrapTexture* texture =
				(index % 3 == 0 && secondary_texture != NULL) ? secondary_texture : primary_texture;
			if (texture == NULL || !texture->loaded || texture->width <= 0 || texture->height <= 0)
			{
				continue;
			}

			const float phase_offset = state->phase * (0.10f + 0.018f * static_cast<float>(index));
			const float travel = fmodf(phase_offset + 0.087f * static_cast<float>(index), 1.0f);
			const float sway = sinf(state->phase * (0.55f + 0.03f * static_cast<float>(index)) + static_cast<float>(index) * 0.9f);
			const float depth = 0.55f + 0.45f * sinf(static_cast<float>(index) * 1.37f);
			const float virtual_x = -12.0f + travel * 664.0f + sway * (16.0f + depth * 9.0f);
			const float virtual_top = 76.0f + fmodf(static_cast<float>(index) * 36.0f, 220.0f) + sinf(state->phase * 0.42f + static_cast<float>(index) * 0.7f) * 11.0f;
			const float leaf_width_virtual = 8.0f + depth * 8.0f;
			const float leaf_height_virtual =
				leaf_width_virtual * (static_cast<float>(texture->height) / static_cast<float>(texture->width));
			const float mapped_x = MapLegacyPreviewX(scene_rect, virtual_x);
			const float mapped_y = MapLegacyPreviewY(scene_rect, virtual_top, leaf_height_virtual);
			const float mapped_width = MapLegacyPreviewWidth(scene_rect, leaf_width_virtual);
			const float mapped_height = MapLegacyPreviewHeight(scene_rect, leaf_height_virtual);
			const float angle = state->phase * (0.7f + 0.06f * static_cast<float>(index)) + static_cast<float>(index) * 0.45f;
			const float alpha = 0.025f + depth * 0.045f;
			const float tint = 0.66f + depth * 0.12f;
			DrawTexturedQuadRotatedUv(
				render_backend,
				*texture,
				mapped_x + mapped_width * 0.5f,
				mapped_y + mapped_height * 0.5f,
				mapped_width,
				mapped_height,
				angle,
				0.0f,
				0.0f,
				1.0f,
				1.0f,
				tint,
				0.74f + depth * 0.08f,
				0.66f + depth * 0.08f,
				alpha);
		}
	}

	void DrawLegacyDescriptionPanel(
		platform::RenderBackend& render_backend,
		const BootstrapState* state,
		const PreviewRect& scene_rect,
		float x,
		float top,
		float width,
		float height)
	{
		const float mapped_x = MapLegacyPreviewX(scene_rect, x);
		const float mapped_width = MapLegacyPreviewWidth(scene_rect, width);
		const float mapped_height = MapLegacyPreviewHeight(scene_rect, height);
		const float mapped_y = MapLegacyPreviewY(scene_rect, top, height);
		DrawFilledRect(render_backend, mapped_x, mapped_y, mapped_width, mapped_height, 0.05f, 0.07f, 0.11f, 0.62f);

		if (state != NULL && state->interface_server_desc_fill_texture.loaded)
		{
			const float strip_height = MapLegacyPreviewHeight(scene_rect, 6.0f);
			DrawTexturedQuad(render_backend, state->interface_server_desc_fill_texture, mapped_x, mapped_y + mapped_height - strip_height, mapped_width, strip_height, 0.95f);
			DrawTexturedQuadUv(render_backend, state->interface_server_desc_fill_texture, mapped_x, mapped_y, mapped_width, strip_height, 0.0f, 0.5f, 1.0f, 1.0f, 0.95f);
		}
		if (state != NULL && state->interface_server_desc_cap_texture.loaded)
		{
			const float cap_width = MapLegacyPreviewWidth(scene_rect, 6.0f);
			DrawTexturedQuadUv(render_backend, state->interface_server_desc_cap_texture, mapped_x, mapped_y, cap_width, mapped_height, 0.0f, 0.0f, 0.5f, 1.0f, 0.92f);
			DrawTexturedQuadUv(render_backend, state->interface_server_desc_cap_texture, mapped_x + mapped_width - cap_width, mapped_y, cap_width, mapped_height, 0.5f, 0.0f, 1.0f, 1.0f, 0.92f);
		}
		if (state != NULL && state->interface_server_desc_corner_texture.loaded)
		{
			const float corner_size = MapLegacyPreviewWidth(scene_rect, 4.0f);
			DrawTexturedQuad(render_backend, state->interface_server_desc_corner_texture, mapped_x, mapped_y + mapped_height - corner_size, corner_size, corner_size, 0.92f);
			DrawTexturedQuad(render_backend, state->interface_server_desc_corner_texture, mapped_x + mapped_width - corner_size, mapped_y + mapped_height - corner_size, corner_size, corner_size, 0.92f);
			DrawTexturedQuad(render_backend, state->interface_server_desc_corner_texture, mapped_x, mapped_y, corner_size, corner_size, 0.92f);
			DrawTexturedQuad(render_backend, state->interface_server_desc_corner_texture, mapped_x + mapped_width - corner_size, mapped_y, corner_size, corner_size, 0.92f);
		}
	}

	void DrawLegacyDescriptionPanelPixels(
		platform::RenderBackend& render_backend,
		const BootstrapState* state,
		float x,
		float y,
		float width,
		float height)
	{
		DrawFilledRect(render_backend, x, y, width, height, 0.05f, 0.07f, 0.11f, 0.62f);

		if (state != NULL && state->interface_server_desc_fill_texture.loaded)
		{
			const float strip_height = 6.0f;
			DrawTexturedQuad(render_backend, state->interface_server_desc_fill_texture, x, y + height - strip_height, width, strip_height, 0.95f);
			DrawTexturedQuadUv(render_backend, state->interface_server_desc_fill_texture, x, y, width, strip_height, 0.0f, 0.5f, 1.0f, 1.0f, 0.95f);
		}
		if (state != NULL && state->interface_server_desc_cap_texture.loaded)
		{
			const float cap_width = 6.0f;
			DrawTexturedQuadUv(render_backend, state->interface_server_desc_cap_texture, x, y, cap_width, height, 0.0f, 0.0f, 0.5f, 1.0f, 0.92f);
			DrawTexturedQuadUv(render_backend, state->interface_server_desc_cap_texture, x + width - cap_width, y, cap_width, height, 0.5f, 0.0f, 1.0f, 1.0f, 0.92f);
		}
		if (state != NULL && state->interface_server_desc_corner_texture.loaded)
		{
			const float corner_size = 4.0f;
			DrawTexturedQuad(render_backend, state->interface_server_desc_corner_texture, x, y + height - corner_size, corner_size, corner_size, 0.92f);
			DrawTexturedQuad(render_backend, state->interface_server_desc_corner_texture, x + width - corner_size, y + height - corner_size, corner_size, corner_size, 0.92f);
			DrawTexturedQuad(render_backend, state->interface_server_desc_corner_texture, x, y, corner_size, corner_size, 0.92f);
			DrawTexturedQuad(render_backend, state->interface_server_desc_corner_texture, x + width - corner_size, y, corner_size, corner_size, 0.92f);
		}
	}

	void DrawLegacyInterfaceEdge(
		platform::RenderBackend& render_backend,
		const BootstrapState* state,
		const PreviewRect& scene_rect)
	{
		if (state == NULL)
		{
			return;
		}

		const float virtual_width = GetLegacyPreviewRectVirtualWidth(scene_rect);
		const float top_center_x = (virtual_width - 256.0f) * 0.5f;
		const float top_right_x = virtual_width - 192.0f;
		const float side_right_x = virtual_width - 106.0f;

		if (state->interface_server_group_button_texture.loaded)
		{
			DrawTexturedQuadPixels(
				render_backend,
				state->interface_server_group_button_texture,
				MapLegacyPreviewX(scene_rect, top_right_x),
				MapLegacyPreviewY(scene_rect, 0.0f, 37.0f),
				MapLegacyPreviewWidth(scene_rect, 192.0f),
				MapLegacyPreviewHeight(scene_rect, 37.0f),
				0.0f,
				0.0f,
				192.0f,
				37.0f,
				0.96f);
			DrawTexturedQuadUv(
				render_backend,
				state->interface_server_group_button_texture,
				MapLegacyPreviewX(scene_rect, 0.0f),
				MapLegacyPreviewY(scene_rect, 0.0f, 37.0f),
				MapLegacyPreviewWidth(scene_rect, 192.0f),
				MapLegacyPreviewHeight(scene_rect, 37.0f),
				192.0f / static_cast<float>(state->interface_server_group_button_texture.width),
				0.0f,
				0.0f,
				37.0f / static_cast<float>(state->interface_server_group_button_texture.height),
				0.96f);
		}

		if (state->interface_server_button_texture.loaded)
		{
			DrawTexturedQuadPixels(
				render_backend,
				state->interface_server_button_texture,
				MapLegacyPreviewX(scene_rect, side_right_x),
				MapLegacyPreviewY(scene_rect, 3.0f, 256.0f),
				MapLegacyPreviewWidth(scene_rect, 106.0f),
				MapLegacyPreviewHeight(scene_rect, 256.0f),
				0.0f,
				0.0f,
				106.0f,
				256.0f,
				0.84f);
			DrawTexturedQuadUv(
				render_backend,
				state->interface_server_button_texture,
				MapLegacyPreviewX(scene_rect, 0.0f),
				MapLegacyPreviewY(scene_rect, 3.0f, 256.0f),
				MapLegacyPreviewWidth(scene_rect, 106.0f),
				MapLegacyPreviewHeight(scene_rect, 256.0f),
				106.0f / static_cast<float>(state->interface_server_button_texture.width),
				0.0f,
				0.0f,
				256.0f / static_cast<float>(state->interface_server_button_texture.height),
				0.84f);
		}

		if (state->interface_server_gauge_texture.loaded)
		{
			DrawTexturedQuadPixels(
				render_backend,
				state->interface_server_gauge_texture,
				MapLegacyPreviewX(scene_rect, side_right_x),
				MapLegacyPreviewY(scene_rect, 259.0f, 222.0f),
				MapLegacyPreviewWidth(scene_rect, 106.0f),
				MapLegacyPreviewHeight(scene_rect, 222.0f),
				0.0f,
				0.0f,
				106.0f,
				222.0f,
				0.82f);
			DrawTexturedQuadUv(
				render_backend,
				state->interface_server_gauge_texture,
				MapLegacyPreviewX(scene_rect, 0.0f),
				MapLegacyPreviewY(scene_rect, 259.0f, 222.0f),
				MapLegacyPreviewWidth(scene_rect, 106.0f),
				MapLegacyPreviewHeight(scene_rect, 222.0f),
				106.0f / static_cast<float>(state->interface_server_gauge_texture.width),
				0.0f,
				0.0f,
				222.0f / static_cast<float>(state->interface_server_gauge_texture.height),
				0.82f);
		}

		if (state->interface_server_deco_texture.loaded)
		{
			DrawTexturedQuadPixels(
				render_backend,
				state->interface_server_deco_texture,
				MapLegacyPreviewX(scene_rect, top_center_x),
				MapLegacyPreviewY(scene_rect, 0.0f, 70.0f),
				MapLegacyPreviewWidth(scene_rect, 256.0f),
				MapLegacyPreviewHeight(scene_rect, 70.0f),
				0.0f,
				0.0f,
				256.0f,
				70.0f,
				0.90f);
		}
	}

	void DrawInterfacePreview(BootstrapState* state)
	{
		if (state == NULL || !state->has_interface_preview || !state->egl_window.IsReady())
		{
			return;
		}

		const int width = state->egl_window.GetWidth();
		const int height = state->egl_window.GetHeight();
		if (width <= 0 || height <= 0)
		{
			return;
		}

		platform::RenderBackend& render_backend = platform::GetRenderBackend();
		render_backend.PushOrthoScene(width, height);
		render_backend.SetDepthTestEnabled(false);
		render_backend.SetCullFaceEnabled(false);
		render_backend.SetTextureEnabled(true);
		render_backend.SetAlphaTestEnabled(false);
		render_backend.SetBlendState(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		render_backend.SetCurrentColor(1.0f, 1.0f, 1.0f, 1.0f);

		const PreviewRect scene_rect = GetLegacyPreviewSceneRect(width, height);
		const float scene_scale = scene_rect.height / GetLegacyPreviewRectVirtualHeight(scene_rect);
		const bool show_character_stage = IsPreviewCharacterStage(state);
		const bool show_login_fields = !show_character_stage && IsPreviewLoginStage(state);
		const bool show_server_browser = !show_character_stage && IsPreviewServerBrowserStage(state);
		const bool show_select_button = !show_login_fields && !show_server_browser && !show_character_stage;
		DrawFilledRect(render_backend, 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 0.0f, 0.0f, 1.0f);
		if (state->interface_background_uses_legacy_login || state->interface_uses_object95_scene)
		{
			if (state->interface_scene_cloud_texture.loaded)
			{
				DrawTexturedQuadCoverRect(
					render_backend,
					state->interface_scene_cloud_texture,
					scene_rect,
					1.0f);
			}
		}
		else
		{
			DrawCombinedBackgroundRect(
				render_backend,
				state->interface_background_left_texture,
				state->interface_background_right_texture,
				scene_rect,
				0.95f);
		}
		DrawLegacySceneAtmosphere(render_backend, state, scene_rect);
		DrawLegacySceneLeaves(render_backend, state, scene_rect);
		DrawBootstrapSceneBars(render_backend, scene_rect, static_cast<float>(width), static_cast<float>(height));
		DrawFilledRect(
			render_backend,
			scene_rect.x,
			scene_rect.y + scene_rect.height - MapLegacyPreviewHeight(scene_rect, 25.0f),
			scene_rect.width,
			MapLegacyPreviewHeight(scene_rect, 25.0f),
			0.0f,
			0.0f,
			0.0f,
			1.0f);
		DrawFilledRect(
			render_backend,
			scene_rect.x,
			scene_rect.y,
			scene_rect.width,
			MapLegacyPreviewHeight(scene_rect, 25.0f),
			0.0f,
			0.0f,
			0.0f,
			1.0f);
		DrawFilledRect(
			render_backend,
			scene_rect.x,
			scene_rect.y + MapLegacyPreviewHeight(scene_rect, 25.0f),
			scene_rect.width,
			scene_rect.height - MapLegacyPreviewHeight(scene_rect, 50.0f),
			0.0f,
			0.0f,
			0.0f,
			state->interface_uses_object95_scene ? 0.04f : ((state->interface_background_uses_legacy_login) ? 0.08f : 0.18f));
		if (!show_character_stage)
		{
			DrawLegacyInterfaceEdge(render_backend, state, scene_rect);
		}

		if (state->legacy_login_ui_ready)
		{
			platform::SyncLegacyLoginUiVisibility(show_server_browser, show_login_fields);
		}
		const PreviewHitTarget current_touch_target = state->touch_active ? HitTestPreviewTarget(state, state->touch_x, state->touch_y) : PreviewHitTarget_None;
		platform::LegacyLoginUiBottomState legacy_bottom_state = {};
		const bool has_legacy_bottom_state = platform::CollectLegacyLoginUiBottomState(&legacy_bottom_state);
		platform::LegacyLoginUiServerBrowserState legacy_server_browser = {};
		const bool has_legacy_server_browser =
			state->legacy_login_ui_ready &&
			platform::CollectLegacyLoginUiServerBrowserState(state->preview_selected_server_group, &legacy_server_browser);
		platform::LegacyCharacterUiState legacy_character_state = {};
		const bool has_legacy_character_state =
			state->legacy_character_ui_ready &&
			show_character_stage &&
			platform::CollectLegacyCharacterUiState(&legacy_character_state);
		const bool legacy_character_create_visible =
			has_legacy_character_state &&
			legacy_character_state.create_window_visible;

		if (!show_character_stage && state->interface_title_glow_texture.loaded)
		{
			const float glow_width_virtual = 204.8f;
			const float glow_height_virtual =
				glow_width_virtual * (static_cast<float>(state->interface_title_glow_texture.height) / static_cast<float>(state->interface_title_glow_texture.width));
			const float logo_alpha = std::min(1.0f, state->phase * 0.35f);
			const float glow_alpha =
				(0.72f + 0.12f * sinf(state->phase * 1.5f)) * logo_alpha;
			DrawTexturedQuad(
				render_backend,
				state->interface_title_glow_texture,
				MapLegacyPreviewX(scene_rect, GetLegacyPreviewRectVirtualWidth(scene_rect) * 0.5f - glow_width_virtual * 0.5f),
				MapLegacyPreviewY(scene_rect, 25.0f, glow_height_virtual),
				MapLegacyPreviewWidth(scene_rect, glow_width_virtual),
				MapLegacyPreviewHeight(scene_rect, glow_height_virtual),
				glow_alpha);
		}

		if (!show_character_stage && state->interface_title_texture.loaded)
		{
			const float title_width_virtual = 204.8f;
			const float title_height_virtual =
				title_width_virtual * (static_cast<float>(state->interface_title_texture.height) / static_cast<float>(state->interface_title_texture.width));
			const float title_alpha = std::min(1.0f, state->phase * 0.45f);
			DrawTexturedQuad(
				render_backend,
				state->interface_title_texture,
				MapLegacyPreviewX(scene_rect, GetLegacyPreviewRectVirtualWidth(scene_rect) * 0.5f - title_width_virtual * 0.5f),
				MapLegacyPreviewY(scene_rect, 25.0f, title_height_virtual),
				MapLegacyPreviewWidth(scene_rect, title_width_virtual),
				MapLegacyPreviewHeight(scene_rect, title_height_virtual),
				title_alpha);
		}

		if (show_select_button)
		{
			static bool logged_select_button_rect = false;
			const platform::LegacyLoginUiButtonState* legacy_select_button =
				(has_legacy_server_browser && !legacy_server_browser.group_buttons.empty())
					? &legacy_server_browser.group_buttons[0]
					: NULL;
			const float button_x_virtual = 266.0f;
			const float button_top_virtual = 190.0f;
			const float button_width_virtual = 108.0f;
			const float button_height_virtual = 26.0f;
			const bool button_hot =
				state->touch_active &&
				state->preview_pressed_target == PreviewHitTarget_ButtonLogin &&
				current_touch_target == PreviewHitTarget_ButtonLogin;
			const bool has_legacy_select_rect =
				legacy_select_button != NULL &&
				legacy_select_button->width > 0 &&
				legacy_select_button->height > 0;
			const PreviewRect mapped_legacy_select_rect =
				has_legacy_select_rect ? MapLegacyPreviewRect(scene_rect, *legacy_select_button) : PreviewRect();
			const float button_x =
				has_legacy_select_rect ? mapped_legacy_select_rect.x : MapLegacyPreviewX(scene_rect, GetLegacyPreviewCenteredBaseX(scene_rect, button_x_virtual));
			const float button_y =
				has_legacy_select_rect ? mapped_legacy_select_rect.y : MapLegacyPreviewY(scene_rect, button_top_virtual, button_height_virtual);
			const float button_width =
				has_legacy_select_rect ? mapped_legacy_select_rect.width : MapLegacyPreviewWidth(scene_rect, button_width_virtual);
			const float button_height =
				has_legacy_select_rect ? mapped_legacy_select_rect.height : MapLegacyPreviewHeight(scene_rect, button_height_virtual);
			state->preview_login_button_rect = { button_x, button_y, button_width, button_height };
			if (!logged_select_button_rect && width > 32 && height > 32)
			{
				logged_select_button_rect = true;
				__android_log_print(
					ANDROID_LOG_INFO,
					kLogTag,
					"PreviewSelectButtonRect x=%.1f y=%.1f w=%.1f h=%.1f screen=%dx%d",
					button_x,
					button_y,
					button_width,
					button_height,
					width,
					height);
			}
			if (state->interface_server_group_button_texture.loaded)
			{
				int button_row = button_hot ? 2 : 0;
				if (legacy_select_button != NULL)
				{
					button_row = ResolveLegacyButtonRow(legacy_select_button, 4, button_row);
				}
				DrawTexturedQuadPixels(
					render_backend,
					state->interface_server_group_button_texture,
					button_x,
					button_y,
					button_width,
					button_height,
					0.0f,
					26.0f * static_cast<float>(button_row),
					108.0f,
					26.0f,
					0.98f);
			}
			else if (state->interface_medium_button_texture.loaded)
			{
				DrawTexturedQuadRow(
					render_backend,
					state->interface_medium_button_texture,
					button_x,
					button_y,
					button_width,
					button_height,
					button_hot ? 2 : 1,
					3,
					0.98f);
			}
			const std::string button_label =
				(legacy_select_button != NULL && !legacy_select_button->label.empty())
					? legacy_select_button->label
					: GetPreviewPrimaryButtonLabel(state);
			const float button_pixel = std::max(1.0f, 2.0f * scene_scale);
			const float button_text_width = MeasureBootstrapTextWidth(button_label, button_pixel);
			DrawBootstrapTextShadowed(
				render_backend,
				button_label,
				button_x + (button_width - button_text_width) * 0.5f,
				button_y + (button_height - button_pixel * 7.0f) * 0.5f,
				button_pixel,
				0.98f,
				0.98f,
				0.98f,
				0.96f);

			const bool show_connect_status =
				state->connect_server_bootstrap.status != platform::ConnectServerBootstrapStatus_Idle &&
				!state->preview_status_message.empty();
			if (show_connect_status)
			{
				const std::string status_text = FitPreviewText(state->preview_status_message, 44u, false);
				const float status_pixel = std::max(1.0f, 1.8f * scene_scale);
					DrawBootstrapTextShadowed(
						render_backend,
						status_text,
						scene_rect.x + (scene_rect.width - MeasureBootstrapTextWidth(status_text, status_pixel)) * 0.5f,
						MapLegacyPreviewY(scene_rect, 178.0f, 0.0f),
						status_pixel,
					0.92f,
					0.94f,
					0.98f,
					0.88f);
			}
		}
		else if (show_server_browser)
		{
			NormalizePreviewServerSelection(state);
			std::vector<PreviewVisibleServerGroup> visible_groups;
			CollectPreviewVisibleServerGroups(state, &visible_groups);
			std::vector<PreviewVisibleServerRoom> visible_rooms;
			CollectPreviewVisibleServerRooms(state, state->preview_selected_server_group, &visible_rooms);

			const float group_button_width_virtual = 108.0f;
			const float group_button_height_virtual = 26.0f;
			const float room_button_width_virtual = 193.0f;
			const float room_button_height_virtual = 26.0f;
			const float center_group_x_virtual = 266.0f;
			const float right_group_x_virtual = 370.0f;
			const float room_x_virtual = 402.0f;
			const float group_column_top_virtual = 181.0f;
			const float room_column_top_virtual = 181.0f;
			const float description_x_virtual = 63.0f;
			const float description_top_virtual = 452.0f;
			const float description_width_virtual = 514.0f;
			const float description_height_virtual = 28.0f;

			for (size_t index = 0; index < visible_groups.size() && index < static_cast<size_t>(kPreviewServerGroupSlotCount); ++index)
			{
				const PreviewVisibleServerGroup& group = visible_groups[index];
				const platform::LegacyLoginUiButtonState* legacy_group_button = NULL;
				if (has_legacy_server_browser &&
					group.button_position >= 0 &&
					static_cast<size_t>(group.button_position) < legacy_server_browser.group_buttons.size())
				{
					legacy_group_button = &legacy_server_browser.group_buttons[group.button_position];
				}

				float group_x_virtual = center_group_x_virtual;
				float group_top_virtual = group_column_top_virtual;
				const int draw_slot = GetPreviewServerGroupDrawSlot(group);
				if (draw_slot <= 0)
				{
					group_x_virtual = center_group_x_virtual;
					group_top_virtual = 421.0f;
				}
				else if (draw_slot <= 10)
				{
					group_x_virtual = center_group_x_virtual;
					group_top_virtual = group_column_top_virtual + static_cast<float>(draw_slot - 1) * group_button_height_virtual;
				}
				else
				{
					group_x_virtual = right_group_x_virtual;
					group_top_virtual = group_column_top_virtual + static_cast<float>(draw_slot - 11) * group_button_height_virtual;
				}

				const int target = PreviewHitTarget_ServerGroup0 + static_cast<int>(index);
				const bool group_selected =
					(legacy_group_button != NULL && legacy_group_button->checked) ||
					(legacy_group_button == NULL && static_cast<int>(group.group_index) == state->preview_selected_server_group);
				const bool group_hot =
					state->touch_active &&
					state->preview_pressed_target == target &&
					current_touch_target == target;
				const PreviewRect mapped_legacy_group_rect =
					(legacy_group_button != NULL && legacy_group_button->visible)
						? MapLegacyPreviewRect(scene_rect, *legacy_group_button)
						: PreviewRect();
				const float group_x =
					(legacy_group_button != NULL && legacy_group_button->visible)
						? mapped_legacy_group_rect.x
						: MapLegacyPreviewX(scene_rect, group_x_virtual);
				const float group_y =
					(legacy_group_button != NULL && legacy_group_button->visible)
						? mapped_legacy_group_rect.y
						: MapLegacyPreviewY(scene_rect, group_top_virtual, group_button_height_virtual);
				const float group_width =
					(legacy_group_button != NULL && legacy_group_button->visible)
						? mapped_legacy_group_rect.width
						: MapLegacyPreviewWidth(scene_rect, group_button_width_virtual);
				const float group_height =
					(legacy_group_button != NULL && legacy_group_button->visible)
						? mapped_legacy_group_rect.height
						: MapLegacyPreviewHeight(scene_rect, group_button_height_virtual);
				state->preview_server_group_rects[index] = { group_x, group_y, group_width, group_height };
				if (state->interface_server_group_button_texture.loaded)
				{
					const int group_row = ResolveLegacyButtonRow(
						legacy_group_button,
						4,
						group_selected ? 3 : (group_hot ? 2 : 0));
					DrawTexturedQuadRow(
						render_backend,
						state->interface_server_group_button_texture,
						group_x,
						group_y,
						group_width,
						group_height,
						group_row,
						4,
						group_selected ? 1.0f : 0.95f);
				}
				else
				{
					DrawFilledRect(
						render_backend,
						group_x,
						group_y,
						group_width,
						group_height,
						group_selected ? 0.28f : 0.12f,
						group_selected ? 0.30f : 0.12f,
						group_selected ? 0.34f : 0.12f,
						0.86f);
				}

				const std::string group_label = FitPreviewText(
					(legacy_group_button != NULL && !legacy_group_button->label.empty()) ? legacy_group_button->label : group.name,
					14u,
					false);
				const float group_pixel = std::max(1.0f, 1.7f * scene_scale);
				const float group_text_width = MeasureBootstrapTextWidth(group_label, group_pixel);
				DrawBootstrapTextShadowed(
					render_backend,
					group_label,
					group_x + (group_width - group_text_width) * 0.5f,
					group_y + (group_height - group_pixel * 7.0f) * 0.5f,
					group_pixel,
					group_selected ? 0.98f : 0.90f,
					group_selected ? 0.94f : 0.90f,
					group_selected ? 0.82f : 0.90f,
					0.96f);
			}

			const size_t visible_room_count = std::min<size_t>(visible_rooms.size(), static_cast<size_t>(kPreviewServerSlotCount));
			for (size_t slot = 0; slot < visible_room_count; ++slot)
			{
				static bool logged_first_room_rect = false;
				const PreviewVisibleServerRoom& room = visible_rooms[slot];
				const int legacy_room_slot = room.legacy_room_slot >= 0 ? room.legacy_room_slot : static_cast<int>(slot);
				const platform::LegacyLoginUiButtonState* legacy_room_button =
					(has_legacy_server_browser &&
					 legacy_room_slot >= 0 &&
					 static_cast<size_t>(legacy_room_slot) < legacy_server_browser.room_buttons.size())
						? &legacy_server_browser.room_buttons[legacy_room_slot]
						: NULL;
				const PreviewRect mapped_legacy_room_rect =
					(legacy_room_button != NULL && legacy_room_button->visible)
						? MapLegacyPreviewRect(scene_rect, *legacy_room_button)
						: PreviewRect();
				const float room_x =
					(legacy_room_button != NULL && legacy_room_button->visible)
						? mapped_legacy_room_rect.x
						: MapLegacyPreviewX(scene_rect, room_x_virtual);
				const float room_y =
					(legacy_room_button != NULL && legacy_room_button->visible)
						? mapped_legacy_room_rect.y
						: MapLegacyPreviewY(scene_rect, room_column_top_virtual + static_cast<float>(slot) * room_button_height_virtual, room_button_height_virtual);
				const float room_width =
					(legacy_room_button != NULL && legacy_room_button->visible)
						? mapped_legacy_room_rect.width
						: MapLegacyPreviewWidth(scene_rect, room_button_width_virtual);
				const float room_height =
					(legacy_room_button != NULL && legacy_room_button->visible)
						? mapped_legacy_room_rect.height
						: MapLegacyPreviewHeight(scene_rect, room_button_height_virtual);
				const bool room_selected =
					(legacy_room_button != NULL && legacy_room_button->checked) ||
					(legacy_room_button == NULL && legacy_room_slot == state->preview_selected_server_slot);
				const int target = PreviewHitTarget_ServerEntry0 + static_cast<int>(slot);
				const bool room_hot =
					state->touch_active &&
					state->preview_pressed_target == target &&
					current_touch_target == target;
				state->preview_server_entry_rects[slot] = { room_x, room_y, room_width, room_height };
				if (!logged_first_room_rect && slot == 0 && width > 32 && height > 32)
				{
					logged_first_room_rect = true;
					__android_log_print(
						ANDROID_LOG_INFO,
						kLogTag,
						"PreviewRoomRect slot=%zu x=%.1f y=%.1f w=%.1f h=%.1f tap_x=%.1f tap_y=%.1f screen=%dx%d label=%s",
						slot,
						room_x,
						room_y,
						room_width,
						room_height,
						room_x + room_width * 0.5f,
						static_cast<float>(height) - (room_y + room_height * 0.5f),
						width,
						height,
						room.label.empty() ? "-" : room.label.c_str());
				}

				if (state->interface_server_button_texture.loaded)
				{
					const int room_row = ResolveLegacyButtonRow(
						legacy_room_button,
						3,
						room_hot ? 2 : (room_selected ? 1 : 0));
					DrawTexturedQuadRow(
						render_backend,
						state->interface_server_button_texture,
						room_x,
						room_y,
						room_width,
						room_height,
						room_row,
						3,
						0.98f);
				}
				else
				{
					DrawFilledRect(
						render_backend,
						room_x,
						room_y,
						room_width,
						room_height,
						room_selected ? 0.26f : 0.10f,
						room_selected ? 0.24f : 0.10f,
						room_selected ? 0.18f : 0.10f,
						0.82f);
				}

				const platform::LegacyLoginUiRectState* legacy_room_gauge =
					(has_legacy_server_browser &&
					 legacy_room_slot >= 0 &&
					 static_cast<size_t>(legacy_room_slot) < legacy_server_browser.room_gauges.size())
						? &legacy_server_browser.room_gauges[legacy_room_slot]
						: NULL;
				const PreviewRect mapped_legacy_room_gauge =
					(legacy_room_gauge != NULL && legacy_room_gauge->visible)
						? MapLegacyPreviewRect(scene_rect, *legacy_room_gauge)
						: PreviewRect();
				const float gauge_y = (legacy_room_gauge != NULL && legacy_room_gauge->visible)
					? mapped_legacy_room_gauge.y
					: room_y + MapLegacyPreviewHeight(scene_rect, 3.0f);
				const float gauge_width = (legacy_room_gauge != NULL && legacy_room_gauge->visible)
					? mapped_legacy_room_gauge.width
					: MapLegacyPreviewWidth(scene_rect, 160.0f);
				const float gauge_height = (legacy_room_gauge != NULL && legacy_room_gauge->visible)
					? mapped_legacy_room_gauge.height
					: MapLegacyPreviewHeight(scene_rect, 4.0f);
				const float gauge_x = (legacy_room_gauge != NULL && legacy_room_gauge->visible)
					? mapped_legacy_room_gauge.x
					: room_x + MapLegacyPreviewWidth(scene_rect, 16.0f);
				DrawFilledRect(render_backend, gauge_x, gauge_y, gauge_width, gauge_height, 0.10f, 0.10f, 0.10f, 0.66f);
				const float gauge_ratio = room.load_percent >= 128 ? 1.0f : std::min(1.0f, static_cast<float>(room.load_percent) / 100.0f);
				if (gauge_ratio > 0.0f)
				{
					if (state->interface_server_gauge_texture.loaded)
					{
						DrawTexturedQuad(render_backend, state->interface_server_gauge_texture, gauge_x, gauge_y, gauge_width * gauge_ratio, gauge_height, 0.96f);
					}
					else
					{
						DrawFilledRect(render_backend, gauge_x, gauge_y, gauge_width * gauge_ratio, gauge_height, 0.60f, 0.72f, 0.25f, 0.94f);
					}
				}

				const std::string room_label = FitPreviewText(
					(legacy_room_button != NULL && !legacy_room_button->label.empty())
						? legacy_room_button->label
						: room.label,
					29u,
					false);
				const float room_pixel = std::max(1.0f, 1.75f * scene_scale);
				DrawBootstrapTextShadowed(
					render_backend,
					room_label,
					room_x + MapLegacyPreviewWidth(scene_rect, 16.0f),
					room_y + MapLegacyPreviewHeight(scene_rect, 9.0f),
					room_pixel,
					0.96f,
					0.95f,
					0.90f,
					0.94f);
			}

			if (state->interface_server_deco_texture.loaded)
			{
				const bool used_legacy_server_deco =
					has_legacy_server_browser &&
					(legacy_server_browser.left_deco_rect.visible ||
					 legacy_server_browser.right_deco_rect.visible ||
					 legacy_server_browser.left_arrow_rect.visible ||
					 legacy_server_browser.right_arrow_rect.visible);
				if (used_legacy_server_deco)
				{
					if (legacy_server_browser.left_deco_rect.visible)
					{
						const PreviewRect left_deco_rect = MapLegacyPreviewRect(scene_rect, legacy_server_browser.left_deco_rect);
						DrawTexturedQuadPixels(
							render_backend,
							state->interface_server_deco_texture,
							left_deco_rect.x,
							left_deco_rect.y,
							left_deco_rect.width,
							left_deco_rect.height,
							0.0f,
							0.0f,
							68.0f,
							95.0f,
							0.92f);
					}
					if (legacy_server_browser.right_deco_rect.visible)
					{
						const PreviewRect right_deco_rect = MapLegacyPreviewRect(scene_rect, legacy_server_browser.right_deco_rect);
						DrawTexturedQuadPixels(
							render_backend,
							state->interface_server_deco_texture,
							right_deco_rect.x,
							right_deco_rect.y,
							right_deco_rect.width,
							right_deco_rect.height,
							68.0f,
							0.0f,
							68.0f,
							95.0f,
							0.92f);
					}
					if (legacy_server_browser.left_arrow_rect.visible)
					{
						const PreviewRect left_arrow_rect = MapLegacyPreviewRect(scene_rect, legacy_server_browser.left_arrow_rect);
						DrawTexturedQuadPixels(
							render_backend,
							state->interface_server_deco_texture,
							left_arrow_rect.x,
							left_arrow_rect.y,
							left_arrow_rect.width,
							left_arrow_rect.height,
							136.0f,
							0.0f,
							23.0f,
							29.0f,
							0.96f);
					}
					if (legacy_server_browser.right_arrow_rect.visible)
					{
						const PreviewRect right_arrow_rect = MapLegacyPreviewRect(scene_rect, legacy_server_browser.right_arrow_rect);
						DrawTexturedQuadPixels(
							render_backend,
							state->interface_server_deco_texture,
							right_arrow_rect.x,
							right_arrow_rect.y,
							right_arrow_rect.width,
							right_arrow_rect.height,
							136.0f,
							30.0f,
							23.0f,
							29.0f,
							0.96f);
					}
				}
				else if (!visible_groups.empty())
				{
					int selected_draw_slot = -1;
					if (has_legacy_server_browser && legacy_server_browser.selected_button_index >= 0)
					{
						selected_draw_slot = legacy_server_browser.selected_button_index;
					}
					else
					{
						const int selected_group_slot = FindPreviewVisibleServerGroupSlot(visible_groups, state->preview_selected_server_group);
						selected_draw_slot =
							(selected_group_slot >= 0 && static_cast<size_t>(selected_group_slot) < visible_groups.size())
								? ((visible_groups[selected_group_slot].button_position >= 0)
										? visible_groups[selected_group_slot].button_position
										: GetPreviewServerGroupDrawSlot(visible_groups[selected_group_slot]))
								: -1;
					}
					if (selected_draw_slot >= 1 && selected_draw_slot <= 10)
					{
						const float deco_x = MapLegacyPreviewX(scene_rect, 258.0f);
						const float deco_y = MapLegacyPreviewY(scene_rect, 162.0f, 95.0f);
						DrawTexturedQuadPixels(render_backend, state->interface_server_deco_texture, deco_x, deco_y, MapLegacyPreviewWidth(scene_rect, 68.0f), MapLegacyPreviewHeight(scene_rect, 95.0f), 0.0f, 0.0f, 68.0f, 95.0f, 0.92f);
						const float arrow_y = MapLegacyPreviewY(
							scene_rect,
							group_column_top_virtual + static_cast<float>(selected_draw_slot - 1) * group_button_height_virtual - 2.0f,
							29.0f);
						DrawTexturedQuadPixels(
							render_backend,
							state->interface_server_deco_texture,
							MapLegacyPreviewX(scene_rect, center_group_x_virtual + 107.0f),
							arrow_y,
							MapLegacyPreviewWidth(scene_rect, 23.0f),
							MapLegacyPreviewHeight(scene_rect, 29.0f),
							136.0f,
							0.0f,
							23.0f,
							29.0f,
							0.96f);
					}
					else if (selected_draw_slot >= 11)
					{
						const float deco_x = MapLegacyPreviewX(scene_rect, 418.0f);
						const float deco_y = MapLegacyPreviewY(scene_rect, 162.0f, 95.0f);
						DrawTexturedQuadPixels(render_backend, state->interface_server_deco_texture, deco_x, deco_y, MapLegacyPreviewWidth(scene_rect, 68.0f), MapLegacyPreviewHeight(scene_rect, 95.0f), 68.0f, 0.0f, 68.0f, 95.0f, 0.92f);
						const float arrow_y = MapLegacyPreviewY(
							scene_rect,
							group_column_top_virtual + static_cast<float>(selected_draw_slot - 11) * group_button_height_virtual - 2.0f,
							29.0f);
						DrawTexturedQuadPixels(
							render_backend,
							state->interface_server_deco_texture,
							MapLegacyPreviewX(scene_rect, right_group_x_virtual - 23.0f),
							arrow_y,
							MapLegacyPreviewWidth(scene_rect, 23.0f),
							MapLegacyPreviewHeight(scene_rect, 29.0f),
							136.0f,
							30.0f,
							23.0f,
							29.0f,
							0.96f);
					}
				}
			}

			if (has_legacy_server_browser && legacy_server_browser.description_panel_rect.visible)
			{
				const PreviewRect description_panel_rect = MapLegacyPreviewRect(scene_rect, legacy_server_browser.description_panel_rect);
				DrawLegacyDescriptionPanelPixels(
					render_backend,
					state,
					description_panel_rect.x,
					description_panel_rect.y,
					description_panel_rect.width,
					description_panel_rect.height);
			}
			else
			{
				DrawLegacyDescriptionPanel(
					render_backend,
					state,
					scene_rect,
					description_x_virtual,
					description_top_virtual,
					description_width_virtual,
					description_height_virtual);
			}

			bool drew_description_lines = false;
			if (has_legacy_server_browser && legacy_server_browser.description_panel_rect.visible)
			{
				const PreviewRect description_panel_rect = MapLegacyPreviewRect(scene_rect, legacy_server_browser.description_panel_rect);
				const float description_pixel = std::max(1.0f, 1.55f * scene_scale);
				float line_y = description_panel_rect.y + MapLegacyPreviewHeight(scene_rect, 6.0f);
				const float line_x = description_panel_rect.x + MapLegacyPreviewWidth(scene_rect, 10.0f);
				for (size_t line_index = 0; line_index < legacy_server_browser.description_lines.size(); ++line_index)
				{
					if (legacy_server_browser.description_lines[line_index].empty())
					{
						continue;
					}

					DrawBootstrapTextShadowed(
						render_backend,
						legacy_server_browser.description_lines[line_index],
						line_x,
						line_y,
						description_pixel,
						0.90f,
						0.95f,
						1.0f,
						0.92f);
					line_y += description_pixel * 8.0f;
					drew_description_lines = true;
				}
			}
			if (!drew_description_lines)
			{
				std::string description_text;
				if (has_legacy_server_browser)
				{
					for (size_t line_index = 0; line_index < legacy_server_browser.description_lines.size(); ++line_index)
					{
						if (legacy_server_browser.description_lines[line_index].empty())
						{
							continue;
						}

						if (!description_text.empty())
						{
							description_text.append(" ");
						}
						description_text.append(legacy_server_browser.description_lines[line_index]);
					}
				}
				if (description_text.empty())
				{
					description_text = state->preview_status_message;
					for (size_t index = 0; index < visible_groups.size(); ++index)
					{
						if (static_cast<int>(visible_groups[index].group_index) == state->preview_selected_server_group &&
							!visible_groups[index].description.empty())
						{
							description_text = visible_groups[index].description;
							break;
						}
					}
				}
				description_text = FitPreviewText(description_text, 76u, false);
				const PreviewRect description_panel_rect =
					(has_legacy_server_browser && legacy_server_browser.description_panel_rect.visible)
						? MapLegacyPreviewRect(scene_rect, legacy_server_browser.description_panel_rect)
						: PreviewRect();
				const float description_x = (has_legacy_server_browser && legacy_server_browser.description_panel_rect.visible)
					? description_panel_rect.x + MapLegacyPreviewWidth(scene_rect, 10.0f)
					: MapLegacyPreviewX(scene_rect, description_x_virtual + 10.0f);
				const float description_y = (has_legacy_server_browser && legacy_server_browser.description_panel_rect.visible)
					? description_panel_rect.y + MapLegacyPreviewHeight(scene_rect, 7.0f)
					: MapLegacyPreviewY(scene_rect, description_top_virtual + 7.0f, 0.0f);
				const float description_pixel = std::max(1.0f, 1.55f * scene_scale);
				DrawBootstrapTextShadowed(
					render_backend,
					description_text,
					description_x,
					description_y,
					description_pixel,
					0.90f,
					0.95f,
					1.0f,
					0.92f);
			}
			if (has_legacy_server_browser && legacy_server_browser.selected_group_is_pvp)
			{
				const float notice_pixel = std::max(1.0f, 1.45f * scene_scale);
				for (size_t line_index = 0; line_index < legacy_server_browser.pvp_notice_lines.size(); ++line_index)
				{
					if (legacy_server_browser.pvp_notice_lines[line_index].empty())
					{
						continue;
					}

					DrawBootstrapTextShadowed(
						render_backend,
						legacy_server_browser.pvp_notice_lines[line_index],
						MapLegacyPreviewX(scene_rect, 90.0f),
						MapLegacyPreviewY(scene_rect, 104.0f + static_cast<float>(line_index) * 15.0f, 0.0f),
						notice_pixel,
						1.0f,
						1.0f,
						1.0f,
						0.96f);
				}
			}
		}
		else if (show_login_fields && state->interface_login_window_texture.loaded)
		{
			platform::LegacyLoginUiLoginState legacy_login_state = {};
			const bool has_legacy_login_state = platform::CollectLegacyLoginUiLoginState(&legacy_login_state);
			const PreviewRect mapped_login_panel_rect =
				(has_legacy_login_state && legacy_login_state.panel_rect.visible)
					? MapLegacyPreviewRect(scene_rect, legacy_login_state.panel_rect)
					: PreviewRect();
			const PreviewRect mapped_account_field_rect =
				(has_legacy_login_state && legacy_login_state.account_field_rect.visible)
					? MapLegacyPreviewRect(scene_rect, legacy_login_state.account_field_rect)
					: PreviewRect();
			const PreviewRect mapped_password_field_rect =
				(has_legacy_login_state && legacy_login_state.password_field_rect.visible)
					? MapLegacyPreviewRect(scene_rect, legacy_login_state.password_field_rect)
					: PreviewRect();
			const PreviewRect mapped_account_input_rect =
				(has_legacy_login_state && legacy_login_state.account_input_rect.visible)
					? MapLegacyPreviewRect(scene_rect, legacy_login_state.account_input_rect)
					: PreviewRect();
			const PreviewRect mapped_password_input_rect =
				(has_legacy_login_state && legacy_login_state.password_input_rect.visible)
					? MapLegacyPreviewRect(scene_rect, legacy_login_state.password_input_rect)
					: PreviewRect();
			const float panel_x = (has_legacy_login_state && legacy_login_state.panel_rect.visible)
				? mapped_login_panel_rect.x
				: MapLegacyPreviewX(scene_rect, GetLegacyPreviewCenteredBaseX(scene_rect, 220.0f));
			const float panel_y = (has_legacy_login_state && legacy_login_state.panel_rect.visible)
				? mapped_login_panel_rect.y
				: MapLegacyPreviewY(scene_rect, 187.0f, 137.0f);
			const float panel_width = (has_legacy_login_state && legacy_login_state.panel_rect.visible)
				? mapped_login_panel_rect.width
				: MapLegacyPreviewWidth(scene_rect, 200.0f);
			const float panel_height = (has_legacy_login_state && legacy_login_state.panel_rect.visible)
				? mapped_login_panel_rect.height
				: MapLegacyPreviewHeight(scene_rect, 137.0f);
			DrawTexturedQuad(render_backend, state->interface_login_window_texture, panel_x, panel_y, panel_width, panel_height, 0.97f);

			const float field_x = (has_legacy_login_state && legacy_login_state.account_field_rect.visible)
				? mapped_account_field_rect.x
				: MapLegacyPreviewX(scene_rect, GetLegacyPreviewCenteredBaseX(scene_rect, 280.0f));
			const float field_width = (has_legacy_login_state && legacy_login_state.account_field_rect.visible)
				? mapped_account_field_rect.width
				: MapLegacyPreviewWidth(scene_rect, 114.0f);
			const float field_height = (has_legacy_login_state && legacy_login_state.account_field_rect.visible)
				? mapped_account_field_rect.height
				: MapLegacyPreviewHeight(scene_rect, 21.0f);
			const float account_y = (has_legacy_login_state && legacy_login_state.account_field_rect.visible)
				? mapped_account_field_rect.y
				: MapLegacyPreviewY(scene_rect, 257.0f, 21.0f);
			const float password_y = (has_legacy_login_state && legacy_login_state.password_field_rect.visible)
				? mapped_password_field_rect.y
				: MapLegacyPreviewY(scene_rect, 298.0f, 21.0f);
			float account_target_x = field_x;
			float account_target_y = account_y;
			float account_target_right = field_x + field_width;
			float account_target_bottom = account_y + field_height;
			float password_target_x = field_x;
			float password_target_y = password_y;
			float password_target_right = field_x + field_width;
			float password_target_bottom = password_y + field_height;
			if (has_legacy_login_state && legacy_login_state.account_input_rect.visible)
			{
				const float input_x = mapped_account_input_rect.x;
				const float input_y = mapped_account_input_rect.y;
				const float input_right = input_x + mapped_account_input_rect.width;
				const float input_bottom = input_y + mapped_account_input_rect.height;
				account_target_x = std::min(account_target_x, input_x);
				account_target_y = std::min(account_target_y, input_y);
				account_target_right = std::max(account_target_right, input_right);
				account_target_bottom = std::max(account_target_bottom, input_bottom);
			}
			if (has_legacy_login_state && legacy_login_state.password_input_rect.visible)
			{
				const float input_x = mapped_password_input_rect.x;
				const float input_y = mapped_password_input_rect.y;
				const float input_right = input_x + mapped_password_input_rect.width;
				const float input_bottom = input_y + mapped_password_input_rect.height;
				password_target_x = std::min(password_target_x, input_x);
				password_target_y = std::min(password_target_y, input_y);
				password_target_right = std::max(password_target_right, input_right);
				password_target_bottom = std::max(password_target_bottom, input_bottom);
			}
			state->preview_account_rect = {
				account_target_x,
				account_target_y,
				account_target_right - account_target_x,
				account_target_bottom - account_target_y
			};
			state->preview_password_rect = {
				password_target_x,
				password_target_y,
				password_target_right - password_target_x,
				password_target_bottom - password_target_y
			};
			{
				static int field_rect_log = 0;
				if (field_rect_log < 3) {
					++field_rect_log;
					__android_log_print(ANDROID_LOG_INFO, kLogTag,
						"AccountRect x=%.1f y=%.1f w=%.1f h=%.1f PasswordRect x=%.1f y=%.1f w=%.1f h=%.1f",
						account_target_x, account_target_y,
						account_target_right - account_target_x, account_target_bottom - account_target_y,
						password_target_x, password_target_y,
						password_target_right - password_target_x, password_target_bottom - password_target_y);
				}
			}

			const bool account_focused =
				has_legacy_login_state ? legacy_login_state.account_focused : (state->preview_focused_target == PreviewHitTarget_FieldAccount);
			const bool password_focused =
				has_legacy_login_state ? legacy_login_state.password_focused : (state->preview_focused_target == PreviewHitTarget_FieldPassword);
			DrawFilledRect(
				render_backend,
				field_x,
				account_y,
				field_width,
				field_height,
				0.08f,
				0.07f,
				0.06f,
				account_focused ? 0.30f : 0.16f);
			DrawFilledRect(
				render_backend,
				field_x,
				password_y,
				field_width,
				field_height,
				0.08f,
				0.07f,
				0.06f,
				password_focused ? 0.30f : 0.16f);

			const float label_pixel = std::max(1.0f, 1.55f * scene_scale);
			const float field_pixel = std::max(1.0f, 1.78f * scene_scale);
			const float field_padding_x = MapLegacyPreviewWidth(scene_rect, 6.0f);
			const size_t account_visible_count = static_cast<size_t>((field_width - field_padding_x * 2.0f) / (field_pixel * 6.0f));
			const std::string account_label =
				(has_legacy_login_state && !legacy_login_state.account_label.empty())
					? legacy_login_state.account_label
					: GetPreviewTargetLabel(state, PreviewHitTarget_FieldAccount);
			const std::string password_label =
				(has_legacy_login_state && !legacy_login_state.password_label.empty())
					? legacy_login_state.password_label
					: GetPreviewTargetLabel(state, PreviewHitTarget_FieldPassword);
			const std::string account_display = FitPreviewText(state->preview_account_value, account_visible_count > 0 ? account_visible_count : 1u, true);
			const std::string password_display = FitPreviewText(BuildPreviewPasswordMask(state->preview_password_value), account_visible_count > 0 ? account_visible_count : 1u, true);

			DrawBootstrapTextShadowed(
				render_backend,
				account_label,
				(has_legacy_login_state ? MapLegacyPreviewX(scene_rect, static_cast<float>(legacy_login_state.account_label_x)) : MapLegacyPreviewX(scene_rect, GetLegacyPreviewCenteredBaseX(scene_rect, 200.0f))),
				(has_legacy_login_state ? MapLegacyPreviewY(scene_rect, static_cast<float>(legacy_login_state.account_label_y), 0.0f) : MapLegacyPreviewY(scene_rect, 266.0f, 0.0f)),
				label_pixel,
				1.0f,
				1.0f,
				1.0f,
				0.96f);
			DrawBootstrapTextShadowed(
				render_backend,
				password_label,
				(has_legacy_login_state ? MapLegacyPreviewX(scene_rect, static_cast<float>(legacy_login_state.password_label_x)) : MapLegacyPreviewX(scene_rect, GetLegacyPreviewCenteredBaseX(scene_rect, 200.0f))),
				(has_legacy_login_state ? MapLegacyPreviewY(scene_rect, static_cast<float>(legacy_login_state.password_label_y), 0.0f) : MapLegacyPreviewY(scene_rect, 305.0f, 0.0f)),
				label_pixel,
				1.0f,
				1.0f,
				1.0f,
				0.96f);
			DrawBootstrapTextShadowed(
				render_backend,
				account_display,
				(has_legacy_login_state && legacy_login_state.account_input_rect.visible)
					? mapped_account_input_rect.x
					: (field_x + field_padding_x),
				(has_legacy_login_state && legacy_login_state.account_input_rect.visible)
					? mapped_account_input_rect.y
					: (account_y + MapLegacyPreviewHeight(scene_rect, 5.0f)),
				field_pixel,
				1.0f,
				0.90f,
				0.82f,
				0.96f);
			DrawBootstrapTextShadowed(
				render_backend,
				password_display,
				(has_legacy_login_state && legacy_login_state.password_input_rect.visible)
					? mapped_password_input_rect.x
					: (field_x + field_padding_x),
				(has_legacy_login_state && legacy_login_state.password_input_rect.visible)
					? mapped_password_input_rect.y
					: (password_y + MapLegacyPreviewHeight(scene_rect, 5.0f)),
				field_pixel,
				1.0f,
				0.90f,
				0.82f,
				0.96f);

			if (has_legacy_login_state && !legacy_login_state.server_name.empty())
			{
				DrawBootstrapTextShadowed(
					render_backend,
					legacy_login_state.server_name,
					MapLegacyPreviewX(scene_rect, static_cast<float>(legacy_login_state.server_name_x)) +
						(MapLegacyPreviewWidth(scene_rect, static_cast<float>(legacy_login_state.server_name_width)) - MeasureBootstrapTextWidth(legacy_login_state.server_name, field_pixel)) * 0.5f,
					MapLegacyPreviewY(scene_rect, static_cast<float>(legacy_login_state.server_name_y), 0.0f),
					field_pixel,
					1.0f,
					1.0f,
					1.0f,
					0.96f);
			}

			const bool caret_visible = fmodf(state->phase * 10.0f, 2.0f) < 1.0f;
			if (caret_visible && (account_focused || password_focused))
			{
				const bool on_account = account_focused;
				const std::string& caret_text = on_account ? account_display : password_display;
				const float caret_x = field_x + field_padding_x + MeasureBootstrapTextWidth(caret_text, field_pixel) + field_pixel;
				const float caret_y = (on_account ? account_y : password_y) + MapLegacyPreviewHeight(scene_rect, 4.0f);
				const float caret_height = field_pixel * 7.0f;
				DrawFilledRect(render_backend, caret_x, caret_y, field_pixel, caret_height, 0.95f, 0.88f, 0.70f, 0.90f);
			}

			if (state->interface_medium_button_texture.loaded)
			{
				const platform::LegacyLoginUiButtonState* legacy_login_button =
					(has_legacy_login_state && legacy_login_state.login_button.visible)
						? &legacy_login_state.login_button
						: NULL;
				const platform::LegacyLoginUiButtonState* legacy_cancel_button =
					(has_legacy_login_state && legacy_login_state.cancel_button.visible)
						? &legacy_login_state.cancel_button
						: NULL;
				const PreviewRect mapped_login_button_rect =
					(legacy_login_button != NULL) ? MapLegacyPreviewRect(scene_rect, *legacy_login_button) : PreviewRect();
				const PreviewRect mapped_cancel_button_rect =
					(legacy_cancel_button != NULL) ? MapLegacyPreviewRect(scene_rect, *legacy_cancel_button) : PreviewRect();
				const float button_width = legacy_login_button != NULL
					? mapped_login_button_rect.width
					: MapLegacyPreviewWidth(scene_rect, 60.0f);
				const float button_height = legacy_login_button != NULL
					? mapped_login_button_rect.height
					: MapLegacyPreviewHeight(scene_rect, 18.0f);
				const float button_y = (legacy_login_button != NULL)
					? mapped_login_button_rect.y
					: MapLegacyPreviewY(scene_rect, 343.0f, 18.0f);
				const float login_x = (legacy_login_button != NULL)
					? mapped_login_button_rect.x
					: MapLegacyPreviewX(scene_rect, GetLegacyPreviewCenteredBaseX(scene_rect, 250.0f));
				const float cancel_x = (legacy_cancel_button != NULL)
					? mapped_cancel_button_rect.x
					: MapLegacyPreviewX(scene_rect, GetLegacyPreviewCenteredBaseX(scene_rect, 332.0f));
				const float cancel_width = legacy_cancel_button != NULL
					? mapped_cancel_button_rect.width
					: button_width;
				const float cancel_height = legacy_cancel_button != NULL
					? mapped_cancel_button_rect.height
					: button_height;
				const bool login_hot = state->touch_active && state->preview_pressed_target == PreviewHitTarget_ButtonLogin && current_touch_target == PreviewHitTarget_ButtonLogin;
				const bool cancel_hot = state->touch_active && state->preview_pressed_target == PreviewHitTarget_ButtonCancel && current_touch_target == PreviewHitTarget_ButtonCancel;
				const std::string login_label =
					(has_legacy_login_state && !legacy_login_state.login_button.label.empty())
						? legacy_login_state.login_button.label
						: GetPreviewTargetLabel(state, PreviewHitTarget_ButtonLogin);
				const std::string cancel_label =
					(has_legacy_login_state && !legacy_login_state.cancel_button.label.empty())
						? legacy_login_state.cancel_button.label
						: GetPreviewTargetLabel(state, PreviewHitTarget_ButtonCancel);
				const float button_pixel = std::max(1.0f, 1.55f * scene_scale);

				state->preview_login_button_rect = { login_x, button_y, button_width, button_height };
				state->preview_cancel_button_rect = { cancel_x, button_y, cancel_width, cancel_height };
				{
					static int login_rect_log = 0;
					if (login_rect_log < 3) {
						++login_rect_log;
						__android_log_print(ANDROID_LOG_INFO, kLogTag,
							"LoginButtonRect x=%.1f y=%.1f w=%.1f h=%.1f cancel_x=%.1f legacy=%s screen=%dx%d",
							login_x, button_y, button_width, button_height, cancel_x,
							legacy_login_button ? "yes" : "no", width, height);
					}
				}
				const int login_row = ResolveLegacyButtonRow(legacy_login_button, 3, login_hot ? 1 : 0);
				const int cancel_row = ResolveLegacyButtonRow(legacy_cancel_button, 3, cancel_hot ? 1 : 0);
				DrawTexturedQuadUv(
					render_backend,
					state->interface_medium_button_texture,
					login_x,
					button_y,
					button_width,
					button_height,
					0.002f,
					0.002f + 0.211f * static_cast<float>(login_row),
					0.952f,
					0.202f + 0.211f * static_cast<float>(login_row),
					1.0f);
				DrawTexturedQuadUv(
					render_backend,
					state->interface_medium_button_texture,
					cancel_x,
					button_y,
					cancel_width,
					cancel_height,
					0.002f,
					0.002f + 0.211f * static_cast<float>(cancel_row),
					0.952f,
					0.202f + 0.211f * static_cast<float>(cancel_row),
					1.0f);
				DrawBootstrapTextShadowed(render_backend, login_label, login_x + (button_width - MeasureBootstrapTextWidth(login_label, button_pixel)) * 0.5f, button_y + (button_height - button_pixel * 7.0f) * 0.5f, button_pixel, 0.93f, 0.84f, 0.63f, 0.95f);
				DrawBootstrapTextShadowed(render_backend, cancel_label, cancel_x + (cancel_width - MeasureBootstrapTextWidth(cancel_label, button_pixel)) * 0.5f, button_y + (cancel_height - button_pixel * 7.0f) * 0.5f, button_pixel, 0.93f, 0.84f, 0.63f, 0.95f);
			}

			const bool show_login_status_overlay =
				!state->preview_status_message.empty() &&
				(state->game_server_bootstrap.status != platform::GameServerBootstrapStatus_Idle ||
				 state->connect_server_bootstrap.status == platform::ConnectServerBootstrapStatus_Failed);
			if (show_login_status_overlay)
			{
				const std::string status_text = FitPreviewText(state->preview_status_message, 34u, false);
				const float status_pixel = std::max(1.0f, 1.55f * scene_scale);
				DrawBootstrapTextShadowed(render_backend, status_text, scene_rect.x + (scene_rect.width - MeasureBootstrapTextWidth(status_text, status_pixel)) * 0.5f, MapLegacyPreviewY(scene_rect, 176.0f, 0.0f), status_pixel, 0.84f, 0.93f, 1.0f, 0.90f);
			}

			if (state->connect_server_bootstrap.server_address_received &&
				state->game_server_bootstrap.status != platform::GameServerBootstrapStatus_Idle)
			{
				const std::string address_text = FitPreviewText(BuildGameServerSummary(state), 36u, false);
				const float address_pixel = std::max(1.0f, 1.5f * scene_scale);
				DrawBootstrapTextShadowed(render_backend, address_text, scene_rect.x + (scene_rect.width - MeasureBootstrapTextWidth(address_text, address_pixel)) * 0.5f, MapLegacyPreviewY(scene_rect, 248.0f, 0.0f), address_pixel, 0.72f, 0.94f, 0.72f, 0.88f);
			}

		}

		std::vector<platform::LegacyCharacterUiEntryState> preview_character_entries;
		const bool has_preview_character_entries =
			show_character_stage &&
			!legacy_character_create_visible &&
			CollectPreviewCharacterEntries(state, &preview_character_entries);
		if (show_character_stage)
		{
			platform::RenderLegacyCharacterSceneRuntime(
				static_cast<int>(scene_rect.x),
				static_cast<int>(scene_rect.y),
				static_cast<int>(scene_rect.width),
				static_cast<int>(scene_rect.height));
		}
		if (has_preview_character_entries)
		{
			const size_t visible_count = std::min<size_t>(preview_character_entries.size(), 10u);
			for (size_t index = 0; index < visible_count; ++index)
			{
				const platform::LegacyCharacterUiEntryState& entry = preview_character_entries[index];
				const float row_x = MapLegacyPreviewX(scene_rect, 470.0f + (GetLegacyPreviewRectVirtualWidth(scene_rect) - 640.0f));
				const float row_y = MapLegacyPreviewY(scene_rect, 50.0f + 37.0f * static_cast<float>(index), 35.0f);
				const float row_width = MapLegacyPreviewWidth(scene_rect, 150.0f);
				const float row_height = MapLegacyPreviewHeight(scene_rect, 35.0f);
				state->preview_character_entry_rects[index] = { row_x, row_y, row_width, row_height };
				const int target = PreviewHitTarget_CharacterEntry0 + static_cast<int>(index);
				const bool row_hot =
					state->touch_active &&
					state->preview_pressed_target == target &&
					current_touch_target == target;
				const bool row_selected =
					state->game_server_bootstrap.selected_character_slot != 0xFF &&
					state->game_server_bootstrap.selected_character_slot == entry.slot;
				if (row_selected || row_hot)
				{
					DrawFilledRect(render_backend, row_x, row_y, row_width, row_height, 0.18f, 0.16f, 0.12f, row_selected ? 0.34f : 0.20f);
				}
			}
		}

		if (has_legacy_character_state)
		{
			const PreviewRect info_rect = MapLegacyPreviewRect(scene_rect, legacy_character_state.info_rect);
			const PreviewRect create_rect = MapLegacyPreviewRect(scene_rect, legacy_character_state.create_button);
			const PreviewRect menu_rect = MapLegacyPreviewRect(scene_rect, legacy_character_state.menu_button);
			const PreviewRect connect_rect = MapLegacyPreviewRect(scene_rect, legacy_character_state.connect_button);
			const PreviewRect delete_rect = MapLegacyPreviewRect(scene_rect, legacy_character_state.delete_button);

			(void)info_rect;

			if (!legacy_character_create_visible)
			{
				state->preview_character_create_rect = create_rect;
				state->preview_character_menu_rect = menu_rect;
				state->preview_character_connect_rect = connect_rect;
				state->preview_character_delete_rect = delete_rect;
			}
			else
			{
				state->preview_character_create_window_rect = MapLegacyPreviewRect(scene_rect, legacy_character_state.create_window_rect);
				state->preview_character_create_ok_rect = MapLegacyPreviewRect(scene_rect, legacy_character_state.create_ok_button);
				state->preview_character_create_cancel_rect = MapLegacyPreviewRect(scene_rect, legacy_character_state.create_cancel_button);
			}

			render_backend.PushOrthoSceneViewport(
				static_cast<int>(scene_rect.x),
				static_cast<int>(scene_rect.y),
				static_cast<int>(scene_rect.width),
				static_cast<int>(scene_rect.height),
				height,
				GetLegacyPreviewRectVirtualWidth(scene_rect),
				GetLegacyPreviewRectVirtualHeight(scene_rect));
			platform::RenderLegacyCharacterUiRuntime();
			render_backend.PopScene();
		}

		if (!show_character_stage && state->interface_menu_button_texture.loaded && state->interface_credit_button_texture.loaded)
		{
			float menu_x = MapLegacyPreviewX(scene_rect, 0.0f);
			float menu_y = MapLegacyPreviewY(scene_rect, 413.0f, 30.0f);
			float menu_width = MapLegacyPreviewWidth(scene_rect, 54.0f);
			float menu_height = MapLegacyPreviewHeight(scene_rect, 30.0f);
			float credit_x = MapLegacyPreviewX(scene_rect, GetLegacyPreviewRectVirtualWidth(scene_rect) - 54.0f);
			float credit_y = menu_y;
			float credit_width = menu_width;
			float credit_height = menu_height;

			if (has_legacy_bottom_state && legacy_bottom_state.menu_button.visible)
			{
				const PreviewRect menu_rect = MapLegacyPreviewRect(scene_rect, legacy_bottom_state.menu_button);
				menu_x = menu_rect.x;
				menu_y = menu_rect.y;
				menu_width = menu_rect.width;
				menu_height = menu_rect.height;
			}

			if (has_legacy_bottom_state && legacy_bottom_state.credit_button.visible)
			{
				const PreviewRect credit_rect = MapLegacyPreviewRect(scene_rect, legacy_bottom_state.credit_button);
				credit_x = credit_rect.x;
				credit_y = credit_rect.y;
				credit_width = credit_rect.width;
				credit_height = credit_rect.height;
			}

			state->preview_menu_button_rect = { menu_x, menu_y, menu_width, menu_height };
			state->preview_credit_button_rect = { credit_x, credit_y, credit_width, credit_height };

			const bool menu_hot = state->touch_active && state->preview_pressed_target == PreviewHitTarget_ButtonMenu && current_touch_target == PreviewHitTarget_ButtonMenu;
			const bool credit_hot = state->touch_active && state->preview_pressed_target == PreviewHitTarget_ButtonCredit && current_touch_target == PreviewHitTarget_ButtonCredit;
			const int menu_row = ResolveLegacyButtonRow(
				has_legacy_bottom_state ? &legacy_bottom_state.menu_button : NULL,
				3,
				menu_hot ? 2 : 0);
			const int credit_row = ResolveLegacyButtonRow(
				has_legacy_bottom_state ? &legacy_bottom_state.credit_button : NULL,
				3,
				credit_hot ? 2 : 0);

			DrawTexturedQuadRow(render_backend, state->interface_menu_button_texture, menu_x, menu_y, menu_width, menu_height, menu_row, 3, 1.0f);
			DrawTexturedQuadRow(render_backend, state->interface_credit_button_texture, credit_x, credit_y, credit_width, credit_height, credit_row, 3, 1.0f);
		}

		if (!show_character_stage && state->interface_bottom_deco_texture.loaded)
		{
			float deco_x = MapLegacyPreviewX(scene_rect, 481.0f);
			float deco_y = MapLegacyPreviewY(scene_rect, 354.0f, 103.0f);
			float deco_width = MapLegacyPreviewWidth(scene_rect, 189.0f);
			float deco_height = MapLegacyPreviewHeight(scene_rect, 103.0f);
			if (has_legacy_bottom_state && legacy_bottom_state.deco_visible)
			{
				const platform::LegacyLoginUiRectState legacy_deco_rect = {
					legacy_bottom_state.deco_visible,
					legacy_bottom_state.deco_x,
					legacy_bottom_state.deco_y,
					legacy_bottom_state.deco_width,
					legacy_bottom_state.deco_height
				};
				const PreviewRect deco_rect = MapLegacyPreviewRect(scene_rect, legacy_deco_rect);
				deco_x = deco_rect.x;
				deco_y = deco_rect.y;
				deco_width = deco_rect.width;
				deco_height = deco_rect.height;
			}
			DrawTexturedQuad(
				render_backend,
				state->interface_bottom_deco_texture,
				deco_x,
				deco_y,
				deco_width,
				deco_height,
				0.94f);
		}

		if (!show_character_stage && state->interface_credit_logo_texture.loaded)
		{
			const float logo_width_virtual = 164.0f;
			const float logo_height_virtual =
				logo_width_virtual * (static_cast<float>(state->interface_credit_logo_texture.height) / static_cast<float>(state->interface_credit_logo_texture.width));
			DrawTexturedQuad(
				render_backend,
				state->interface_credit_logo_texture,
				MapLegacyPreviewX(scene_rect, GetLegacyPreviewRectVirtualWidth(scene_rect) * 0.5f - logo_width_virtual * 0.5f),
				MapLegacyPreviewY(scene_rect, 456.0f, logo_height_virtual),
				MapLegacyPreviewWidth(scene_rect, logo_width_virtual),
				MapLegacyPreviewHeight(scene_rect, logo_height_virtual),
				0.86f);
		}

		render_backend.PopScene();
	}

	void CreateIfPossible(BootstrapState* state, android_app* app)
	{
		if (state == NULL || app == NULL || app->window == NULL || state->egl_window.IsReady())
		{
			return;
		}

		state->egl_window.AttachWindow(app->window);

		platform::AndroidEglConfig config = {};
		config.red_bits = 8;
		config.green_bits = 8;
		config.blue_bits = 8;
		config.alpha_bits = 8;
		config.depth_bits = 24;
		config.stencil_bits = 8;
		config.swap_interval = 1;

		const platform::AndroidEglResult result = state->egl_window.Create(config);
		if (!result.ok)
		{
			__android_log_print(ANDROID_LOG_ERROR, kLogTag, "Create EGL failed at %s (0x%x)", result.step, result.error_code);
			state->egl_window.Destroy();
			state->egl_window.DetachWindow();
			return;
		}

		ConfigureGameDataRoot(state, app);
		BootstrapClientIniConfig(state, ExtractPackagedClientConfig(app));
			BootstrapClientRuntimeConfig(state, ExtractPackagedRuntimeConfig(app));
			BootstrapLanguageAssets(state);
			BootstrapGlobalText(state);
			BootstrapServerListScript(state);
			BootstrapCoreGameConfig(state);
			BootstrapPacketCrypto(state);

		platform::SetPreferredRenderBackendType(platform::RenderBackendType_OpenGLES2);
		platform::InitializeRenderBackend();
		BootstrapInterfacePreview(state);
		{
			std::string legacy_login_ui_error;
			const bool legacy_login_ui_ready = platform::InitializeLegacyLoginUiRuntime(
				state->egl_window.GetWidth(),
				state->egl_window.GetHeight(),
				&legacy_login_ui_error);
			state->legacy_login_ui_ready = legacy_login_ui_ready;
			__android_log_print(
				legacy_login_ui_ready ? ANDROID_LOG_INFO : ANDROID_LOG_WARN,
				kLogTag,
				"LegacyLoginUi = ready=%s reason=%s",
				legacy_login_ui_ready ? "yes" : "no",
				legacy_login_ui_error.empty() ? "-" : legacy_login_ui_error.c_str());
		}
		{
			std::string legacy_character_ui_error;
			const bool legacy_character_ui_ready = platform::InitializeLegacyCharacterUiRuntime(
				state->egl_window.GetWidth(),
				state->egl_window.GetHeight(),
				&legacy_character_ui_error);
			state->legacy_character_ui_ready = legacy_character_ui_ready;
			__android_log_print(
				legacy_character_ui_ready ? ANDROID_LOG_INFO : ANDROID_LOG_WARN,
				kLogTag,
				"LegacyCharacterUi = ready=%s reason=%s",
				legacy_character_ui_ready ? "yes" : "no",
				legacy_character_ui_error.empty() ? "-" : legacy_character_ui_error.c_str());
		}

		LogInfo("EGL window ready");
	}

	void DestroySurface(BootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		platform::ShutdownGameServerBootstrap(&state->game_server_bootstrap);
		platform::ShutdownConnectServerBootstrap(&state->connect_server_bootstrap);
		platform::ShutdownLegacyCharacterUiRuntime();
		state->legacy_character_ui_ready = false;
		platform::ShutdownLegacyLoginUiRuntime();
		state->legacy_login_ui_ready = false;
		DestroyInterfacePreviewTextures(state);
		platform::ShutdownRenderBackend();
		state->egl_window.Destroy();
		state->egl_window.DetachWindow();
	}

	void TriggerPreviewAction(BootstrapState* state, android_app* app, int target, const char* source)
	{
		if (state == NULL || !IsPreviewButtonTarget(target))
		{
			return;
		}

		const bool is_keyboard_confirm =
			source != NULL &&
			(strcmp(source, "enter") == 0 || strcmp(source, "keyboard") == 0);

		if (state->legacy_login_ui_ready)
		{
			bool handled_by_legacy = false;

			if (target == PreviewHitTarget_ButtonMenu)
			{
				SetPreviewFocus(state, app, PreviewHitTarget_None);
				handled_by_legacy = platform::HandleLegacyLoginMainButtonAction(0);
			}
			else if (target == PreviewHitTarget_ButtonCredit)
			{
				SetPreviewFocus(state, app, PreviewHitTarget_None);
				handled_by_legacy = platform::HandleLegacyLoginMainButtonAction(1);
			}
			else if (IsPreviewServerBrowserStage(state) &&
				target >= PreviewHitTarget_ServerGroup0 &&
				target <= PreviewHitTarget_ServerGroup20)
			{
				SetPreviewFocus(state, app, PreviewHitTarget_None);
				handled_by_legacy = TryHandleLegacyServerGroupTarget(state, target);
			}
			else if (IsPreviewServerBrowserStage(state) &&
				target >= PreviewHitTarget_ServerEntry0 &&
				target <= PreviewHitTarget_ServerEntry15)
			{
				SetPreviewFocus(state, app, PreviewHitTarget_None);
				handled_by_legacy = TryHandleLegacyServerRoomTarget(state, target);
			}
			else if (IsPreviewLoginStage(state) && target == PreviewHitTarget_ButtonLogin)
			{
				SetPreviewFocus(state, app, PreviewHitTarget_None);
				handled_by_legacy = platform::HandleLegacyLoginConfirmAction();
			}
			else if (IsPreviewLoginStage(state) && target == PreviewHitTarget_ButtonCancel)
			{
				SetPreviewFocus(state, app, PreviewHitTarget_None);
				handled_by_legacy = platform::HandleLegacyLoginCancelAction();
			}

			if (handled_by_legacy)
			{
				SyncPreviewSelectionFromLegacyRuntime(state);
				state->preview_action_count += 1;
				goto log_preview_action;
			}
		}

		if (state->legacy_character_ui_ready && IsPreviewCharacterStage(state))
		{
			platform::LegacyCharacterUiAction legacy_character_action = platform::LegacyCharacterUiAction_None;
			switch (target)
			{
			case PreviewHitTarget_CharacterCreate:
				legacy_character_action = platform::LegacyCharacterUiAction_Create;
				break;
			case PreviewHitTarget_CharacterMenu:
				legacy_character_action = platform::LegacyCharacterUiAction_Menu;
				break;
			case PreviewHitTarget_CharacterConnect:
				legacy_character_action = platform::LegacyCharacterUiAction_Connect;
				break;
			case PreviewHitTarget_CharacterDelete:
				legacy_character_action = platform::LegacyCharacterUiAction_Delete;
				break;
			default:
				break;
			}

			if (legacy_character_action != platform::LegacyCharacterUiAction_None)
			{
				SetPreviewFocus(state, app, PreviewHitTarget_None);
				if (platform::HandleLegacyCharacterButtonAction(legacy_character_action))
				{
					SyncSelectedCharacterFromLegacyRuntime(state);
					state->preview_action_count += 1;
					goto log_preview_action;
				}
			}

			if (target == PreviewHitTarget_CharacterCreateConfirm)
			{
				SetPreviewFocus(state, app, PreviewHitTarget_None);
				if (platform::HandleLegacyCharacterCreateConfirmAction())
				{
					state->preview_action_count += 1;
					goto log_preview_action;
				}
			}
			else if (target == PreviewHitTarget_CharacterCreateCancel)
			{
				SetPreviewFocus(state, app, PreviewHitTarget_None);
				if (platform::HandleLegacyCharacterCreateCancelAction())
				{
					state->preview_action_count += 1;
					goto log_preview_action;
				}
			}
		}

		state->preview_action_count += 1;
		switch (target)
		{
			case PreviewHitTarget_ServerGroup0:
			case PreviewHitTarget_ServerGroup1:
			case PreviewHitTarget_ServerGroup2:
			case PreviewHitTarget_ServerGroup3:
			case PreviewHitTarget_ServerGroup4:
			case PreviewHitTarget_ServerGroup5:
			case PreviewHitTarget_ServerGroup6:
			case PreviewHitTarget_ServerGroup7:
			case PreviewHitTarget_ServerGroup8:
			case PreviewHitTarget_ServerGroup9:
			case PreviewHitTarget_ServerGroup10:
			case PreviewHitTarget_ServerGroup11:
			case PreviewHitTarget_ServerGroup12:
			case PreviewHitTarget_ServerGroup13:
			case PreviewHitTarget_ServerGroup14:
			case PreviewHitTarget_ServerGroup15:
			case PreviewHitTarget_ServerGroup16:
			case PreviewHitTarget_ServerGroup17:
			case PreviewHitTarget_ServerGroup18:
			case PreviewHitTarget_ServerGroup19:
			case PreviewHitTarget_ServerGroup20:
			{
				if (TryHandleLegacyServerGroupTarget(state, target))
				{
					break;
				}

				std::vector<PreviewVisibleServerGroup> visible_groups;
				CollectPreviewVisibleServerGroups(state, &visible_groups);
				const int group_slot = GetPreviewServerGroupSlotFromTarget(target);
				if (group_slot >= 0 && static_cast<size_t>(group_slot) < visible_groups.size())
				{
					state->preview_selected_server_group = static_cast<int>(visible_groups[group_slot].group_index);
					state->preview_selected_server_slot = 0;
					ClearPreviewSelectedServerAddress(state);
					platform::DisconnectGameServerBootstrap(&state->game_server_bootstrap, "GameServer reiniciado");
					if (!visible_groups[group_slot].description.empty())
					{
						SetPreviewStatusMessage(state, visible_groups[group_slot].description);
					}
					else
					{
						SetPreviewStatusMessage(state, std::string("Grupo selecionado: ") + visible_groups[group_slot].name);
					}
				}
				break;
			}
			case PreviewHitTarget_ServerEntry0:
			case PreviewHitTarget_ServerEntry1:
			case PreviewHitTarget_ServerEntry2:
			case PreviewHitTarget_ServerEntry3:
			case PreviewHitTarget_ServerEntry4:
			case PreviewHitTarget_ServerEntry5:
			case PreviewHitTarget_ServerEntry6:
			case PreviewHitTarget_ServerEntry7:
			case PreviewHitTarget_ServerEntry8:
			case PreviewHitTarget_ServerEntry9:
			case PreviewHitTarget_ServerEntry10:
			case PreviewHitTarget_ServerEntry11:
			case PreviewHitTarget_ServerEntry12:
			case PreviewHitTarget_ServerEntry13:
			case PreviewHitTarget_ServerEntry14:
			case PreviewHitTarget_ServerEntry15:
			{
				if (TryHandleLegacyServerRoomTarget(state, target))
				{
					break;
				}

				const int slot = GetPreviewServerSlotFromTarget(target);
				std::vector<PreviewVisibleServerRoom> visible_rooms;
				CollectPreviewVisibleServerRooms(state, state->preview_selected_server_group, &visible_rooms);
				if (slot >= 0 &&
					static_cast<size_t>(slot) < visible_rooms.size())
				{
					const PreviewVisibleServerRoom& room = visible_rooms[slot];
					state->preview_selected_server_slot = room.legacy_room_slot >= 0 ? room.legacy_room_slot : slot;
					unsigned short legacy_connect_index = room.connect_index;
					platform::TrySelectLegacyLoginServerEntry(
						state->preview_selected_server_group,
						room.legacy_room_slot >= 0 ? room.legacy_room_slot : slot,
						&legacy_connect_index);
					if (state->connect_server_bootstrap.status == platform::ConnectServerBootstrapStatus_Connected &&
						state->connect_server_bootstrap.server_list_received)
					{
						SetPreviewFocus(state, app, PreviewHitTarget_None);
						platform::DisconnectGameServerBootstrap(&state->game_server_bootstrap, "GameServer reiniciado");
						ClearPreviewSelectedServerAddress(state);
						if (!platform::RequestConnectServerAddressBootstrap(&state->connect_server_bootstrap, legacy_connect_index))
						{
							SetPreviewStatusMessage(state, state->connect_server_bootstrap.status_message);
						}
						else
						{
							RefreshPreviewStatusFromConnectServer(state);
						}
					}
					else
					{
						std::ostringstream stream;
						stream << room.label << " selecionada";
						SetPreviewStatusMessage(state, stream.str());
					}
				}
				break;
			}
			case PreviewHitTarget_CharacterEntry0:
			case PreviewHitTarget_CharacterEntry1:
			case PreviewHitTarget_CharacterEntry2:
			case PreviewHitTarget_CharacterEntry3:
			case PreviewHitTarget_CharacterEntry4:
			case PreviewHitTarget_CharacterEntry5:
			case PreviewHitTarget_CharacterEntry6:
			case PreviewHitTarget_CharacterEntry7:
			case PreviewHitTarget_CharacterEntry8:
			case PreviewHitTarget_CharacterEntry9:
			{
				const int character_slot = GetPreviewCharacterSlotFromTarget(target);
				std::vector<platform::LegacyCharacterUiEntryState> preview_character_entries;
				if (character_slot >= 0 &&
					CollectPreviewCharacterEntries(state, &preview_character_entries) &&
					static_cast<size_t>(character_slot) < preview_character_entries.size())
				{
					const platform::LegacyCharacterUiEntryState& entry = preview_character_entries[character_slot];
					state->game_server_bootstrap.selected_character_slot = entry.slot;
					state->game_server_bootstrap.selected_character_name = entry.name;
					if (state->legacy_character_ui_ready)
					{
						platform::SelectLegacyCharacterUiSlot(entry.slot);
					}

					std::ostringstream stream;
					stream << "Personagem selecionado: " << entry.name;
					SetPreviewStatusMessage(state, stream.str());
				}
				break;
			}
			case PreviewHitTarget_ButtonLogin:
				SetPreviewFocus(state, app, PreviewHitTarget_None);
				if (IsPreviewLoginStage(state))
				{
					const bool handled_by_legacy = state->legacy_login_ui_ready
						? platform::HandleLegacyLoginConfirmAction()
						: false;
					if (handled_by_legacy)
					{
						RefreshPreviewStatusFromGameServer(state);
					}
					else if (!platform::RequestGameServerLoginBootstrap(&state->game_server_bootstrap))
					{
						SetPreviewStatusMessage(state, state->game_server_bootstrap.status_message);
					}
					else
					{
						RefreshPreviewStatusFromGameServer(state);
					}
				}
				else if (state->connect_server_bootstrap.status == platform::ConnectServerBootstrapStatus_Connected)
				{
					if (state->legacy_login_ui_ready)
					{
						platform::SelectLegacyPrimaryServerGroup();
						SyncPreviewSelectionFromLegacyRuntime(state, 0);
					}
					if (!state->connect_server_bootstrap.server_list_received)
					{
						SetPreviewStatusMessage(state, "Buscando salas...");
					}
					else if (state->connect_server_bootstrap.server_entries.empty())
					{
						SetPreviewStatusMessage(state, "Nenhuma sala encontrada");
					}
					else
					{
						SetPreviewStatusMessage(state, "Clique em uma sala");
					}
				}
				else if (!platform::StartConnectServerBootstrap(&state->connect_server_bootstrap))
				{
					SetPreviewStatusMessage(state, state->connect_server_bootstrap.status_message);
				}
				else
				{
					const bool used_legacy_group_selection = state->legacy_login_ui_ready;
					if (state->legacy_login_ui_ready)
					{
						platform::SelectLegacyPrimaryServerGroup();
						SyncPreviewSelectionFromLegacyRuntime(state, 0);
					}
					platform::DisconnectGameServerBootstrap(&state->game_server_bootstrap, "GameServer reiniciado");
					ClearPreviewSelectedServerAddress(state);
					if (!used_legacy_group_selection)
					{
						state->preview_selected_server_group = -1;
					}
					state->preview_selected_server_slot = -1;
					RefreshPreviewStatusFromConnectServer(state);
				}
				break;
			case PreviewHitTarget_ButtonCancel:
				SetPreviewFocus(state, app, PreviewHitTarget_None);
				if (IsPreviewLoginStage(state) && state->legacy_login_ui_ready)
				{
					if (platform::HandleLegacyLoginCancelAction())
					{
						RefreshPreviewStatusFromGameServer(state);
						break;
					}
				}
				if (state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_Connecting ||
					state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_Connected ||
					state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_JoinReady ||
					state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_LoginSucceeded ||
					state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_LoginFailed ||
					state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_CharacterListReady ||
					state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_MapJoinReady)
				{
					platform::DisconnectGameServerBootstrap(&state->game_server_bootstrap, "GameServer cancelado");
					ClearPreviewSelectedServerAddress(state);
					state->preview_focused_target = PreviewHitTarget_None;
					SetPreviewStatusMessage(state, "Selecione uma sala");
				}
				else if (state->connect_server_bootstrap.status == platform::ConnectServerBootstrapStatus_Connecting)
				{
					state->preview_selected_server_slot = -1;
					platform::DisconnectConnectServerBootstrap(&state->connect_server_bootstrap, "Conexao cancelada");
					SetPreviewStatusMessage(state, state->connect_server_bootstrap.status_message);
				}
				else if (state->connect_server_bootstrap.status == platform::ConnectServerBootstrapStatus_Connected)
				{
					state->preview_selected_server_slot = -1;
					platform::DisconnectConnectServerBootstrap(&state->connect_server_bootstrap, "Conexao cancelada");
					SetPreviewStatusMessage(state, state->connect_server_bootstrap.status_message);
				}
			else if (state->connect_server_bootstrap.status == platform::ConnectServerBootstrapStatus_Idle &&
					 state->game_server_bootstrap.status == platform::GameServerBootstrapStatus_Idle)
			{
				state->preview_password_value.clear();
				SetPreviewStatusMessage(state, std::string(GetPreviewTargetLabel(state, target)) + " limpou a senha");
			}
			else
			{
				SetPreviewStatusMessage(state, state->connect_server_bootstrap.status_message);
			}
			break;
		case PreviewHitTarget_ButtonMenu:
			SetPreviewFocus(state, app, PreviewHitTarget_None);
			SetPreviewStatusMessage(state, "Menu preview");
			break;
		case PreviewHitTarget_ButtonCredit:
			SetPreviewFocus(state, app, PreviewHitTarget_None);
			SetPreviewStatusMessage(state, "Creditos preview");
			break;
		default:
			break;
		}

log_preview_action:
		__android_log_print(
			ANDROID_LOG_INFO,
			kLogTag,
			"PreviewAction #%u source=%s target=%s gx=%d gy=%d user=%s pass_len=%zu",
			state->preview_action_count,
			source != NULL ? source : "unknown",
			GetPreviewActionTargetLabel(state, target),
			state->game_mouse_state.x,
			state->game_mouse_state.y,
			state->preview_account_value.empty() ? "-" : state->preview_account_value.c_str(),
			state->preview_password_value.size());
	}

	int32_t HandleKeyInput(android_app* app, BootstrapState* state, AInputEvent* event)
	{
		if (state == NULL || event == NULL)
		{
			return 0;
		}

		const int32_t action = AKeyEvent_getAction(event);
		const int32_t key_code = AKeyEvent_getKeyCode(event);
		const int32_t meta_state = AKeyEvent_getMetaState(event);
		if (action != AKEY_EVENT_ACTION_DOWN)
		{
			return 0;
		}

		if (key_code == AKEYCODE_BACK && IsPreviewFieldTarget(state->preview_focused_target))
		{
			SetPreviewFocus(state, app, PreviewHitTarget_None);
			__android_log_print(ANDROID_LOG_INFO, kLogTag, "PreviewKey back closes field focus");
			return 1;
		}

		if (key_code == AKEYCODE_TAB)
		{
			const PreviewHitTarget next_target =
				state->preview_focused_target == PreviewHitTarget_FieldAccount ? PreviewHitTarget_FieldPassword : PreviewHitTarget_FieldAccount;
			SetPreviewFocus(state, app, next_target);
			__android_log_print(ANDROID_LOG_INFO, kLogTag, "PreviewKey tab focus=%s", GetPreviewTargetLabel(state, next_target));
			return 1;
		}

		if (key_code == AKEYCODE_ENTER || key_code == AKEYCODE_NUMPAD_ENTER)
		{
			if (state->preview_focused_target == PreviewHitTarget_FieldAccount)
			{
				SetPreviewFocus(state, app, PreviewHitTarget_FieldPassword);
				__android_log_print(ANDROID_LOG_INFO, kLogTag, "PreviewKey enter focus=%s", GetPreviewTargetLabel(state, PreviewHitTarget_FieldPassword));
				return 1;
			}

			if (state->preview_focused_target == PreviewHitTarget_FieldPassword)
			{
				TriggerPreviewAction(state, app, PreviewHitTarget_ButtonLogin, "enter");
				return 1;
			}
		}

		if (key_code == AKEYCODE_DEL)
		{
			if (!ErasePreviewCharacter(state))
			{
				return 0;
			}

			state->preview_key_event_count += 1;
			__android_log_print(
				ANDROID_LOG_INFO,
				kLogTag,
				"PreviewKey #%u field=%s key=DEL len=%zu",
				state->preview_key_event_count,
				GetPreviewTargetLabel(state, state->preview_focused_target),
				GetFocusedPreviewFieldValue(state) != NULL ? GetFocusedPreviewFieldValue(state)->size() : 0u);
			return 1;
		}

		const char character = TranslateAndroidKeyCodeToAscii(key_code, meta_state);
		if (character == '\0' || !AppendPreviewCharacter(state, character))
		{
			return 0;
		}

		state->preview_key_event_count += 1;
		__android_log_print(
			ANDROID_LOG_INFO,
			kLogTag,
			"PreviewKey #%u field=%s char=%s len=%zu",
			state->preview_key_event_count,
			GetPreviewTargetLabel(state, state->preview_focused_target),
			state->preview_focused_target == PreviewHitTarget_FieldPassword ? "*" : std::string(1, character).c_str(),
			GetFocusedPreviewFieldValue(state) != NULL ? GetFocusedPreviewFieldValue(state)->size() : 0u);
		return 1;
	}

	void HandleCommand(android_app* app, int32_t command)
	{
		BootstrapState* state = (BootstrapState*)app->userData;
		switch (command)
		{
		case APP_CMD_INIT_WINDOW:
			CreateIfPossible(state, app);
			break;

		case APP_CMD_TERM_WINDOW:
			DestroySurface(state);
			break;

		case APP_CMD_GAINED_FOCUS:
			if (state != NULL)
			{
				state->has_focus = true;
			}
			break;

		case APP_CMD_LOST_FOCUS:
			if (state != NULL)
			{
				state->has_focus = false;
				platform::ApplyInactiveWindowMouseState(&state->game_mouse_state);
				HideSoftKeyboard(app);
			}
			break;

		default:
			break;
		}
	}

	int32_t HandleInput(android_app* app, AInputEvent* event)
	{
		if (app == NULL || event == NULL)
		{
			return 0;
		}

		BootstrapState* state = (BootstrapState*)app->userData;
		if (state == NULL)
		{
			return 0;
		}

		const int32_t event_type = AInputEvent_getType(event);
		if (event_type == AINPUT_EVENT_TYPE_KEY)
		{
			return HandleKeyInput(app, state, event);
		}

		if (event_type != AINPUT_EVENT_TYPE_MOTION)
		{
			return 0;
		}

		const int32_t action = AMotionEvent_getAction(event);
		const int32_t action_mask = (action & AMOTION_EVENT_ACTION_MASK);
		const size_t pointer_count = AMotionEvent_getPointerCount(event);
		if (pointer_count == 0)
		{
			return 0;
		}

		const size_t action_pointer_index =
			static_cast<size_t>((action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
		const size_t safe_action_index = action_pointer_index < pointer_count ? action_pointer_index : 0;
		const int action_pointer_id = AMotionEvent_getPointerId(event, safe_action_index);
		const float action_x = AMotionEvent_getX(event, safe_action_index);
		const float action_y = AMotionEvent_getY(event, safe_action_index);

		state->touch_event_counter += 1;
		state->multi_touch_active = (pointer_count > 1);
		platform::PrepareGameMouseStateForMessage(&state->game_mouse_state);

		switch (action_mask)
		{
		case AMOTION_EVENT_ACTION_DOWN:
		case AMOTION_EVENT_ACTION_POINTER_DOWN:
			state->touch_active = true;
			if (action_mask == AMOTION_EVENT_ACTION_DOWN || state->touch_pointer_id < 0)
			{
				state->touch_pointer_id = action_pointer_id;
			}
			{
				const int tracked_index = FindPointerIndex(event, state->touch_pointer_id);
				const int safe_tracked_index = tracked_index >= 0 ? tracked_index : static_cast<int>(safe_action_index);
				state->touch_x = AMotionEvent_getX(event, safe_tracked_index);
				state->touch_y = AMotionEvent_getY(event, safe_tracked_index);
				}
				platform::UpdateGameMousePositionFromWindowPixels(&state->game_mouse_state, state->game_mouse_metrics, state->touch_x, state->touch_y);
				platform::SetLegacyClientCursorPosition(state->game_mouse_state.x, state->game_mouse_state.y);
				platform::SyncLegacyClientMouseState(&state->game_mouse_state);
				if (action_mask == AMOTION_EVENT_ACTION_DOWN)
				{
				platform::SetGameMouseLeftButtonDown(&state->game_mouse_state);
				state->preview_pressed_target = HitTestPreviewTarget(state, state->touch_x, state->touch_y);
				__android_log_print(
					ANDROID_LOG_INFO,
					kLogTag,
					"PreviewTouchTarget down=%s gx=%d gy=%d",
					GetPreviewActionTargetLabel(state, state->preview_pressed_target),
					state->game_mouse_state.x,
					state->game_mouse_state.y);
				if (IsPreviewFieldTarget(state->preview_pressed_target))
				{
					SetPreviewFocus(state, app, static_cast<PreviewHitTarget>(state->preview_pressed_target));
					__android_log_print(
						ANDROID_LOG_INFO,
						kLogTag,
						"PreviewField focus=%s gx=%d gy=%d",
						GetPreviewTargetLabel(state, state->preview_pressed_target),
						state->game_mouse_state.x,
						state->game_mouse_state.y);
				}
			}
			state->touch_move_log_counter = 0;
			LogTouchEvent(state, GetMotionActionName(action_mask), action_pointer_id, action_x, action_y, pointer_count);
			return 1;

		case AMOTION_EVENT_ACTION_MOVE:
		{
			int tracked_index = FindPointerIndex(event, state->touch_pointer_id);
			if (tracked_index < 0)
			{
				tracked_index = 0;
				state->touch_pointer_id = AMotionEvent_getPointerId(event, 0);
			}

			state->touch_active = true;
				state->touch_x = AMotionEvent_getX(event, tracked_index);
				state->touch_y = AMotionEvent_getY(event, tracked_index);
				platform::UpdateGameMousePositionFromWindowPixels(&state->game_mouse_state, state->game_mouse_metrics, state->touch_x, state->touch_y);
				platform::SetLegacyClientCursorPosition(state->game_mouse_state.x, state->game_mouse_state.y);
				platform::SyncLegacyClientMouseState(&state->game_mouse_state);
				state->touch_move_log_counter += 1;
			if (state->touch_move_log_counter == 1 || (state->touch_move_log_counter % 20u) == 0u)
			{
				LogTouchEvent(state, GetMotionActionName(action_mask), state->touch_pointer_id, state->touch_x, state->touch_y, pointer_count);
			}
			return 1;
		}

		case AMOTION_EVENT_ACTION_UP:
		case AMOTION_EVENT_ACTION_POINTER_UP:
		{
				state->touch_x = action_x;
				state->touch_y = action_y;
				platform::UpdateGameMousePositionFromWindowPixels(&state->game_mouse_state, state->game_mouse_metrics, state->touch_x, state->touch_y);
				platform::SetLegacyClientCursorPosition(state->game_mouse_state.x, state->game_mouse_state.y);
				platform::SyncLegacyClientMouseState(&state->game_mouse_state);
				const PreviewHitTarget released_target = HitTestPreviewTarget(state, state->touch_x, state->touch_y);
			if (action_mask == AMOTION_EVENT_ACTION_POINTER_UP && pointer_count > 1)
			{
				int next_index = 0;
				if (static_cast<int>(safe_action_index) == next_index && pointer_count > 1)
				{
					next_index = 1;
				}
				state->touch_pointer_id = AMotionEvent_getPointerId(event, next_index);
					state->touch_x = AMotionEvent_getX(event, next_index);
					state->touch_y = AMotionEvent_getY(event, next_index);
					platform::UpdateGameMousePositionFromWindowPixels(&state->game_mouse_state, state->game_mouse_metrics, state->touch_x, state->touch_y);
					platform::SetLegacyClientCursorPosition(state->game_mouse_state.x, state->game_mouse_state.y);
					platform::SyncLegacyClientMouseState(&state->game_mouse_state);
					state->touch_active = true;
				state->multi_touch_active = ((pointer_count - 1) > 1);
			}
			else
			{
				platform::SetGameMouseLeftButtonUp(&state->game_mouse_state);
				if (IsPreviewButtonTarget(state->preview_pressed_target) && state->preview_pressed_target == released_target)
				{
					TriggerPreviewAction(state, app, state->preview_pressed_target, "tap");
				}
				else if (!IsPreviewFieldTarget(released_target) && state->preview_pressed_target == PreviewHitTarget_None)
				{
					SetPreviewFocus(state, app, PreviewHitTarget_None);
				}
				state->preview_pressed_target = PreviewHitTarget_None;
				state->touch_active = false;
				state->multi_touch_active = false;
				state->touch_pointer_id = -1;
			}
			LogTouchEvent(state, GetMotionActionName(action_mask), action_pointer_id, action_x, action_y, pointer_count);
			return 1;
		}

		case AMOTION_EVENT_ACTION_CANCEL:
				state->touch_x = action_x;
				state->touch_y = action_y;
				platform::UpdateGameMousePositionFromWindowPixels(&state->game_mouse_state, state->game_mouse_metrics, state->touch_x, state->touch_y);
				platform::SetLegacyClientCursorPosition(state->game_mouse_state.x, state->game_mouse_state.y);
				platform::ResetGameMouseState(&state->game_mouse_state, state->game_mouse_state.x, state->game_mouse_state.y);
				platform::SyncLegacyClientMouseState(&state->game_mouse_state);
			state->preview_pressed_target = PreviewHitTarget_None;
			state->touch_active = false;
			state->multi_touch_active = false;
			state->touch_pointer_id = -1;
			state->touch_move_log_counter = 0;
			LogTouchEvent(state, GetMotionActionName(action_mask), action_pointer_id, action_x, action_y, pointer_count);
			return 1;

		default:
			return 0;
		}
	}

	bool IsBootstrapReadyForOriginalGameClient(const BootstrapState* state)
	{
		return state->has_game_data
			&& state->has_core_bootstrap
			&& state->has_client_ini_bootstrap
			&& state->has_packet_crypto_bootstrap
			&& state->has_language_assets
			&& state->has_global_text_bootstrap;
	}

	bool TryInitializeOriginalGameClient(BootstrapState* state)
	{
		if (state->original_game_initialized)
		{
			return true;
		}

		if (!IsBootstrapReadyForOriginalGameClient(state))
		{
			return false;
		}

		const int surface_width = state->egl_window.GetWidth();
		const int surface_height = state->egl_window.GetHeight();

		__android_log_print(ANDROID_LOG_INFO, kLogTag,
			"Initializing original game client (%dx%d)", surface_width, surface_height);

		const bool ok = platform::InitializeOriginalGameClient(surface_width, surface_height);
		if (!ok)
		{
			__android_log_print(ANDROID_LOG_ERROR, kLogTag,
				"InitializeOriginalGameClient failed");
			return false;
		}

		state->original_game_initialized = true;
		__android_log_print(ANDROID_LOG_INFO, kLogTag,
			"Original game client initialized, SceneFlag=%d", SceneFlag);
		return true;
	}

	void RenderOriginalGameFrame(BootstrapState* state)
	{
		// Sync touch/mouse input to legacy engine globals
		platform::SyncLegacyClientMouseState(&state->game_mouse_state);

		// Poll network I/O (replaces WSAAsyncSelect on Android)
		platform::PollSocketIO(SocketClient);

		// Process incoming packets
		ProtocolCompiler(&SocketClient, 0, 0);

		// Render if frame pacing allows
		if (CheckRenderNextFrame())
		{
			RenderScene(g_hDC);
			// NOTE: Do NOT call SwapBuffers() here — the game's own rendering
			// pipeline calls PresentCurrentFrame() (eglSwapBuffers) inside
			// MainScene()/RenderTitleSceneUI(). A second swap would display
			// an undefined/black back buffer instead of the rendered content.
		}

		// Advance one-shot mouse events (push/pop/doubleclick)
		platform::AdvanceLegacyClientMouseState(&state->game_mouse_state);
	}

	void RenderBootstrapFrame(BootstrapState* state)
	{
		PollPreviewConnectServer(state);
		PollPreviewGameServer(state);
		const bool preview_character_stage = IsPreviewCharacterStage(state);
		platform::SetLegacyClientSceneFlag(preview_character_stage ? CHARACTER_SCENE : LOG_IN_SCENE);
		if (state->legacy_login_ui_ready)
		{
			platform::BindLegacyLoginFlowRuntime(
				&state->connect_server_bootstrap,
				&state->game_server_bootstrap,
				&state->preview_selected_server_group,
				&state->preview_selected_server_slot,
				&state->preview_account_value,
				&state->preview_password_value,
				&state->preview_status_message);
			platform::SyncLegacyLoginUiCredentials(
				state->preview_account_value.c_str(),
				state->preview_password_value.c_str(),
				state->preview_focused_target == PreviewHitTarget_FieldAccount,
				state->preview_focused_target == PreviewHitTarget_FieldPassword);
			platform::SyncLegacyLoginUiVisibility(
				!preview_character_stage && IsPreviewServerBrowserStage(state),
				!preview_character_stage && IsPreviewLoginStage(state));
			if (!preview_character_stage)
			{
				platform::UpdateLegacyLoginUiRuntime(&state->game_mouse_state, 1.0 / 60.0);
			}
			if (!preview_character_stage && IsPreviewLoginStage(state))
			{
				const int focused_field = platform::GetLegacyLoginFocusedField();
				if (focused_field == 1)
				{
					state->preview_focused_target = PreviewHitTarget_FieldAccount;
				}
				else if (focused_field == 2)
				{
					state->preview_focused_target = PreviewHitTarget_FieldPassword;
				}
				else if (IsPreviewFieldTarget(state->preview_focused_target))
				{
					state->preview_focused_target = PreviewHitTarget_None;
				}
			}
			if (IsPreviewServerBrowserStage(state))
			{
				const int legacy_selected_group = platform::GetLegacySelectedServerGroupIndex();
				if (legacy_selected_group >= 0)
				{
					state->preview_selected_server_group = legacy_selected_group;
				}

				const int legacy_selected_room = platform::GetLegacySelectedServerRoomSlot();
				if (legacy_selected_room >= 0)
				{
					state->preview_selected_server_slot = legacy_selected_room;
				}
			}
		}
		if (state->legacy_character_ui_ready && preview_character_stage)
		{
			platform::BindLegacyCharacterUiRuntime(
				&state->game_server_bootstrap,
				&state->preview_status_message);
			platform::RefreshLegacyCharacterUiRuntime(
				state->egl_window.GetWidth(),
				state->egl_window.GetHeight());
			if (state->game_server_bootstrap.selected_character_slot != 0xFF)
			{
				platform::SelectLegacyCharacterUiSlot(state->game_server_bootstrap.selected_character_slot);
			}
			platform::UpdateLegacyCharacterUiRuntime(&state->game_mouse_state, 1.0 / 60.0);
			SyncSelectedCharacterFromLegacyRuntime(state);

			switch (platform::ConsumeLegacyCharacterUiAction())
			{
			case platform::LegacyCharacterUiAction_Connect:
				if (state->game_server_bootstrap.selected_character_slot != 0xFF)
				{
					if (!platform::RequestJoinMapServerBySlotBootstrap(
						&state->game_server_bootstrap,
						state->game_server_bootstrap.selected_character_slot))
					{
						SetPreviewStatusMessage(state, "Falha ao entrar com personagem");
					}
					else
					{
						RefreshPreviewStatusFromGameServer(state);
					}
				}
				else
				{
					SetPreviewStatusMessage(state, "Selecione um personagem");
				}
				break;
			case platform::LegacyCharacterUiAction_Create:
				break;
			case platform::LegacyCharacterUiAction_Menu:
				SetPreviewStatusMessage(state, "Menu da tela de personagem");
				break;
			case platform::LegacyCharacterUiAction_Delete:
				if (state->game_server_bootstrap.selected_character_slot != 0xFF)
				{
					if (!platform::RequestDeleteCharacterBootstrap(
						&state->game_server_bootstrap,
						state->game_server_bootstrap.selected_character_slot,
						""))
					{
						SetPreviewStatusMessage(state, "Falha ao excluir personagem");
					}
					else
					{
						RefreshPreviewStatusFromGameServer(state);
					}
				}
				else
				{
					SetPreviewStatusMessage(state, "Selecione um personagem");
				}
				break;
			default:
				break;
			}

			{
				const unsigned char create_result = platform::ConsumeCreateCharacterResult(&state->game_server_bootstrap);
				if (create_result != 0xFF)
				{
					if (create_result == 1)
					{
						SetPreviewStatusMessage(state, "Personagem criado com sucesso");
					}
					else if (create_result == 2)
					{
						SetPreviewStatusMessage(state, "Nome de personagem ja existe");
					}
					else
					{
						SetPreviewStatusMessage(state, "Falha ao criar personagem");
					}
				}

				const unsigned char delete_result = platform::ConsumeDeleteCharacterResult(&state->game_server_bootstrap);
				if (delete_result != 0xFF)
				{
					if (delete_result == 1)
					{
						SetPreviewStatusMessage(state, "Personagem excluido com sucesso");
					}
					else if (delete_result == 0)
					{
						SetPreviewStatusMessage(state, "Erro: personagem em guilda");
					}
					else if (delete_result == 3)
					{
						SetPreviewStatusMessage(state, "Erro: personagem com item bloqueado");
					}
					else
					{
						SetPreviewStatusMessage(state, "Codigo pessoal incorreto");
					}
				}
			}
		}
		state->phase += 0.01f;
		float red = 0.10f + 0.05f * sinf(state->phase);
		float green = 0.12f + 0.04f * sinf(state->phase * 0.7f);
		float blue = 0.16f + 0.06f * cosf(state->phase * 0.5f);
		if (state->has_game_data)
		{
			green += 0.18f;
		}
		else
		{
			red += 0.22f;
			green += 0.03f;
		}
		if (state->has_core_bootstrap)
		{
			blue += 0.18f;
		}
			if (state->has_client_ini_bootstrap)
			{
				green += 0.08f;
				blue += 0.06f;
			}
			if (state->has_runtime_config)
			{
				green += 0.03f;
			}
			if (state->has_language_assets)
			{
				green += 0.04f;
				blue += 0.03f;
			}
			if (state->has_global_text_bootstrap)
			{
				green += 0.05f;
			}
			if (state->has_interface_preview)
			{
				red += 0.02f;
				green += 0.06f;
				blue += 0.04f;
			}
			if (state->has_packet_crypto_bootstrap)
			{
				red += 0.05f;
			blue += 0.08f;
		}

		platform::RenderBackend& render_backend = platform::GetRenderBackend();
		render_backend.SetViewport(0, 0, state->egl_window.GetWidth(), state->egl_window.GetHeight(), state->egl_window.GetHeight());
		render_backend.SetClearColor(red, green, blue, 1.0f);
		render_backend.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		DrawInterfacePreview(state);
		DrawTouchOverlay(state);
		render_backend.PresentCurrentFrame();
	}
}

void android_main(struct android_app* app)
{
	BootstrapState state = {};
	state.has_focus = false;
	state.has_game_data = false;
	state.has_core_bootstrap = false;
	state.has_client_config = false;
	state.has_client_ini_bootstrap = false;
	state.has_runtime_config = false;
	state.has_language_assets = false;
	state.has_global_text_bootstrap = false;
	state.has_interface_preview = false;
	state.legacy_login_ui_ready = false;
	state.has_packet_crypto_bootstrap = false;
	state.touch_active = false;
	state.multi_touch_active = false;
	state.touch_x = 0.0f;
	state.touch_y = 0.0f;
	state.touch_pointer_id = -1;
	state.preview_pressed_target = PreviewHitTarget_None;
	state.preview_focused_target = PreviewHitTarget_None;
	state.touch_move_log_counter = 0;
	state.touch_event_counter = 0;
	state.preview_action_count = 0;
	state.preview_key_event_count = 0;
	state.preview_selected_server_group = -1;
	state.preview_selected_server_slot = -1;
	state.preview_connect_logged_status = static_cast<int>(platform::ConnectServerBootstrapStatus_Idle);
	state.preview_connect_logged_attempt = 0;
	state.preview_game_logged_status = static_cast<int>(platform::GameServerBootstrapStatus_Idle);
	state.preview_game_logged_attempt = 0;
	ResetPreviewHitAreas(&state);
	state.interface_background_uses_legacy_login = false;
	state.interface_uses_object95_scene = false;
	state.auto_login_user.clear();
	state.preview_account_value.clear();
	state.preview_password_value.clear();
	state.preview_status_message.clear();
	platform::ResetGameMouseState(&state.game_mouse_state, 0, 0);
	state.game_mouse_metrics = platform::CreateGameMouseMetrics(0, 0, 1.0f);
	platform::InitializeClientRuntimeConfig(NULL, &state.runtime_config);
	platform::InitializeLegacyClientRuntime();
	platform::ApplyLegacyClientRuntimeConfig(&state.runtime_config);
	platform::SyncLegacyClientMouseState(&state.game_mouse_state);
	platform::InitializeConnectServerBootstrapState(&state.connect_server_bootstrap);
	platform::InitializeGameServerBootstrapState(&state.game_server_bootstrap);
	state.phase = 0.0f;
	state.original_game_initialized = false;

	app->userData = &state;
	app->onAppCmd = HandleCommand;
	app->onInputEvent = HandleInput;

	LogInfo("android_main started");

	while (true)
	{
		int events = 0;
		android_poll_source* source = NULL;
		const int poll_timeout = (state.egl_window.IsReady() && state.has_focus) ? 0 : -1;
		const int ident = ALooper_pollOnce(poll_timeout, NULL, &events, (void**)&source);

		if (ident >= 0)
		{
			if (source != NULL)
			{
				source->process(app, source);
			}

			if (app->destroyRequested != 0)
			{
				DestroySurface(&state);
				LogInfo("android_main finished");
				return;
			}

			continue;
		}

		if (!state.original_game_initialized)
		{
			TryInitializeOriginalGameClient(&state);
		}

		if (state.original_game_initialized)
		{
			RenderOriginalGameFrame(&state);
		}
		else
		{
			RenderBootstrapFrame(&state);
		}
	}
}

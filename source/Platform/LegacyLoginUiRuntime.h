#pragma once

#if defined(__ANDROID__)

#include <string>
#include <vector>

#include "Platform/GameMouseInput.h"
#include "Platform/GameConnectServerBootstrap.h"
#include "Platform/GameServerBootstrap.h"

namespace platform
{
	struct LegacyLoginUiButtonState
	{
		bool visible;
		bool checked;
		int current_frame;
		int x;
		int y;
		int width;
		int height;
		std::string label;
	};

	struct LegacyLoginUiRectState
	{
		bool visible;
		int x;
		int y;
		int width;
		int height;
	};

	struct LegacyLoginUiBottomState
	{
		bool ready;
		LegacyLoginUiButtonState menu_button;
		LegacyLoginUiButtonState credit_button;
		int deco_x;
		int deco_y;
		int deco_width;
		int deco_height;
		bool deco_visible;
	};

	struct LegacyLoginUiServerBrowserState
	{
		bool ready;
		int selected_button_index;
		bool selected_group_is_pvp;
		std::vector<LegacyLoginUiButtonState> group_buttons;
		std::vector<LegacyLoginUiButtonState> room_buttons;
		std::vector<LegacyLoginUiRectState> room_gauges;
		LegacyLoginUiRectState description_panel_rect;
		LegacyLoginUiRectState left_deco_rect;
		LegacyLoginUiRectState right_deco_rect;
		LegacyLoginUiRectState left_arrow_rect;
		LegacyLoginUiRectState right_arrow_rect;
		std::vector<std::string> description_lines;
		std::vector<std::string> pvp_notice_lines;
	};

	struct LegacyLoginUiLoginState
	{
		bool ready;
		bool account_focused;
		bool password_focused;
		LegacyLoginUiRectState panel_rect;
		LegacyLoginUiRectState account_field_rect;
		LegacyLoginUiRectState password_field_rect;
		LegacyLoginUiRectState account_input_rect;
		LegacyLoginUiRectState password_input_rect;
		LegacyLoginUiButtonState login_button;
		LegacyLoginUiButtonState cancel_button;
		int account_label_x;
		int account_label_y;
		int password_label_x;
		int password_label_y;
		int server_name_x;
		int server_name_y;
		int server_name_width;
		std::string account_label;
		std::string password_label;
		std::string server_name;
	};

	bool InitializeLegacyLoginUiRuntime(int screen_width, int screen_height, std::string* out_error_message);
	void ShutdownLegacyLoginUiRuntime();
	void RefreshLegacyLoginUiRuntime(int screen_width, int screen_height, int selected_group_index);
	void BindLegacyLoginFlowRuntime(
		ConnectServerBootstrapState* connect_server_state,
		GameServerBootstrapState* game_server_state,
		int* selected_group_index,
		int* selected_room_slot,
		std::string* account_value,
		std::string* password_value,
		std::string* status_message);
	void SyncLegacyLoginUiCredentials(const char* account, const char* password, bool focus_account, bool focus_password);
	bool HandleLegacyLoginMainButtonAction(int button_index);
	bool HandleLegacyServerGroupAction(int button_index);
	bool HandleLegacyServerRoomAction(int room_slot);
	bool HandleLegacyLoginConfirmAction();
	bool HandleLegacyLoginCancelAction();
	bool FocusLegacyLoginAccountField();
	bool FocusLegacyLoginPasswordField();
	void ClearLegacyLoginFieldFocus();
	bool AppendLegacyLoginFieldCharacter(char value);
	bool BackspaceLegacyLoginFieldCharacter();
	int GetLegacyLoginFocusedField();
	int GetLegacySelectedServerGroupIndex();
	int GetLegacySelectedServerRoomSlot();
	bool SelectLegacyPrimaryServerGroup();
	void UpdateLegacyLoginUiRuntime(GameMouseState* mouse_state, double delta_tick);
	void SyncLegacyLoginUiVisibility(bool show_server_browser, bool show_login_fields);
	bool CollectLegacyLoginUiBottomState(LegacyLoginUiBottomState* out_state);
	bool CollectLegacyLoginUiServerBrowserState(int selected_group_index, LegacyLoginUiServerBrowserState* out_state);
	bool CollectLegacyLoginUiLoginState(LegacyLoginUiLoginState* out_state);
}

#endif

#pragma once

#if defined(__ANDROID__)

#include <string>
#include <vector>

#include "Platform/GameMouseInput.h"
#include "Platform/GameServerBootstrap.h"
#include "Platform/LegacyLoginUiRuntime.h"

namespace platform
{
	enum LegacyCharacterUiAction
	{
		LegacyCharacterUiAction_None = 0,
		LegacyCharacterUiAction_Create,
		LegacyCharacterUiAction_Menu,
		LegacyCharacterUiAction_Connect,
		LegacyCharacterUiAction_Delete,
	};

	struct LegacyCharacterUiState
	{
		bool ready;
		bool account_block_item;
		bool create_window_visible;
		LegacyLoginUiRectState info_rect;
		LegacyLoginUiRectState deco_rect;
		LegacyLoginUiRectState create_window_rect;
		LegacyLoginUiRectState create_input_rect;
		LegacyLoginUiRectState create_desc_rect;
		LegacyLoginUiButtonState create_button;
		LegacyLoginUiButtonState menu_button;
		LegacyLoginUiButtonState connect_button;
		LegacyLoginUiButtonState delete_button;
		LegacyLoginUiButtonState create_ok_button;
		LegacyLoginUiButtonState create_cancel_button;
	};

	struct LegacyCharacterUiEntryState
	{
		bool visible;
		unsigned char slot;
		unsigned short level;
		bool selected;
		std::string name;
	};

	bool InitializeLegacyCharacterUiRuntime(int screen_width, int screen_height, std::string* out_error_message);
	void ShutdownLegacyCharacterUiRuntime();
	void BindLegacyCharacterUiRuntime(GameServerBootstrapState* game_server_state, std::string* status_message);
	void RefreshLegacyCharacterUiRuntime(int screen_width, int screen_height);
	void UpdateLegacyCharacterUiRuntime(GameMouseState* mouse_state, double delta_tick);
	void RenderLegacyCharacterSceneRuntime(int x, int y, int width, int height);
	void RenderLegacyCharacterUiRuntime();
	bool CollectLegacyCharacterUiState(LegacyCharacterUiState* out_state);
	bool CollectLegacyCharacterUiEntries(std::vector<LegacyCharacterUiEntryState>* out_entries);
	bool HandleLegacyCharacterButtonAction(LegacyCharacterUiAction action);
	bool HandleLegacyCharacterCreateConfirmAction();
	bool HandleLegacyCharacterCreateCancelAction();
	void SetLegacyCharacterUiCreateWindowVisible(bool visible);
	bool IsLegacyCharacterUiCreateWindowVisible();
	void SetLegacyCharacterUiStatusMessage(const char* message);
	void SelectLegacyCharacterUiSlot(int character_slot);
	int GetLegacyCharacterUiSelectedSlot();
	int GetLegacyCharacterUiMaxCharacters();
	LegacyCharacterUiAction ConsumeLegacyCharacterUiAction();
}

#endif

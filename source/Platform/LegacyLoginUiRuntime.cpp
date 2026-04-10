#include "stdafx.h"

#if defined(__ANDROID__)

#include "Platform/LegacyLoginUiRuntime.h"

#include "_TextureIndex.h"
#include "Button.h"
#include "GaugeBar.h"
#include "GlobalBitmap.h"
#include "GlobalText.h"
#include "GameCensorship.h"
#include "Input.h"
#include "LoginMainWin.h"
#include "LoginWin.h"
#include "ServerSelWin.h"
#include "ServerListManager.h"
#include "UIControls.h"
#include "UIMng.h"

#include "Platform/GameAssetPath.h"
#include "Platform/LegacyAndroidUiCompat.h"
#include "Platform/LegacyClientRuntime.h"
#include "Platform/LegacyLoginServerListRuntime.h"

#include <cstring>

namespace
{
	struct LegacyLoginUiRuntimeState
	{
		bool initialized;
		int screen_width;
		int screen_height;
		int virtual_width;
		int virtual_height;
		platform::ConnectServerBootstrapState* connect_server_state;
		platform::GameServerBootstrapState* game_server_state;
		int* selected_group_index;
		int* selected_room_slot;
		std::string* account_value;
		std::string* password_value;
		std::string* status_message;
	};

	LegacyLoginUiRuntimeState g_legacy_login_ui_runtime = {};

	bool LoadLegacyLoginTexture(GLuint bitmap_index, const char* asset_path)
	{
		if (Bitmaps.FindTexture(bitmap_index) != NULL)
		{
			return true;
		}

		return Bitmaps.LoadImage(bitmap_index, asset_path != NULL ? asset_path : "", GL_LINEAR, GL_CLAMP_TO_EDGE);
	}

	bool InitializeLegacyLoginTextures()
	{
		return
			LoadLegacyLoginTexture(BITMAP_LOG_IN, "Interface\\cha_bt.tga") &&
			LoadLegacyLoginTexture(BITMAP_LOG_IN + 1, "Interface\\server_b2_all.tga") &&
			LoadLegacyLoginTexture(BITMAP_LOG_IN + 2, "Interface\\server_b2_loding.jpg") &&
			LoadLegacyLoginTexture(BITMAP_LOG_IN + 3, "Interface\\server_deco_all.tga") &&
			LoadLegacyLoginTexture(BITMAP_LOG_IN + 4, "Interface\\server_menu_b_all.tga") &&
			LoadLegacyLoginTexture(BITMAP_LOG_IN + 5, "Interface\\server_credit_b_all.tga") &&
			LoadLegacyLoginTexture(BITMAP_LOG_IN + 6, "Interface\\deco.tga") &&
			LoadLegacyLoginTexture(BITMAP_LOG_IN + 7, "Custom\\NewInterface\\login_back.jpg") &&
			LoadLegacyLoginTexture(BITMAP_LOG_IN + 8, "Custom\\NewInterface\\btn_medium.jpg") &&
			LoadLegacyLoginTexture(BITMAP_LOG_IN + 11, "Interface\\server_ex03.tga") &&
			LoadLegacyLoginTexture(BITMAP_LOG_IN + 12, "Interface\\server_ex01.tga") &&
			LoadLegacyLoginTexture(BITMAP_LOG_IN + 13, "Interface\\server_ex02.jpg") &&
			LoadLegacyLoginTexture(BITMAP_BUTTON, "Interface\\message_ok_b_all.tga") &&
			LoadLegacyLoginTexture(BITMAP_BUTTON + 1, "Interface\\loding_cancel_b_all.tga") &&
			LoadLegacyLoginTexture(BITMAP_BUTTON + 2, "Interface\\message_close_b_all.tga");
	}

	int ComputeLegacyVirtualHeight(int screen_height)
	{
		if (screen_height <= 0)
		{
			return 480;
		}

		return 480;
	}

	int ComputeLegacyVirtualWidth(int screen_width, int screen_height)
	{
		if (screen_width <= 0 || screen_height <= 0)
		{
			return 640;
		}

		const float ui_scale = static_cast<float>(screen_height) / 480.0f;
		if (ui_scale <= 0.0f)
		{
			return 640;
		}

		const int virtual_width = static_cast<int>(static_cast<float>(screen_width) / ui_scale + 0.5f);
		return virtual_width > 0 ? virtual_width : 640;
	}

	void ApplyLegacyLoginLayout(int screen_width, int screen_height)
	{
		g_legacy_login_ui_runtime.screen_width = screen_width;
		g_legacy_login_ui_runtime.screen_height = screen_height;
		g_legacy_login_ui_runtime.virtual_width = ComputeLegacyVirtualWidth(screen_width, screen_height);
		g_legacy_login_ui_runtime.virtual_height = ComputeLegacyVirtualHeight(screen_height);
		CInput::Instance().AndroidConfigure(
			reinterpret_cast<HWND>(1),
			g_legacy_login_ui_runtime.virtual_width,
			g_legacy_login_ui_runtime.virtual_height);
		CUIMng& ui_mng = CUIMng::Instance();

		const int base_y = int(567.0f / 600.0f * static_cast<float>(g_legacy_login_ui_runtime.virtual_height));
		ui_mng.m_LoginMainWin.SetPosition(
			30,
			base_y - ui_mng.m_LoginMainWin.GetHeight() - 11);

		ui_mng.m_ServerSelWin.SetPosition(
			(g_legacy_login_ui_runtime.virtual_width - ui_mng.m_ServerSelWin.GetWidth()) / 2,
			(g_legacy_login_ui_runtime.virtual_height - ui_mng.m_ServerSelWin.GetHeight()) / 2);

		ui_mng.m_LoginWin.SetPosition(
			(g_legacy_login_ui_runtime.virtual_width - ui_mng.m_LoginWin.GetWidth()) / 2,
			(g_legacy_login_ui_runtime.virtual_height - ui_mng.m_LoginWin.GetHeight()) * 2 / 3);
	}

	void CopyButtonState(CButton* button, platform::LegacyLoginUiButtonState* out_state)
	{
		if (out_state == NULL)
		{
			return;
		}

		out_state->visible = button != NULL && button->IsShow();
		out_state->checked = button != NULL && button->IsCheck();
		out_state->current_frame = button != NULL ? button->GetNowFrame() : 0;
		out_state->x = button != NULL ? button->GetXPos() : 0;
		out_state->y = button != NULL ? button->GetYPos() : 0;
		out_state->width = button != NULL ? button->GetWidth() : 0;
		out_state->height = button != NULL ? button->GetHeight() : 0;
		out_state->label = (button != NULL && button->GetText() != NULL) ? button->GetText() : "";
	}

	void CopyRectState(int x, int y, int width, int height, platform::LegacyLoginUiRectState* out_state)
	{
		if (out_state == NULL)
		{
			return;
		}

		out_state->visible = width > 0 && height > 0;
		out_state->x = x;
		out_state->y = y;
		out_state->width = width;
		out_state->height = height;
	}

	void CopySpriteRectState(CSprite* sprite, platform::LegacyLoginUiRectState* out_state)
	{
		if (sprite == NULL)
		{
			CopyRectState(0, 0, 0, 0, out_state);
			return;
		}

		CopyRectState(sprite->GetXPos(), sprite->GetYPos(), sprite->GetWidth(), sprite->GetHeight(), out_state);
		if (out_state != NULL)
		{
			out_state->visible = sprite->IsShow();
		}
	}

	void CopyGaugeRectState(CGaugeBar* gauge, platform::LegacyLoginUiRectState* out_state)
	{
		if (gauge == NULL)
		{
			CopyRectState(0, 0, 0, 0, out_state);
			return;
		}

		CopyRectState(gauge->GetXPos(), gauge->GetYPos(), gauge->GetWidth(), gauge->GetHeight(), out_state);
		if (out_state != NULL)
		{
			out_state->visible = gauge->IsShow();
		}
	}

	void CopyWindowRectState(CWin* window, platform::LegacyLoginUiRectState* out_state)
	{
		if (window == NULL)
		{
			CopyRectState(0, 0, 0, 0, out_state);
			return;
		}

		CopyRectState(window->GetXPos(), window->GetYPos(), window->GetWidth(), window->GetHeight(), out_state);
		if (out_state != NULL)
		{
			out_state->visible = window->IsShow();
		}
	}

	void SetBoundStatusMessage(const std::string& message)
	{
		if (g_legacy_login_ui_runtime.status_message != NULL)
		{
			*g_legacy_login_ui_runtime.status_message = message;
		}
	}

	void ClearSelectedGameServerAddress()
	{
		if (g_legacy_login_ui_runtime.connect_server_state == NULL)
		{
			return;
		}

		g_legacy_login_ui_runtime.connect_server_state->server_address_requested = false;
		g_legacy_login_ui_runtime.connect_server_state->server_address_received = false;
		g_legacy_login_ui_runtime.connect_server_state->requested_server_index = 0;
		g_legacy_login_ui_runtime.connect_server_state->selected_game_server_host.clear();
		g_legacy_login_ui_runtime.connect_server_state->selected_game_server_port = 0;
	}

	void SyncInputText(CUITextInputBox* input_box, const char* expected_text)
	{
		if (input_box == NULL)
		{
			return;
		}

		char current_text[MAX_TEXT_LENGTH + 1] = { 0 };
		input_box->GetText(current_text, MAX_TEXT_LENGTH);
		const char* safe_expected_text = expected_text != NULL ? expected_text : "";
		if (strcmp(current_text, safe_expected_text) != 0)
		{
			input_box->SetText(safe_expected_text);
		}
	}

	void SyncBoundCredentialsFromControls()
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return;
		}

		CUIMng& ui_mng = CUIMng::Instance();
		char account[MAX_ID_SIZE + 1] = { 0 };
		char password[MAX_PASSWORD_SIZE + 1] = { 0 };
		if (ui_mng.m_LoginWin.GetIDInputBox() != NULL)
		{
			ui_mng.m_LoginWin.GetIDInputBox()->GetText(account, MAX_ID_SIZE + 1);
		}
		if (ui_mng.m_LoginWin.GetPassInputBox() != NULL)
		{
			ui_mng.m_LoginWin.GetPassInputBox()->GetText(password, MAX_PASSWORD_SIZE + 1);
		}

		if (g_legacy_login_ui_runtime.account_value != NULL)
		{
			*g_legacy_login_ui_runtime.account_value = account;
		}
		if (g_legacy_login_ui_runtime.password_value != NULL)
		{
			*g_legacy_login_ui_runtime.password_value = password;
		}
	}
}

int SeparateTextIntoLines(const char* text, char* separated, int max_line, int line_size)
{
	if (text == NULL || separated == NULL || max_line <= 0 || line_size <= 0)
	{
		return 0;
	}

	memset(separated, 0, static_cast<size_t>(max_line) * static_cast<size_t>(line_size));
	int line_count = 0;
	const char* cursor = text;
	while (*cursor != '\0' && line_count < max_line)
	{
		int write_index = 0;
		const char* line_start = cursor;
		const char* last_space = NULL;
		while (*cursor != '\0' && *cursor != '\n' && write_index < (line_size - 1))
		{
			if (*cursor == ' ')
			{
				last_space = cursor;
			}
			++cursor;
			++write_index;
		}

		if (*cursor != '\0' && *cursor != '\n' && write_index >= (line_size - 1) && last_space != NULL && last_space > line_start)
		{
			cursor = last_space;
			write_index = static_cast<int>(last_space - line_start);
		}

		strncpy(separated + line_count * line_size, line_start, write_index);
		separated[line_count * line_size + write_index] = '\0';
		++line_count;

		while (*cursor == ' ' || *cursor == '\n' || *cursor == '\r')
		{
			++cursor;
		}
	}

	return line_count;
}

namespace platform
{
	bool InitializeLegacyLoginUiRuntime(int screen_width, int screen_height, std::string* out_error_message)
	{
		if (g_legacy_login_ui_runtime.initialized)
		{
			ApplyLegacyLoginLayout(screen_width, screen_height);
			return true;
		}

		if (!InitializeLegacyLoginTextures())
		{
			if (out_error_message != NULL)
			{
				*out_error_message = "Legacy login textures failed to load";
			}
			return false;
		}

		CUIMng& ui_mng = CUIMng::Instance();
		ui_mng.Create();
		ui_mng.CreateLoginScene();
		ui_mng.m_LoginMainWin.Show(true);
		ui_mng.m_ServerSelWin.Show(true);
		ui_mng.m_LoginWin.Show(true);

		ApplyLegacyLoginLayout(screen_width, screen_height);
		g_legacy_login_ui_runtime.initialized = true;
		return true;
	}

	void ShutdownLegacyLoginUiRuntime()
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return;
		}

		CUIMng::Instance().Release();
		Bitmaps.UnloadAllImages();
		memset(&g_legacy_login_ui_runtime, 0, sizeof(g_legacy_login_ui_runtime));
	}

	void RefreshLegacyLoginUiRuntime(int screen_width, int screen_height, int selected_group_index)
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return;
		}

		ApplyLegacyLoginLayout(screen_width, screen_height);
		CUIMng& ui_mng = CUIMng::Instance();

		std::vector<LegacyLoginServerGroupState> groups;
		if (CollectLegacyLoginServerGroups(&groups) && !groups.empty())
		{
			int selected_button_index = -1;
			for (size_t index = 0; index < groups.size(); ++index)
			{
				if (static_cast<int>(groups[index].group_index) == selected_group_index)
				{
					selected_button_index = groups[index].button_position;
					break;
				}
			}

			if (selected_button_index < 0 && !groups.empty())
			{
				selected_button_index = groups[0].button_position;
			}

			ui_mng.m_ServerSelWin.SetSelectedServerButtonIndex(selected_button_index);
			ui_mng.m_ServerSelWin.SyncSelectedServerButtonCheck();
		}

		ui_mng.m_ServerSelWin.UpdateDisplay();
	}

	void BindLegacyLoginFlowRuntime(
		ConnectServerBootstrapState* connect_server_state,
		GameServerBootstrapState* game_server_state,
		int* selected_group_index,
		int* selected_room_slot,
		std::string* account_value,
		std::string* password_value,
		std::string* status_message)
	{
		g_legacy_login_ui_runtime.connect_server_state = connect_server_state;
		g_legacy_login_ui_runtime.game_server_state = game_server_state;
		g_legacy_login_ui_runtime.selected_group_index = selected_group_index;
		g_legacy_login_ui_runtime.selected_room_slot = selected_room_slot;
		g_legacy_login_ui_runtime.account_value = account_value;
		g_legacy_login_ui_runtime.password_value = password_value;
		g_legacy_login_ui_runtime.status_message = status_message;
	}

	void SyncLegacyLoginUiCredentials(const char* account, const char* password, bool focus_account, bool focus_password)
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return;
		}

		CUIMng& ui_mng = CUIMng::Instance();
		SyncInputText(ui_mng.m_LoginWin.GetIDInputBox(), account);
		SyncInputText(ui_mng.m_LoginWin.GetPassInputBox(), password);
		if (focus_account && ui_mng.m_LoginWin.GetIDInputBox() != NULL)
		{
			ui_mng.m_LoginWin.GetIDInputBox()->GiveFocus(FALSE);
		}
		else if (focus_password && ui_mng.m_LoginWin.GetPassInputBox() != NULL)
		{
			ui_mng.m_LoginWin.GetPassInputBox()->GiveFocus(FALSE);
		}
	}

	bool HandleLegacyLoginMainButtonAction(int button_index)
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return false;
		}

		switch (button_index)
		{
		case LMW_BTN_MENU:
			SetBoundStatusMessage("Menu preview");
			return true;
		case LMW_BTN_CREDIT:
			SetBoundStatusMessage("Creditos preview");
			return true;
		default:
			return false;
		}
	}

	bool HandleLegacyServerGroupAction(int button_index)
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return false;
		}

		CServerGroup* group = g_ServerListManager->GetServerGroupByBtnPos(button_index);
		if (group == NULL)
		{
			return false;
		}

		CUIMng& ui_mng = CUIMng::Instance();
		ui_mng.m_ServerSelWin.SetSelectedServerButtonIndex(button_index);
		ui_mng.m_ServerSelWin.SyncSelectedServerButtonCheck();
		ui_mng.m_ServerSelWin.UpdateDisplay();

		if (g_legacy_login_ui_runtime.selected_group_index != NULL)
		{
			*g_legacy_login_ui_runtime.selected_group_index = group->m_iServerIndex;
		}
		if (g_legacy_login_ui_runtime.selected_room_slot != NULL)
		{
			*g_legacy_login_ui_runtime.selected_room_slot = 0;
		}

		if (g_legacy_login_ui_runtime.game_server_state != NULL)
		{
			DisconnectGameServerBootstrap(g_legacy_login_ui_runtime.game_server_state, "GameServer reiniciado");
		}
		ClearSelectedGameServerAddress();

		if (group->m_szDescription[0] != '\0')
		{
			SetBoundStatusMessage(group->m_szDescription);
		}
		else
		{
			SetBoundStatusMessage(std::string("Grupo selecionado: ") + group->m_szName);
		}
		return true;
	}

	bool HandleLegacyServerRoomAction(int room_slot)
	{
		if (!g_legacy_login_ui_runtime.initialized ||
			g_legacy_login_ui_runtime.selected_group_index == NULL ||
			g_legacy_login_ui_runtime.connect_server_state == NULL)
		{
			return false;
		}

		unsigned short connect_index = 0;
		if (!TrySelectLegacyLoginServerEntry(*g_legacy_login_ui_runtime.selected_group_index, room_slot, &connect_index))
		{
			return false;
		}

		if (g_legacy_login_ui_runtime.selected_room_slot != NULL)
		{
			*g_legacy_login_ui_runtime.selected_room_slot = room_slot;
		}

		CUIMng& ui_mng = CUIMng::Instance();
		const CServerGroup* selected_group = ui_mng.m_ServerSelWin.GetSelectedServerGroup();
		CServerInfo* selected_server = selected_group != NULL ? const_cast<CServerGroup*>(selected_group)->GetServerInfo(room_slot) : NULL;
		if (selected_group != NULL && selected_server != NULL)
		{
			int censorship_index = SEASON3A::CGameCensorship::STATE_12;
			if (selected_group->m_bPvPServer)
			{
				censorship_index = SEASON3A::CGameCensorship::STATE_18;
			}
			else if (0x01 & selected_server->m_byNonPvP)
			{
				censorship_index = SEASON3A::CGameCensorship::STATE_15;
			}

			const bool test_server = selected_group->m_iSequence == 0;
			g_ServerListManager->SetSelectServerInfo(
				const_cast<unicode::t_char*>(selected_group->m_szName),
				selected_server->m_iIndex,
				censorship_index,
				selected_server->m_byNonPvP,
				test_server);
		}

		if (g_legacy_login_ui_runtime.game_server_state != NULL)
		{
			DisconnectGameServerBootstrap(g_legacy_login_ui_runtime.game_server_state, "GameServer reiniciado");
		}
		ClearSelectedGameServerAddress();

		if (g_legacy_login_ui_runtime.connect_server_state->status == ConnectServerBootstrapStatus_Connected &&
			g_legacy_login_ui_runtime.connect_server_state->server_list_received)
		{
			if (!RequestConnectServerAddressBootstrap(g_legacy_login_ui_runtime.connect_server_state, connect_index))
			{
				SetBoundStatusMessage(g_legacy_login_ui_runtime.connect_server_state->status_message);
				return false;
			}

			SetBoundStatusMessage(g_legacy_login_ui_runtime.connect_server_state->status_message);
			return true;
		}

		SetBoundStatusMessage("Sala selecionada");
		return true;
	}

	bool HandleLegacyLoginConfirmAction()
	{
		if (!g_legacy_login_ui_runtime.initialized || g_legacy_login_ui_runtime.game_server_state == NULL)
		{
			return false;
		}

		CUIMng& ui_mng = CUIMng::Instance();
		char account[MAX_ID_SIZE + 1] = { 0 };
		char password[MAX_PASSWORD_SIZE + 1] = { 0 };
		if (ui_mng.m_LoginWin.GetIDInputBox() != NULL)
		{
			ui_mng.m_LoginWin.GetIDInputBox()->GetText(account, MAX_ID_SIZE + 1);
		}
		if (ui_mng.m_LoginWin.GetPassInputBox() != NULL)
		{
			ui_mng.m_LoginWin.GetPassInputBox()->GetText(password, MAX_PASSWORD_SIZE + 1);
		}

		if (g_legacy_login_ui_runtime.account_value != NULL)
		{
			*g_legacy_login_ui_runtime.account_value = account;
		}
		if (g_legacy_login_ui_runtime.password_value != NULL)
		{
			*g_legacy_login_ui_runtime.password_value = password;
		}

		if (account[0] == '\0')
		{
			SetBoundStatusMessage("Digite a conta");
			return false;
		}
		if (password[0] == '\0')
		{
			SetBoundStatusMessage("Digite a senha");
			return false;
		}

		GameServerBootstrapState* game_server_state = g_legacy_login_ui_runtime.game_server_state;
		game_server_state->account = account;
		game_server_state->password = password;

		if (!game_server_state->configured)
		{
			SetBoundStatusMessage("Selecione uma sala");
			return false;
		}

		if (!game_server_state->socket_open &&
			(game_server_state->status == GameServerBootstrapStatus_Idle ||
			 game_server_state->status == GameServerBootstrapStatus_Failed))
		{
			if (!StartGameServerBootstrap(game_server_state))
			{
				SetBoundStatusMessage(game_server_state->status_message);
				return false;
			}

			game_server_state->login_pending = true;
			SetBoundStatusMessage(game_server_state->status_message);
			return true;
		}

		if (!game_server_state->join_server_success)
		{
			game_server_state->login_pending = true;
			SetBoundStatusMessage(game_server_state->status_message);
			return true;
		}

		if (!RequestGameServerLoginBootstrap(game_server_state))
		{
			SetBoundStatusMessage(game_server_state->status_message);
			return false;
		}

		SetBoundStatusMessage(game_server_state->status_message);
		return true;
	}

	bool HandleLegacyLoginCancelAction()
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return false;
		}

		if (g_legacy_login_ui_runtime.game_server_state != NULL &&
			g_legacy_login_ui_runtime.game_server_state->status != GameServerBootstrapStatus_Idle)
		{
			DisconnectGameServerBootstrap(g_legacy_login_ui_runtime.game_server_state, "GameServer cancelado");
			ClearSelectedGameServerAddress();
			SetBoundStatusMessage("Selecione uma sala");
			return true;
		}

		if (g_legacy_login_ui_runtime.connect_server_state != NULL &&
			(g_legacy_login_ui_runtime.connect_server_state->server_address_requested ||
			 g_legacy_login_ui_runtime.connect_server_state->server_address_received))
		{
			ClearSelectedGameServerAddress();
			SetBoundStatusMessage("Selecione uma sala");
			return true;
		}

		if (g_legacy_login_ui_runtime.connect_server_state != NULL &&
			g_legacy_login_ui_runtime.connect_server_state->status != ConnectServerBootstrapStatus_Idle)
		{
			DisconnectConnectServerBootstrap(g_legacy_login_ui_runtime.connect_server_state, "Conexao cancelada");
			SetBoundStatusMessage(g_legacy_login_ui_runtime.connect_server_state->status_message);
			return true;
		}

		if (g_legacy_login_ui_runtime.password_value != NULL)
		{
			g_legacy_login_ui_runtime.password_value->clear();
		}
		CUIMng& ui_mng = CUIMng::Instance();
		if (ui_mng.m_LoginWin.GetPassInputBox() != NULL)
		{
			ui_mng.m_LoginWin.GetPassInputBox()->SetText("");
		}
		SetBoundStatusMessage("Senha limpa");
		return true;
	}

	bool FocusLegacyLoginAccountField()
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return false;
		}

		CUITextInputBox* input_box = CUIMng::Instance().m_LoginWin.GetIDInputBox();
		if (input_box == NULL)
		{
			return false;
		}

		input_box->GiveFocus(FALSE);
		return true;
	}

	bool FocusLegacyLoginPasswordField()
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return false;
		}

		CUITextInputBox* input_box = CUIMng::Instance().m_LoginWin.GetPassInputBox();
		if (input_box == NULL)
		{
			return false;
		}

		input_box->GiveFocus(FALSE);
		return true;
	}

	void ClearLegacyLoginFieldFocus()
	{
		ClearLegacyAndroidTextInputFocus();
	}

	int GetLegacyLoginFocusedField()
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return 0;
		}

		CUIMng& ui_mng = CUIMng::Instance();
		if (IsLegacyAndroidTextInputFocused(ui_mng.m_LoginWin.GetIDInputBox()))
		{
			return 1;
		}
		if (IsLegacyAndroidTextInputFocused(ui_mng.m_LoginWin.GetPassInputBox()))
		{
			return 2;
		}
		return 0;
	}

	int GetLegacySelectedServerGroupIndex()
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return -1;
		}

		const CServerGroup* selected_group = CUIMng::Instance().m_ServerSelWin.GetSelectedServerGroup();
		if (selected_group != NULL)
		{
			return selected_group->m_iServerIndex;
		}

		if (g_legacy_login_ui_runtime.selected_group_index != NULL)
		{
			return *g_legacy_login_ui_runtime.selected_group_index;
		}

		return -1;
	}

	int GetLegacySelectedServerRoomSlot()
	{
		if (!g_legacy_login_ui_runtime.initialized || g_legacy_login_ui_runtime.selected_room_slot == NULL)
		{
			return -1;
		}

		return *g_legacy_login_ui_runtime.selected_room_slot;
	}

	bool SelectLegacyPrimaryServerGroup()
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return false;
		}

		const int primary_group_index = GetLegacyLoginPrimaryGroupIndex();
		std::vector<LegacyLoginServerGroupState> groups;
		if (CollectLegacyLoginServerGroups(&groups))
		{
			for (size_t index = 0; index < groups.size(); ++index)
			{
				if (groups[index].group_index == primary_group_index && groups[index].button_position >= 0)
				{
					return HandleLegacyServerGroupAction(groups[index].button_position);
				}
			}
		}

		return HandleLegacyServerGroupAction(0);
	}

	bool AppendLegacyLoginFieldCharacter(char value)
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return false;
		}

		CUIMng& ui_mng = CUIMng::Instance();
		CUITextInputBox* input_box = NULL;
		if (IsLegacyAndroidTextInputFocused(ui_mng.m_LoginWin.GetIDInputBox()))
		{
			input_box = ui_mng.m_LoginWin.GetIDInputBox();
		}
		else if (IsLegacyAndroidTextInputFocused(ui_mng.m_LoginWin.GetPassInputBox()))
		{
			input_box = ui_mng.m_LoginWin.GetPassInputBox();
		}

		if (!AppendLegacyAndroidTextInputChar(input_box, value))
		{
			return false;
		}

		SyncBoundCredentialsFromControls();
		return true;
	}

	bool BackspaceLegacyLoginFieldCharacter()
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return false;
		}

		CUIMng& ui_mng = CUIMng::Instance();
		CUITextInputBox* input_box = NULL;
		if (IsLegacyAndroidTextInputFocused(ui_mng.m_LoginWin.GetIDInputBox()))
		{
			input_box = ui_mng.m_LoginWin.GetIDInputBox();
		}
		else if (IsLegacyAndroidTextInputFocused(ui_mng.m_LoginWin.GetPassInputBox()))
		{
			input_box = ui_mng.m_LoginWin.GetPassInputBox();
		}

		if (!BackspaceLegacyAndroidTextInput(input_box))
		{
			return false;
		}

		SyncBoundCredentialsFromControls();
		return true;
	}

	void UpdateLegacyLoginUiRuntime(GameMouseState* mouse_state, double delta_tick)
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return;
		}

		CUIMng::Instance().Update(delta_tick);
		SyncBoundCredentialsFromControls();
		AdvanceLegacyClientMouseState(mouse_state);
	}

	void SyncLegacyLoginUiVisibility(bool show_server_browser, bool show_login_fields)
	{
		if (!g_legacy_login_ui_runtime.initialized)
		{
			return;
		}

		CUIMng& ui_mng = CUIMng::Instance();
		if (!ui_mng.m_LoginMainWin.IsShow())
		{
			ui_mng.m_LoginMainWin.Show(true);
		}

		if (show_server_browser != ui_mng.m_ServerSelWin.IsShow())
		{
			if (show_server_browser)
			{
				ui_mng.ShowWin(&ui_mng.m_ServerSelWin);
			}
			else
			{
				ui_mng.HideWin(&ui_mng.m_ServerSelWin);
			}
		}

		if (show_login_fields != ui_mng.m_LoginWin.IsShow())
		{
			if (show_login_fields)
			{
				ui_mng.ShowWin(&ui_mng.m_LoginWin);
			}
			else
			{
				ui_mng.HideWin(&ui_mng.m_LoginWin);
			}
		}
	}

	bool CollectLegacyLoginUiBottomState(LegacyLoginUiBottomState* out_state)
	{
		if (out_state == NULL || !g_legacy_login_ui_runtime.initialized)
		{
			return false;
		}

		*out_state = LegacyLoginUiBottomState();
		out_state->ready = true;
		CUIMng& ui_mng = CUIMng::Instance();

		CopyButtonState(const_cast<CButton*>(ui_mng.m_LoginMainWin.GetButton(LMW_BTN_MENU)), &out_state->menu_button);
		CopyButtonState(const_cast<CButton*>(ui_mng.m_LoginMainWin.GetButton(LMW_BTN_CREDIT)), &out_state->credit_button);

		CSprite* deco = const_cast<CSprite*>(ui_mng.m_LoginMainWin.GetDecoSprite());
		out_state->deco_visible = deco != NULL && deco->IsShow();
		out_state->deco_x = deco != NULL ? deco->GetXPos() : 0;
		out_state->deco_y = deco != NULL ? deco->GetYPos() : 0;
		out_state->deco_width = deco != NULL ? deco->GetWidth() : 0;
		out_state->deco_height = deco != NULL ? deco->GetHeight() : 0;
		return true;
	}

	bool CollectLegacyLoginUiServerBrowserState(int selected_group_index, LegacyLoginUiServerBrowserState* out_state)
	{
		if (out_state == NULL || !g_legacy_login_ui_runtime.initialized)
		{
			return false;
		}

		RefreshLegacyLoginUiRuntime(
			g_legacy_login_ui_runtime.screen_width,
			g_legacy_login_ui_runtime.screen_height,
			selected_group_index);

		*out_state = LegacyLoginUiServerBrowserState();
		out_state->ready = true;
		CUIMng& ui_mng = CUIMng::Instance();
		out_state->selected_button_index = ui_mng.m_ServerSelWin.GetSelectedServerButtonIndex();
		CopyWindowRectState(const_cast<CWinEx*>(ui_mng.m_ServerSelWin.GetDescriptionWindow()), &out_state->description_panel_rect);
		CopySpriteRectState(const_cast<CSprite*>(ui_mng.m_ServerSelWin.GetButtonDeco(0)), &out_state->left_deco_rect);
		CopySpriteRectState(const_cast<CSprite*>(ui_mng.m_ServerSelWin.GetButtonDeco(1)), &out_state->right_deco_rect);
		CopySpriteRectState(const_cast<CSprite*>(ui_mng.m_ServerSelWin.GetArrowDeco(0)), &out_state->left_arrow_rect);
		CopySpriteRectState(const_cast<CSprite*>(ui_mng.m_ServerSelWin.GetArrowDeco(1)), &out_state->right_arrow_rect);

		for (int index = 0; index < SSW_SERVER_G_MAX; ++index)
		{
			LegacyLoginUiButtonState button_state = {};
			CopyButtonState(const_cast<CButton*>(ui_mng.m_ServerSelWin.GetServerGroupButton(index)), &button_state);
			out_state->group_buttons.push_back(button_state);
		}

		for (int index = 0; index < SSW_SERVER_MAX; ++index)
		{
			LegacyLoginUiButtonState button_state = {};
			CopyButtonState(const_cast<CButton*>(ui_mng.m_ServerSelWin.GetServerButton(index)), &button_state);
			out_state->room_buttons.push_back(button_state);

			LegacyLoginUiRectState gauge_state = {};
			CopyGaugeRectState(const_cast<CGaugeBar*>(ui_mng.m_ServerSelWin.GetServerGauge(index)), &gauge_state);
			out_state->room_gauges.push_back(gauge_state);
		}

		for (int index = 0; index < SSW_DESC_LINE_MAX; ++index)
		{
			const unicode::t_char* description_line = ui_mng.m_ServerSelWin.GetDescriptionLine(index);
			out_state->description_lines.push_back(description_line != NULL ? description_line : "");
		}

		const CServerGroup* selected_group = ui_mng.m_ServerSelWin.GetSelectedServerGroup();
		out_state->selected_group_is_pvp = selected_group != NULL && selected_group->m_bPvPServer;
		if (out_state->selected_group_is_pvp)
		{
			for (int index = 565; index <= 567; ++index)
			{
				out_state->pvp_notice_lines.push_back(GlobalText[index] != NULL ? GlobalText[index] : "");
			}
		}

		return true;
	}

	bool CollectLegacyLoginUiLoginState(LegacyLoginUiLoginState* out_state)
	{
		if (out_state == NULL || !g_legacy_login_ui_runtime.initialized)
		{
			return false;
		}

		*out_state = LegacyLoginUiLoginState();
		out_state->ready = true;
		CUIMng& ui_mng = CUIMng::Instance();

		const int window_x = ui_mng.m_LoginWin.GetXPos();
		const int window_y = ui_mng.m_LoginWin.GetYPos();
		const float windows_x = static_cast<float>(
			g_legacy_login_ui_runtime.virtual_width > 0 ? g_legacy_login_ui_runtime.virtual_width : 640);
		const CSprite* account_field_sprite = ui_mng.m_LoginWin.GetInputBoxSprite(0);
		const CSprite* password_field_sprite = ui_mng.m_LoginWin.GetInputBoxSprite(1);

		CopyRectState(
			static_cast<int>(windows_x * 0.5f) - 100,
			window_y + 30,
			200,
			137,
			&out_state->panel_rect);

		if (account_field_sprite != NULL)
		{
			CopySpriteRectState(const_cast<CSprite*>(account_field_sprite), &out_state->account_field_rect);
		}
		else
		{
			CopyRectState(
				static_cast<int>(windows_x * 0.5f) - 40,
				window_y + 101,
				114,
				21,
				&out_state->account_field_rect);
		}
		if (password_field_sprite != NULL)
		{
			CopySpriteRectState(const_cast<CSprite*>(password_field_sprite), &out_state->password_field_rect);
		}
		else
		{
			CopyRectState(
				static_cast<int>(windows_x * 0.5f) - 40,
				window_y + 142,
				114,
				21,
				&out_state->password_field_rect);
		}

		CUITextInputBox* id_input = ui_mng.m_LoginWin.GetIDInputBox();
		CUITextInputBox* pass_input = ui_mng.m_LoginWin.GetPassInputBox();
		CopyRectState(
			out_state->account_field_rect.x + 6,
			out_state->account_field_rect.y + 5,
			id_input != NULL ? id_input->GetWidth() : 140,
			id_input != NULL ? id_input->GetHeight() : 14,
			&out_state->account_input_rect);
		CopyRectState(
			out_state->password_field_rect.x + 6,
			out_state->password_field_rect.y + 5,
			pass_input != NULL ? pass_input->GetWidth() : 140,
			pass_input != NULL ? pass_input->GetHeight() : 14,
			&out_state->password_input_rect);

		out_state->account_label_x = window_x + 45;
		out_state->account_label_y = window_y + 110;
		out_state->password_label_x = window_x + 45;
		out_state->password_label_y = window_y + 149;
		out_state->server_name_x = static_cast<int>(windows_x * 0.5f) - 100;
		out_state->server_name_y = window_y + 60;
		out_state->server_name_width = 200;

		out_state->account_label = GlobalText[450] != NULL ? GlobalText[450] : "";
		out_state->password_label = GlobalText[451] != NULL ? GlobalText[451] : "";
		out_state->server_name = g_ServerListManager->GetSelectServerName() != NULL ? g_ServerListManager->GetSelectServerName() : "";
		out_state->account_focused = IsLegacyAndroidTextInputFocused(id_input);
		out_state->password_focused = IsLegacyAndroidTextInputFocused(pass_input);

		out_state->login_button.visible = true;
		out_state->login_button.checked = false;
		out_state->login_button.x = static_cast<int>(windows_x * 0.5f) - 70;
		out_state->login_button.y = window_y + 186;
		out_state->login_button.width = 60;
		out_state->login_button.height = 18;
		out_state->login_button.current_frame =
			CheckMouseIn(
				out_state->login_button.x,
				out_state->login_button.y,
				out_state->login_button.width,
				out_state->login_button.height) ? 1 : 0;
		out_state->login_button.label = GlobalText[452] != NULL ? GlobalText[452] : "";

		out_state->cancel_button.visible = true;
		out_state->cancel_button.checked = false;
		out_state->cancel_button.x = static_cast<int>(windows_x * 0.5f) + 12;
		out_state->cancel_button.y = out_state->login_button.y;
		out_state->cancel_button.width = 60;
		out_state->cancel_button.height = 18;
		out_state->cancel_button.current_frame =
			CheckMouseIn(
				out_state->cancel_button.x,
				out_state->cancel_button.y,
				out_state->cancel_button.width,
				out_state->cancel_button.height) ? 1 : 0;
		out_state->cancel_button.label = GlobalText[453] != NULL ? GlobalText[453] : "";

		return true;
	}
}

#endif

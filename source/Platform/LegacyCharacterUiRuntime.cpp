#include "stdafx.h"

#if defined(__ANDROID__)

#include "Platform/LegacyCharacterUiRuntime.h"

#include "_TextureIndex.h"
#include "CharMakeWin.h"
#include "CharSelMainWin.h"
#include "CharacterList.h"
#include "GlobalBitmap.h"
#include "Input.h"
#include "MapManager.h"
#include "UIMng.h"
#include "ZzzOpenData.h"
#include "ZzzOpenglUtil.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzLodTerrain.h"
#include "ZzzScene.h"
#include "Widescreen.h"

#include "Platform/LegacyClientRuntime.h"

#include <android/log.h>
#include <algorithm>
#include <cstring>

namespace
{
	static const char* kLegacyCharacterLogTag = "mu_android_char";

	enum
	{
		kLegacyCharacterCreateButtonOk = 0,
		kLegacyCharacterCreateButtonCancel = 1,
	};

	struct LegacyCharacterUiRuntimeState
	{
		bool initialized;
		bool scene_data_loaded;
		bool character_render_assets_loaded;
		bool scene_initialized;
		int scene_world;
		int last_logged_live_count;
		int last_logged_selected_slot;
		size_t last_logged_server_count;
		int screen_width;
		int screen_height;
		int virtual_width;
		int virtual_height;
		platform::GameServerBootstrapState* game_server_state;
		std::string* status_message;
		CCharSelMainWin char_sel_main_win;
		CCharMakeWin char_make_win;
		platform::LegacyCharacterUiAction pending_action;
	};

	LegacyCharacterUiRuntimeState g_legacy_character_ui_runtime = {};

	bool LoadLegacyCharacterTexture(GLuint bitmap_index, const char* asset_path, GLint filter = GL_LINEAR, GLint wrap = GL_CLAMP_TO_EDGE)
	{
		if (Bitmaps.FindTexture(bitmap_index) != NULL)
		{
			return true;
		}

		return Bitmaps.LoadImage(bitmap_index, asset_path != NULL ? asset_path : "", filter, wrap);
	}

	bool InitializeLegacyCharacterTextures()
	{
		return
			LoadLegacyCharacterTexture(BITMAP_LOG_IN, "Interface\\cha_id.tga") &&
			LoadLegacyCharacterTexture(BITMAP_LOG_IN + 1, "Interface\\cha_bt.tga") &&
			LoadLegacyCharacterTexture(BITMAP_LOG_IN + 2, "Interface\\deco.tga") &&
			LoadLegacyCharacterTexture(BITMAP_LOG_IN + 3, "Interface\\b_create.tga") &&
			LoadLegacyCharacterTexture(BITMAP_LOG_IN + 4, "Interface\\server_menu_b_all.tga") &&
			LoadLegacyCharacterTexture(BITMAP_LOG_IN + 5, "Interface\\b_connect.tga") &&
			LoadLegacyCharacterTexture(BITMAP_LOG_IN + 6, "Interface\\b_delete.tga") &&
			LoadLegacyCharacterTexture(BITMAP_LOG_IN + 7, "Interface\\character_ex.tga") &&
			LoadLegacyCharacterTexture(BITMAP_LOG_IN + 11, "Interface\\server_ex03.tga", GL_NEAREST, GL_REPEAT) &&
			LoadLegacyCharacterTexture(BITMAP_LOG_IN + 12, "Interface\\server_ex01.tga") &&
			LoadLegacyCharacterTexture(BITMAP_LOG_IN + 13, "Interface\\server_ex02.jpg", GL_NEAREST, GL_REPEAT) &&
			LoadLegacyCharacterTexture(BITMAP_EXT_LOG_IN + 2, "Effect\\Impack03.jpg") &&
			LoadLegacyCharacterTexture(BITMAP_EFFECT, "Logo\\chasellight.jpg") &&
			LoadLegacyCharacterTexture(BITMAP_BUTTON, "Interface\\message_ok_b_all.tga") &&
			LoadLegacyCharacterTexture(BITMAP_BUTTON + 1, "Interface\\loding_cancel_b_all.tga") &&
			LoadLegacyCharacterTexture(BITMAP_BUTTON + 2, "Interface\\message_close_b_all.tga");
	}

	int ComputeLegacyVirtualHeight(int screen_height)
	{
		(void)screen_height;
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

	BYTE ChangeServerClassTypeToLegacyClientClassType(BYTE server_class_type)
	{
		return static_cast<BYTE>(
			(((server_class_type >> 4) & 0x01) << 3) |
			(server_class_type >> 5) |
			(((server_class_type >> 3) & 0x01) << 4));
	}

	void ResolveLegacyCharacterSlotLayout(int slot, float* out_x, float* out_y, float* out_angle)
	{
		float x = 0.0f;
		float y = 0.0f;
		float angle = 0.0f;

		switch (slot)
		{
		case 0: x = 8008.0f; y = 18885.0f; angle = 115.0f; break;
		case 1: x = 7986.0f; y = 19145.0f; angle = 90.0f; break;
		case 2: x = 8046.0f; y = 19400.0f; angle = 75.0f; break;
		case 3: x = 8133.0f; y = 19645.0f; angle = 60.0f; break;
		case 4: x = 8282.0f; y = 19845.0f; angle = 35.0f; break;
		default: break;
		}

		if (out_x != NULL)
		{
			*out_x = x;
		}
		if (out_y != NULL)
		{
			*out_y = y;
		}
		if (out_angle != NULL)
		{
			*out_angle = angle;
		}
	}

	void SetBoundStatusMessage(const std::string& message)
	{
		if (g_legacy_character_ui_runtime.status_message != NULL)
		{
			*g_legacy_character_ui_runtime.status_message = message;
		}
	}

	void SetLegacyCharacterCreateWindowVisibleInternal(bool visible)
	{
		g_legacy_character_ui_runtime.char_make_win.Show(visible);
		g_legacy_character_ui_runtime.char_make_win.Active(visible);
		g_legacy_character_ui_runtime.char_sel_main_win.Active(!visible);
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

	void ApplyLegacyCharacterLayout(int screen_width, int screen_height)
	{
		g_legacy_character_ui_runtime.screen_width = screen_width;
		g_legacy_character_ui_runtime.screen_height = screen_height;
		g_legacy_character_ui_runtime.virtual_width = ComputeLegacyVirtualWidth(screen_width, screen_height);
		g_legacy_character_ui_runtime.virtual_height = ComputeLegacyVirtualHeight(screen_height);
		CInput::Instance().AndroidConfigure(
			reinterpret_cast<HWND>(1),
			g_legacy_character_ui_runtime.virtual_width,
			g_legacy_character_ui_runtime.virtual_height);

		const int base_y = int(567.0f / 600.0f * static_cast<float>(g_legacy_character_ui_runtime.virtual_height));
		g_legacy_character_ui_runtime.char_sel_main_win.SetPosition(
			22,
			base_y - g_legacy_character_ui_runtime.char_sel_main_win.GetHeight() - 11);
		g_legacy_character_ui_runtime.char_make_win.SetPosition(
			(g_legacy_character_ui_runtime.virtual_width - 454) / 2,
			(g_legacy_character_ui_runtime.virtual_height - 406) / 2);
	}

	void ClearLegacyCharacterEntry(CHARACTER* character, int slot_index)
	{
		if (character == NULL)
		{
			return;
		}

		character->Object.Live = false;
		character->ID[0] = '\0';
		character->Level = 0;
		character->Class = 0;
		character->Skin = 0;
		character->CtlCode = 0;
		character->GuildStatus = 0;
		character->Key = static_cast<SHORT>(slot_index);
	}

	void EnsureLegacyCharacterSceneDataLoaded()
	{
		if (g_legacy_character_ui_runtime.scene_data_loaded)
		{
			// Continue below so render assets can still be completed lazily.
		}
		else
		{
			OpenCharacterSceneData();
			g_legacy_character_ui_runtime.scene_data_loaded = true;
		}

		if (!g_legacy_character_ui_runtime.character_render_assets_loaded)
		{
			OpenPlayers();
			OpenPlayerTextures();
			OpenItems();
			OpenItemTextures();
			OpenSkills();
			OpenImages();
			g_legacy_character_ui_runtime.character_render_assets_loaded = true;
		}
	}

	void LogLegacyCharacterSceneState(const char* phase)
	{
		int live_count = 0;
		for (int index = 0; index < GetLegacyCharacterUiMaxCharacters(); ++index)
		{
			if (CharactersClient[index].Object.Live)
			{
				++live_count;
			}
		}

		const size_t server_count =
			(g_legacy_character_ui_runtime.game_server_state != NULL)
				? g_legacy_character_ui_runtime.game_server_state->characters.size()
				: 0u;

		if (live_count == g_legacy_character_ui_runtime.last_logged_live_count &&
			SelectedHero == g_legacy_character_ui_runtime.last_logged_selected_slot &&
			server_count == g_legacy_character_ui_runtime.last_logged_server_count)
		{
			return;
		}

		g_legacy_character_ui_runtime.last_logged_live_count = live_count;
		g_legacy_character_ui_runtime.last_logged_selected_slot = SelectedHero;
		g_legacy_character_ui_runtime.last_logged_server_count = server_count;

		const char* selected_name =
			(SelectedHero >= 0 &&
			 SelectedHero < GetLegacyCharacterUiMaxCharacters() &&
			 CharactersClient[SelectedHero].Object.Live)
				? CharactersClient[SelectedHero].ID
				: "-";

		__android_log_print(
			ANDROID_LOG_INFO,
			kLegacyCharacterLogTag,
			"CharacterScene[%s] world=%d server=%zu live=%d selected=%d name=%s",
			phase != NULL ? phase : "tick",
			g_legacy_character_ui_runtime.scene_world,
			server_count,
			live_count,
			SelectedHero,
			selected_name);
	}

	void MoveLegacyCharacterCamera(const vec3_t origin, const vec3_t position, const vec3_t angle)
	{
		vec3_t transformed_position;
		float matrix[3][4];

		CameraAngle[0] = 0.0f;
		CameraAngle[1] = 0.0f;
		CameraAngle[2] = angle[2];
		AngleMatrix(CameraAngle, matrix);
		VectorIRotate(position, matrix, transformed_position);
		VectorAdd(origin, transformed_position, CameraPosition);
		CameraAngle[0] = angle[0];
		CameraAngle[1] = angle[1];
		CameraAngle[2] = angle[2];
	}

	void SyncLegacyCharacterEntriesFromBootstrap()
	{
		EnsureLegacyCharacterSceneDataLoaded();

		const int max_characters = platform::GetLegacyCharacterUiMaxCharacters();
		std::vector<bool> slot_has_entry(static_cast<size_t>(max_characters), false);
		gCharacterList.MaxCharacters = max_characters;

		if (g_legacy_character_ui_runtime.game_server_state == NULL ||
			!g_legacy_character_ui_runtime.game_server_state->character_list_received)
		{
			for (int index = 0; index < max_characters; ++index)
			{
				ClearLegacyCharacterEntry(&CharactersClient[index], index);
			}
			SelectedHero = -1;
			g_legacy_character_ui_runtime.char_sel_main_win.UpdateDisplay();
			return;
		}

		platform::GameServerBootstrapState* state = g_legacy_character_ui_runtime.game_server_state;
		gCharacterList.MaxCharactersAccount =
			state->max_character_count > 0 ? static_cast<int>(state->max_character_count) : max_characters;
		for (size_t index = 0; index < state->characters.size(); ++index)
		{
			const platform::GameServerCharacterEntry& entry = state->characters[index];
			if (entry.slot >= static_cast<unsigned char>(max_characters))
			{
				continue;
			}
			slot_has_entry[entry.slot] = true;

			float position_x = 0.0f;
			float position_y = 0.0f;
			float angle_z = 0.0f;
			ResolveLegacyCharacterSlotLayout(entry.slot, &position_x, &position_y, &angle_z);
			const int legacy_class = ChangeServerClassTypeToLegacyClientClassType(entry.char_set[0]);

			CHARACTER* character = &CharactersClient[entry.slot];
			const bool requires_recreate =
				!character->Object.Live ||
				character->Class != legacy_class ||
				std::strncmp(character->ID, entry.name.c_str(), MAX_ID_SIZE) != 0;

			if (requires_recreate)
			{
				character = CreateHero(
					entry.slot,
					legacy_class,
					0,
					position_x,
					position_y,
					angle_z);
			}
			if (character == NULL)
			{
				continue;
			}

			character->Level = entry.level;
			character->CtlCode = entry.ctl_code;
			character->GuildStatus = entry.guild_status;
			character->Key = static_cast<SHORT>(entry.slot);
			character->Class = legacy_class;
			character->Skin = legacy_class;
			std::strncpy(character->ID, entry.name.c_str(), MAX_ID_SIZE);
			character->ID[MAX_ID_SIZE] = '\0';
			character->Object.Position[0] = position_x;
			character->Object.Position[1] = position_y;
			character->Object.Position[2] = 163.0f;
			character->Object.Angle[0] = 0.0f;
			character->Object.Angle[1] = 0.0f;
			character->Object.Angle[2] = angle_z;
			character->Object.Scale = 1.15f;
			character->Object.Visible = true;
			ChangeCharacterExt(entry.slot, const_cast<BYTE*>(&entry.char_set[1]));
			gCharacterList.SetCharacterPosition(entry.slot);
		}

		for (int index = 0; index < max_characters; ++index)
		{
			if (!slot_has_entry[static_cast<size_t>(index)])
			{
				ClearLegacyCharacterEntry(&CharactersClient[index], index);
			}
		}

		if (state->selected_character_slot != 0xFF &&
			state->selected_character_slot < static_cast<unsigned char>(max_characters) &&
			CharactersClient[state->selected_character_slot].Object.Live)
		{
			SelectedHero = state->selected_character_slot;
			SelectedCharacter = state->selected_character_slot;
		}
		else if (SelectedHero >= 0 &&
			SelectedHero < max_characters &&
			CharactersClient[SelectedHero].Object.Live)
		{
			// Keep current selection.
			SelectedCharacter = SelectedHero;
		}
		else
		{
			SelectedHero = -1;
			SelectedCharacter = -1;
			for (int index = 0; index < max_characters; ++index)
			{
				if (CharactersClient[index].Object.Live)
				{
					SelectedHero = index;
					SelectedCharacter = index;
					break;
				}
			}
		}

		g_legacy_character_ui_runtime.char_sel_main_win.UpdateDisplay();
		LogLegacyCharacterSceneState("sync");
	}

	void ApplyLegacyCharacterSceneState()
	{
		EnableMainRender = true;
		InitCharacterScene = true;
		FogEnable = false;

#ifdef PJH_NEW_SERVER_SELECT_MAP
		gMapManager.WorldActive = WD_74NEW_CHARACTER_SCENE;
#else
		gMapManager.WorldActive = WD_78NEW_CHARACTER_SCENE;
#endif

		GWidescreen.SceneLogin();
	}

	int ResolveLegacyCharacterSceneWorld()
	{
#ifdef PJH_NEW_SERVER_SELECT_MAP
		return WD_74NEW_CHARACTER_SCENE;
#else
		return WD_78NEW_CHARACTER_SCENE;
#endif
	}

	void InitializeLegacyCharacterSceneRuntimeIfNeeded()
	{
		const int desired_world = ResolveLegacyCharacterSceneWorld();
		if (g_legacy_character_ui_runtime.scene_initialized &&
			g_legacy_character_ui_runtime.scene_world == desired_world)
		{
			return;
		}

		ClearCharacters();
		gMapManager.WorldActive = desired_world;
		gMapManager.LoadWorld(gMapManager.WorldActive);
		OpenCharacterSceneData();
		EnsureLegacyCharacterSceneDataLoaded();

		CreateCharacterPointer(&CharacterView, MODEL_FACE + 1, 0, 0);
		CharacterView.Class = 1;
		CharacterView.Object.Kind = 0;

		SelectedHero = -1;
		SelectedCharacter = -1;
		CUIMng::Instance().CreateCharacterScene();

		if (CharacterAttribute != NULL)
		{
			CharacterAttribute->SkillNumber = 0;
			for (int index = 0; index < MAX_MAGIC; ++index)
			{
				CharacterAttribute->Skill[index] = 0;
			}
		}

		if (CharacterMachine != NULL)
		{
			for (int index = EQUIPMENT_WEAPON_RIGHT; index < EQUIPMENT_HELPER; ++index)
			{
				CharacterMachine->Equipment[index].Level = 0;
			}
		}

		g_legacy_character_ui_runtime.scene_initialized = true;
		g_legacy_character_ui_runtime.scene_world = desired_world;
		__android_log_print(
			ANDROID_LOG_INFO,
			kLegacyCharacterLogTag,
			"CharacterScene init world=%d max=%d",
			desired_world,
			GetLegacyCharacterUiMaxCharacters());
	}
}

namespace platform
{
	bool InitializeLegacyCharacterUiRuntime(int screen_width, int screen_height, std::string* out_error_message)
	{
		if (g_legacy_character_ui_runtime.initialized)
		{
			ApplyLegacyCharacterLayout(screen_width, screen_height);
			return true;
		}

		if (!InitializeLegacyCharacterTextures())
		{
			if (out_error_message != NULL)
			{
				*out_error_message = "Legacy character textures failed to load";
			}
			return false;
		}

		EnsureLegacyCharacterSceneDataLoaded();
		g_legacy_character_ui_runtime.char_sel_main_win.Create();
		g_legacy_character_ui_runtime.char_sel_main_win.Show(true);
		g_legacy_character_ui_runtime.char_sel_main_win.Active(true);
		g_legacy_character_ui_runtime.char_make_win.Create();
		SetLegacyCharacterCreateWindowVisibleInternal(false);
		ApplyLegacyCharacterLayout(screen_width, screen_height);
		g_legacy_character_ui_runtime.initialized = true;
		g_legacy_character_ui_runtime.scene_data_loaded = true;
		g_legacy_character_ui_runtime.pending_action = LegacyCharacterUiAction_None;
		return true;
	}

	void ShutdownLegacyCharacterUiRuntime()
	{
		if (!g_legacy_character_ui_runtime.initialized)
		{
			return;
		}

		g_legacy_character_ui_runtime.char_sel_main_win.Release();
		g_legacy_character_ui_runtime.char_make_win.Release();
		g_legacy_character_ui_runtime.initialized = false;
		g_legacy_character_ui_runtime.scene_data_loaded = false;
		g_legacy_character_ui_runtime.scene_initialized = false;
		g_legacy_character_ui_runtime.scene_world = -1;
		g_legacy_character_ui_runtime.screen_width = 0;
		g_legacy_character_ui_runtime.screen_height = 0;
		g_legacy_character_ui_runtime.virtual_width = 0;
		g_legacy_character_ui_runtime.virtual_height = 0;
		g_legacy_character_ui_runtime.character_render_assets_loaded = false;
		g_legacy_character_ui_runtime.last_logged_live_count = -1;
		g_legacy_character_ui_runtime.last_logged_selected_slot = -2;
		g_legacy_character_ui_runtime.last_logged_server_count = static_cast<size_t>(-1);
		g_legacy_character_ui_runtime.game_server_state = NULL;
		g_legacy_character_ui_runtime.status_message = NULL;
		g_legacy_character_ui_runtime.pending_action = LegacyCharacterUiAction_None;
	}

	void BindLegacyCharacterUiRuntime(GameServerBootstrapState* game_server_state, std::string* status_message)
	{
		g_legacy_character_ui_runtime.game_server_state = game_server_state;
		g_legacy_character_ui_runtime.status_message = status_message;
	}

	void RefreshLegacyCharacterUiRuntime(int screen_width, int screen_height)
	{
		if (!g_legacy_character_ui_runtime.initialized)
		{
			return;
		}

		ApplyLegacyCharacterLayout(screen_width, screen_height);
		SyncLegacyCharacterEntriesFromBootstrap();
	}

	void UpdateLegacyCharacterUiRuntime(GameMouseState* mouse_state, double delta_tick)
	{
		if (!g_legacy_character_ui_runtime.initialized)
		{
			return;
		}

		ApplyLegacyCharacterSceneState();
		InitializeLegacyCharacterSceneRuntimeIfNeeded();
		SyncLegacyCharacterEntriesFromBootstrap();
		AdvanceLegacyClientMouseState(mouse_state);
		gCharacterList.MoveCharacterList();
		MoveCharactersClient();
		MoveCharacterClient(&CharacterView);

		for (int index = 0; index < GetLegacyCharacterUiMaxCharacters(); ++index)
		{
			CHARACTER* character = &CharactersClient[index];
			if (!character->Object.Live)
			{
				continue;
			}

			character->Object.Visible = true;
			MoveCharacterClient(character);
		}

		if (g_legacy_character_ui_runtime.char_make_win.IsShow())
		{
			g_legacy_character_ui_runtime.char_make_win.Update(delta_tick);
			return;
		}

		g_legacy_character_ui_runtime.char_sel_main_win.Update(delta_tick);

		CButton* connect_button = const_cast<CButton*>(g_legacy_character_ui_runtime.char_sel_main_win.GetButton(CSMW_BTN_CONNECT));
		CButton* create_button = const_cast<CButton*>(g_legacy_character_ui_runtime.char_sel_main_win.GetButton(CSMW_BTN_CREATE));
		CButton* menu_button = const_cast<CButton*>(g_legacy_character_ui_runtime.char_sel_main_win.GetButton(CSMW_BTN_MENU));
		CButton* delete_button = const_cast<CButton*>(g_legacy_character_ui_runtime.char_sel_main_win.GetButton(CSMW_BTN_DELETE));

		if (connect_button != NULL && connect_button->IsClick())
		{
			g_legacy_character_ui_runtime.pending_action = LegacyCharacterUiAction_Connect;
		}
		else if (create_button != NULL && create_button->IsClick())
		{
			g_legacy_character_ui_runtime.pending_action = LegacyCharacterUiAction_Create;
		}
		else if (menu_button != NULL && menu_button->IsClick())
		{
			g_legacy_character_ui_runtime.pending_action = LegacyCharacterUiAction_Menu;
		}
		else if (delete_button != NULL && delete_button->IsClick())
		{
			g_legacy_character_ui_runtime.pending_action = LegacyCharacterUiAction_Delete;
		}
	}

	void RenderLegacyCharacterSceneRuntime(int x, int y, int width, int height)
	{
		if (!g_legacy_character_ui_runtime.initialized || width <= 0 || height <= 0)
		{
			return;
		}

		const int previous_scene_flag = SceneFlag;
		SceneFlag = CHARACTER_SCENE;
		ApplyLegacyCharacterSceneState();
		InitializeLegacyCharacterSceneRuntimeIfNeeded();

		FogEnable = false;
		CameraViewNear = 20.0f;
		CameraViewFar = 7000.0f;

		vec3_t frustrum_position;
		Vector(9758.0f, 18913.0f, 675.0f, frustrum_position);
		BeginOpengl(x, y, width, height);
		CreateFrustrum(static_cast<float>(width) / 640.0f, frustrum_position);
		CreateScreenVector(MouseX, MouseY, MouseTarget);

		OBJECT* selected_object =
			(SelectedHero >= 0 && SelectedHero < GetLegacyCharacterUiMaxCharacters())
				? &CharactersClient[SelectedHero].Object
				: NULL;
		for (int index = 0; index < GetLegacyCharacterUiMaxCharacters(); ++index)
		{
			CHARACTER* character = &CharactersClient[index];
			OBJECT* object = &character->Object;
			if (!object->Live)
			{
				continue;
			}

			object->Position[2] = 163.0f;
			Vector(0.0f, 0.0f, 0.0f, character->Object.Light);
		}

		if (SelectedHero >= 0 && selected_object != NULL && selected_object->Live)
		{
			EnableAlphaBlend();
			vec3_t terrain_light;
			Vector(1.0f, 1.0f, 1.0f, terrain_light);
			Vector(1.0f, 1.0f, 1.0f, selected_object->Light);
			AddTerrainLight(selected_object->Position[0], selected_object->Position[1], terrain_light, 1, PrimaryTerrainLight);
			DisableAlphaBlend();
		}

		for (int index = 0; index < GetLegacyCharacterUiMaxCharacters(); ++index)
		{
			CHARACTER* character = &CharactersClient[index];
			OBJECT* object = &character->Object;
			if (!object->Live)
			{
				continue;
			}

			if (character->Helper.Type == MODEL_HELPER + 3 || gHelperSystem.CheckHelperType(character->Helper.Type, 4) == 1)
			{
#ifdef PJH_NEW_SERVER_SELECT_MAP
				object->Position[2] = 194.5f;
#else
				object->Position[2] = 55.0f;
#endif
			}
			else
			{
#ifdef PJH_NEW_SERVER_SELECT_MAP
				object->Position[2] = 169.5f;
#else
				object->Position[2] = 30.0f;
#endif
			}

			object->Visible = true;
		}

		LogLegacyCharacterSceneState("render");
		RenderCharactersClient();
		BeginBitmap();
		gCharacterList.RenderCharacterList();
		EndBitmap();
		EndOpengl();

		SceneFlag = previous_scene_flag;
	}

	void RenderLegacyCharacterUiRuntime()
	{
		if (!g_legacy_character_ui_runtime.initialized)
		{
			return;
		}

		g_legacy_character_ui_runtime.char_sel_main_win.Render();
		if (g_legacy_character_ui_runtime.char_make_win.IsShow())
		{
			g_legacy_character_ui_runtime.char_make_win.Render();
		}
	}

	bool CollectLegacyCharacterUiState(LegacyCharacterUiState* out_state)
	{
		if (out_state == NULL || !g_legacy_character_ui_runtime.initialized)
		{
			return false;
		}

		*out_state = LegacyCharacterUiState();
		out_state->ready = true;
		out_state->account_block_item = g_legacy_character_ui_runtime.char_sel_main_win.HasAccountBlockItem();
		out_state->create_window_visible = g_legacy_character_ui_runtime.char_make_win.IsShow();

		CopySpriteRectState(const_cast<CSprite*>(g_legacy_character_ui_runtime.char_sel_main_win.GetBackSprite(CSMW_SPR_INFO)), &out_state->info_rect);
		CopySpriteRectState(const_cast<CSprite*>(g_legacy_character_ui_runtime.char_sel_main_win.GetBackSprite(CSMW_SPR_DECO)), &out_state->deco_rect);
		CopyRectState(
			g_legacy_character_ui_runtime.char_make_win.GetXPos(),
			g_legacy_character_ui_runtime.char_make_win.GetYPos(),
			g_legacy_character_ui_runtime.char_make_win.GetWidth(),
			g_legacy_character_ui_runtime.char_make_win.GetHeight(),
			&out_state->create_window_rect);
		out_state->create_window_rect.visible = out_state->create_window_visible;
		CopySpriteRectState(const_cast<CSprite*>(g_legacy_character_ui_runtime.char_make_win.GetBackSprite(CMW_SPR_INPUT)), &out_state->create_input_rect);
		CopySpriteRectState(const_cast<CSprite*>(g_legacy_character_ui_runtime.char_make_win.GetBackSprite(CMW_SPR_DESC)), &out_state->create_desc_rect);
		CopyButtonState(const_cast<CButton*>(g_legacy_character_ui_runtime.char_sel_main_win.GetButton(CSMW_BTN_CREATE)), &out_state->create_button);
		CopyButtonState(const_cast<CButton*>(g_legacy_character_ui_runtime.char_sel_main_win.GetButton(CSMW_BTN_MENU)), &out_state->menu_button);
		CopyButtonState(const_cast<CButton*>(g_legacy_character_ui_runtime.char_sel_main_win.GetButton(CSMW_BTN_CONNECT)), &out_state->connect_button);
		CopyButtonState(const_cast<CButton*>(g_legacy_character_ui_runtime.char_sel_main_win.GetButton(CSMW_BTN_DELETE)), &out_state->delete_button);
		CopyButtonState(const_cast<CButton*>(g_legacy_character_ui_runtime.char_make_win.GetActionButton(kLegacyCharacterCreateButtonOk)), &out_state->create_ok_button);
		CopyButtonState(const_cast<CButton*>(g_legacy_character_ui_runtime.char_make_win.GetActionButton(kLegacyCharacterCreateButtonCancel)), &out_state->create_cancel_button);
		return true;
	}

	bool CollectLegacyCharacterUiEntries(std::vector<LegacyCharacterUiEntryState>* out_entries)
	{
		if (out_entries == NULL || !g_legacy_character_ui_runtime.initialized)
		{
			return false;
		}

		out_entries->clear();
		const int max_characters = GetLegacyCharacterUiMaxCharacters();
		for (int index = 0; index < max_characters; ++index)
		{
			CHARACTER* character = &CharactersClient[index];
			if (!character->Object.Live)
			{
				continue;
			}

			LegacyCharacterUiEntryState entry = {};
			entry.visible = true;
			entry.slot = static_cast<unsigned char>(index);
			entry.level = static_cast<unsigned short>(character->Level);
			entry.selected = (SelectedHero == index);
			entry.name = character->ID;
			out_entries->push_back(entry);
		}

		return true;
	}

	bool HandleLegacyCharacterButtonAction(LegacyCharacterUiAction action)
	{
		if (!g_legacy_character_ui_runtime.initialized)
		{
			return false;
		}

		CButton* button = NULL;
		switch (action)
		{
		case LegacyCharacterUiAction_Create:
			button = const_cast<CButton*>(g_legacy_character_ui_runtime.char_sel_main_win.GetButton(CSMW_BTN_CREATE));
			break;
		case LegacyCharacterUiAction_Menu:
			button = const_cast<CButton*>(g_legacy_character_ui_runtime.char_sel_main_win.GetButton(CSMW_BTN_MENU));
			break;
		case LegacyCharacterUiAction_Connect:
			button = const_cast<CButton*>(g_legacy_character_ui_runtime.char_sel_main_win.GetButton(CSMW_BTN_CONNECT));
			break;
		case LegacyCharacterUiAction_Delete:
			button = const_cast<CButton*>(g_legacy_character_ui_runtime.char_sel_main_win.GetButton(CSMW_BTN_DELETE));
			break;
		default:
			return false;
		}

		if (button == NULL || !button->IsShow() || !button->IsEnable())
		{
			return false;
		}

		if (action == LegacyCharacterUiAction_Create)
		{
			SetLegacyCharacterCreateWindowVisibleInternal(true);
			SetBoundStatusMessage("Criacao de personagem");
			return true;
		}

		g_legacy_character_ui_runtime.pending_action = action;
		return true;
	}

	bool HandleLegacyCharacterCreateConfirmAction()
	{
		if (!g_legacy_character_ui_runtime.initialized ||
			!g_legacy_character_ui_runtime.char_make_win.IsShow())
		{
			return false;
		}

		CButton* button = const_cast<CButton*>(g_legacy_character_ui_runtime.char_make_win.GetActionButton(kLegacyCharacterCreateButtonOk));
		if (button == NULL || !button->IsShow() || !button->IsEnable())
		{
			return false;
		}

		SetBoundStatusMessage("Criacao de personagem ainda nao integrada");
		SetLegacyCharacterCreateWindowVisibleInternal(false);
		return true;
	}

	bool HandleLegacyCharacterCreateCancelAction()
	{
		if (!g_legacy_character_ui_runtime.initialized ||
			!g_legacy_character_ui_runtime.char_make_win.IsShow())
		{
			return false;
		}

		CButton* button = const_cast<CButton*>(g_legacy_character_ui_runtime.char_make_win.GetActionButton(kLegacyCharacterCreateButtonCancel));
		if (button == NULL || !button->IsShow() || !button->IsEnable())
		{
			return false;
		}

		SetLegacyCharacterCreateWindowVisibleInternal(false);
		return true;
	}

	void SetLegacyCharacterUiCreateWindowVisible(bool visible)
	{
		if (!g_legacy_character_ui_runtime.initialized)
		{
			return;
		}

		SetLegacyCharacterCreateWindowVisibleInternal(visible);
	}

	bool IsLegacyCharacterUiCreateWindowVisible()
	{
		return g_legacy_character_ui_runtime.initialized &&
			g_legacy_character_ui_runtime.char_make_win.IsShow();
	}

	void SetLegacyCharacterUiStatusMessage(const char* message)
	{
		SetBoundStatusMessage(message != NULL ? message : "");
	}

	void SelectLegacyCharacterUiSlot(int character_slot)
	{
		if (!g_legacy_character_ui_runtime.initialized)
		{
			return;
		}

		if (character_slot < 0 || character_slot >= GetLegacyCharacterUiMaxCharacters())
		{
			SelectedHero = -1;
			SelectedCharacter = -1;
		}
		else
		{
			SelectedHero = character_slot;
			SelectedCharacter = character_slot;
			if (g_legacy_character_ui_runtime.game_server_state != NULL)
			{
				g_legacy_character_ui_runtime.game_server_state->selected_character_slot = static_cast<unsigned char>(character_slot);
				for (size_t index = 0; index < g_legacy_character_ui_runtime.game_server_state->characters.size(); ++index)
				{
					if (g_legacy_character_ui_runtime.game_server_state->characters[index].slot == static_cast<unsigned char>(character_slot))
					{
						g_legacy_character_ui_runtime.game_server_state->selected_character_name =
							g_legacy_character_ui_runtime.game_server_state->characters[index].name;
						break;
					}
				}
			}
		}

		g_legacy_character_ui_runtime.char_sel_main_win.UpdateDisplay();
	}

	int GetLegacyCharacterUiSelectedSlot()
	{
		if (!g_legacy_character_ui_runtime.initialized)
		{
			return -1;
		}

		return SelectedHero;
	}

	int GetLegacyCharacterUiMaxCharacters()
	{
		if (g_legacy_character_ui_runtime.game_server_state != NULL &&
			g_legacy_character_ui_runtime.game_server_state->max_character_count > 0)
		{
			return std::max(10, static_cast<int>(g_legacy_character_ui_runtime.game_server_state->max_character_count));
		}

		return 10;
	}

	LegacyCharacterUiAction ConsumeLegacyCharacterUiAction()
	{
		const LegacyCharacterUiAction action = g_legacy_character_ui_runtime.pending_action;
		g_legacy_character_ui_runtime.pending_action = LegacyCharacterUiAction_None;
		switch (action)
		{
		case LegacyCharacterUiAction_Create:
			SetLegacyCharacterCreateWindowVisibleInternal(true);
			SetBoundStatusMessage("Criacao de personagem");
			return LegacyCharacterUiAction_None;
		case LegacyCharacterUiAction_Menu:
			SetBoundStatusMessage("Menu da tela de personagem");
			break;
		case LegacyCharacterUiAction_Delete:
			SetBoundStatusMessage("Exclusao de personagem ainda nao integrada");
			break;
		default:
			break;
		}
		return action;
	}
}

#endif

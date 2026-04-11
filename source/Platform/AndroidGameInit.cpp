#include "stdafx.h"

#if defined(__ANDROID__)

#include "Platform/AndroidGameInit.h"
#include "Platform/GameAssetPath.h"
#include "Platform/GameClientConfig.h"
#include "Platform/GameConfigBootstrap.h"
#include "Platform/LegacyClientRuntime.h"
#include "Platform/RenderBackend.h"

#include "ZzzBMD.h"
#include "ZzzCharacter.h"
#include "ZzzInfomation.h"
#include "ZzzOpenData.h"
#include "ZzzScene.h"
#include "ZzzTexture.h"
#include "ZzzOpenglUtil.h"
#include "Widescreen.h"
#include "Input.h"
#include "UIMng.h"
#include "UIControls.h"
#include "UIManager.h"
#include "NewUISystem.h"
#include "Time/Timer.h"
#include "w_MapProcess.h"
#include "w_PetProcess.h"
#include "_GlobalFunctions.h"
#include "CreateFont.h"

#include "Platform/AndroidWin32Compat.h"

#include <android/log.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
#include <unistd.h>

namespace
{
	const char* kLogTag = "mu_android_init";

	void LogInfo(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		__android_log_vprint(ANDROID_LOG_INFO, kLogTag, format, args);
		va_end(args);
	}
}

extern CTimer* g_pTimer;
extern HWND g_hWnd;
extern HFONT g_hFont;
extern HFONT g_hFontBold;
extern HFONT g_hFontBig;
extern HFONT g_hFixFont;
extern int RandomTable[];
extern bool Destroy;
extern int g_iChatInputType;
extern CUITextInputBox* g_pSingleTextInputBox;
extern CUITextInputBox* g_pSinglePasswdInputBox;
extern CUIManager* g_pUIManager;

extern GATE_ATTRIBUTE* GateAttribute;
extern SKILL_ATTRIBUTE* SkillAttribute;
extern ITEM_ATTRIBUTE* ItemAttribute;
extern CHARACTER* CharactersClient;
extern CHARACTER_MACHINE* CharacterMachine;
extern CHARACTER_ATTRIBUTE* CharacterAttribute;
extern CHARACTER* Hero;

extern unsigned int WindowWidth;
extern unsigned int WindowHeight;
extern int OpenglWindowWidth;
extern int OpenglWindowHeight;
extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

namespace platform
{
	bool InitializeOriginalGameClient(int surface_width, int surface_height)
	{
		LogInfo("InitializeOriginalGameClient: %dx%d", surface_width, surface_height);

		// Set CWD to app data root so relative paths (Data/...) resolve correctly.
		const char* app_data_root = platform::GetLegacyClientAppDataRoot();
		if (app_data_root && app_data_root[0] != '\0')
		{
			if (chdir(app_data_root) == 0)
				LogInfo("chdir to: %s", app_data_root);
			else
				LogInfo("chdir FAILED for: %s", app_data_root);
		}

		// Match game viewport to actual EGL surface dimensions.
		// The game uses a 640x480 base coordinate system scaled by g_fScreenRate.
		if (surface_width > 0 && surface_height > 0)
		{
			WindowWidth = static_cast<unsigned int>(surface_width);
			WindowHeight = static_cast<unsigned int>(surface_height);
			OpenglWindowWidth = surface_width;
			OpenglWindowHeight = surface_height;
			g_fScreenRate_x = static_cast<float>(surface_width) / 640.0f;
			g_fScreenRate_y = static_cast<float>(surface_height) / 480.0f;
			LogInfo("Viewport: WindowWidth=%u WindowHeight=%u ScreenRate=%.3f,%.3f",
				WindowWidth, WindowHeight, g_fScreenRate_x, g_fScreenRate_y);
		}

		GWidescreen.Init();
		LogInfo("GWidescreen initialized");

		Destroy = false;

		srand(static_cast<unsigned>(time(nullptr)));
		for (int i = 0; i < 100; i++)
		{
			RandomTable[i] = rand() % 360;
		}

		if (GateAttribute == nullptr)
		{
			GateAttribute = new GATE_ATTRIBUTE[MAX_GATES];
			memset(GateAttribute, 0, sizeof(GATE_ATTRIBUTE) * MAX_GATES);
			LogInfo("Allocated GateAttribute[%d]", MAX_GATES);
		}

		if (SkillAttribute == nullptr)
		{
			SkillAttribute = new SKILL_ATTRIBUTE[MAX_SKILLS];
			memset(SkillAttribute, 0, sizeof(SKILL_ATTRIBUTE) * MAX_SKILLS);
			LogInfo("Allocated SkillAttribute[%d]", MAX_SKILLS);
		}

		if (ItemAttribute == nullptr)
		{
			ItemAttribute = new ITEM_ATTRIBUTE[MAX_ITEM];
			memset(ItemAttribute, 0, sizeof(ITEM_ATTRIBUTE) * MAX_ITEM);
			LogInfo("Allocated ItemAttribute[%d]", MAX_ITEM);
		}

		if (CharactersClient == nullptr)
		{
			CharactersClient = new CHARACTER[MAX_CHARACTERS_CLIENT + 1];
			memset(CharactersClient, 0, sizeof(CHARACTER) * (MAX_CHARACTERS_CLIENT + 1));
			LogInfo("Allocated CharactersClient[%d]", MAX_CHARACTERS_CLIENT + 1);
		}

		if (CharacterMachine == nullptr)
		{
			CharacterMachine = new CHARACTER_MACHINE;
			memset(CharacterMachine, 0, sizeof(CHARACTER_MACHINE));
			CharacterAttribute = &CharacterMachine->Character;
			CharacterMachine->Init();
			LogInfo("Allocated CharacterMachine");
		}

		Hero = &CharactersClient[0];

		g_hWnd = reinterpret_cast<HWND>(1);

		// Initialize fonts — replicate logic from Winmain.cpp:1365,1530-1555
		gCreateFont.Init();
		{
			int FontHeight = static_cast<int>(std::ceil(12 + ((WindowHeight - 480) / 200.f)));
			int nFixFontHeight = WindowHeight <= 600 ? 14 : 15;
			int iFontSize = FontHeight - 1;
			int nFixFontSize = nFixFontHeight - 1;
			gCreateFont.SetFont(iFontSize, FontHeight, nFixFontSize, nFixFontHeight);
		}
		LogInfo("Fonts initialized: g_hFont=%p g_hFontBold=%p", (void*)g_hFont, (void*)g_hFontBold);

		CInput::Instance().Create(g_hWnd, static_cast<long>(surface_width), static_cast<long>(surface_height));

		SceneFlag = WEBZEN_SCENE;
		LogInfo("Set SceneFlag = WEBZEN_SCENE");

		SetTargetFps(60.0);

		CUIMng::Instance().Create();
		LogInfo("CUIMng created");

		g_pNewUISystem->Create();
		LogInfo("CNewUISystem created");

		if (g_iChatInputType == 1)
		{
			if (g_pSingleTextInputBox == NULL)
				g_pSingleTextInputBox = new CUITextInputBox;
			if (g_pSinglePasswdInputBox == NULL)
				g_pSinglePasswdInputBox = new CUITextInputBox;
			LogInfo("Allocated text input boxes");
		}

		if (g_pUIManager == NULL)
		{
			g_pUIManager = new CUIManager;
			LogInfo("Allocated CUIManager");
		}

		g_BuffSystem = BuffStateSystem::Make();
		g_MapProcess = MapProcess::Make();
		g_petProcess = PetProcess::Make();
		LogInfo("Allocated BuffSystem, MapProcess, PetProcess");

		LogInfo("InitializeOriginalGameClient complete");
		return true;
	}
}

#endif

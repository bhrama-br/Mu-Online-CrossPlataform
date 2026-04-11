#include "stdafx.h"

#if defined(__ANDROID__)

#include "Platform/LegacyClientRuntime.h"

#include "Input.h"
#include "Time/Timer.h"
#include "ZzzCharacter.h"

#include "Platform/RenderBackend.h"
#include "SimpleModulus.h"

#include <algorithm>
#include <cstring>

class CUIManager;
class CUIMapName;
class CUITextInputBox;
class CChatRoomSocketList;

// Version/Serial are defined in WSclient.cpp (now compiled on Android)
#include "WSclient.h"
extern BYTE Version[SIZE_PROTOCOLVERSION];
extern BYTE Serial[SIZE_PROTOCOLSERIAL + 1];

	namespace
	{
		char g_legacy_server_ip_storage[64] = { 0 };

		long ClampLong(long value, long minimum, long maximum)
		{
			if (value < minimum)
			{
				return minimum;
			}

			if (value > maximum)
			{
				return maximum;
			}

			return value;
		}

		long ScaleLegacyCursorAxis(int virtual_position, long input_extent, unsigned int window_extent, float screen_rate)
		{
			if (input_extent <= 0)
			{
				return static_cast<long>(virtual_position);
			}

			const float safe_screen_rate = screen_rate > 0.0f ? screen_rate : 1.0f;
			const float legacy_virtual_extent = static_cast<float>(window_extent) / safe_screen_rate;
			if (legacy_virtual_extent <= 0.0f)
			{
				return ClampLong(static_cast<long>(virtual_position), 0L, input_extent - 1);
			}

			const float scaled_position =
				(static_cast<float>(virtual_position) / legacy_virtual_extent) * static_cast<float>(input_extent);
			return ClampLong(static_cast<long>(scaled_position), 0L, input_extent - 1);
		}

		void CopyCString(char* destination, size_t destination_size, const char* source)
		{
		if (destination == NULL || destination_size == 0)
		{
			return;
		}

		const char* safe_source = source != NULL ? source : "";
		strncpy(destination, safe_source, destination_size - 1);
		destination[destination_size - 1] = '\0';
	}

	void CopyVersionDigits(const std::string& client_version)
	{
		ZeroMemory(Version, sizeof(Version));
		if (client_version.size() >= 8)
		{
			Version[0] = static_cast<BYTE>(client_version[0] + 1);
			Version[1] = static_cast<BYTE>(client_version[2] + 2);
			Version[2] = static_cast<BYTE>(client_version[3] + 3);
			Version[3] = static_cast<BYTE>(client_version[5] + 4);
			Version[4] = static_cast<BYTE>(client_version[6] + 5);
		}
	}
}

// --- CSimpleModulus stubs (Android: no-op pass-through) ---
// Originally provided by prebuilt SimpleModulus.lib on Windows.
// The stubs were in LegacyCharacterSceneLinkCompat.cpp but that file is now
// excluded by MU_ANDROID_HAS_ZZZSCENE_RUNTIME guard, so we provide them here.
DWORD CSimpleModulus::s_dwSaveLoadXOR[SIZE_ENCRYPTION_KEY] = { 0 };

CSimpleModulus::CSimpleModulus()
{
	Init();
}

CSimpleModulus::~CSimpleModulus()
{
}

void CSimpleModulus::Init(void)
{
	memset(m_dwModulus, 0, sizeof(m_dwModulus));
	memset(m_dwEncryptionKey, 0, sizeof(m_dwEncryptionKey));
	memset(m_dwDecryptionKey, 0, sizeof(m_dwDecryptionKey));
	memset(m_dwXORKey, 0, sizeof(m_dwXORKey));
}

int CSimpleModulus::Encrypt(void* lpTarget, void* lpSource, int iSize)
{
	if (lpTarget != NULL && lpSource != NULL && iSize > 0)
	{
		memcpy(lpTarget, lpSource, static_cast<size_t>(iSize));
	}
	return iSize;
}

int CSimpleModulus::Decrypt(void* lpTarget, void* lpSource, int iSize)
{
	if (lpTarget != NULL && lpSource != NULL && iSize > 0)
	{
		memcpy(lpTarget, lpSource, static_cast<size_t>(iSize));
	}
	return iSize;
}

void CSimpleModulus::EncryptBlock(void* lpTarget, void* lpSource, int nSize)
{
	(void)lpTarget; (void)lpSource; (void)nSize;
}

int CSimpleModulus::DecryptBlock(void* lpTarget, void* lpSource)
{
	(void)lpTarget; (void)lpSource;
	return 0;
}

int CSimpleModulus::AddBits(void* lpBuffer, int nNumBufferBits, void* lpBits, int nInitialBit, int nNumBits)
{
	(void)lpBuffer; (void)nNumBufferBits; (void)lpBits; (void)nInitialBit; (void)nNumBits;
	return 0;
}

void CSimpleModulus::Shift(void* lpBuffer, int nByte, int nShift)
{
	(void)lpBuffer; (void)nByte; (void)nShift;
}

int CSimpleModulus::GetByteOfBit(int nBit)
{
	return (nBit >> 3) + ((nBit & 7) ? 1 : 0);
}

BOOL CSimpleModulus::SaveAllKey(char* lpszFileName) { (void)lpszFileName; return FALSE; }
BOOL CSimpleModulus::LoadAllKey(char* lpszFileName) { (void)lpszFileName; return FALSE; }
BOOL CSimpleModulus::SaveEncryptionKey(char* lpszFileName) { (void)lpszFileName; return FALSE; }
BOOL CSimpleModulus::LoadEncryptionKey(char* lpszFileName) { (void)lpszFileName; return FALSE; }
BOOL CSimpleModulus::SaveDecryptionKey(char* lpszFileName) { (void)lpszFileName; return FALSE; }
BOOL CSimpleModulus::LoadDecryptionKey(char* lpszFileName) { (void)lpszFileName; return FALSE; }
BOOL CSimpleModulus::SaveKey(char* lpszFileName, unsigned short sID, BOOL bMod, BOOL bEnc, BOOL bDec, BOOL bXOR)
{
	(void)lpszFileName; (void)sID; (void)bMod; (void)bEnc; (void)bDec; (void)bXOR;
	return FALSE;
}
BOOL CSimpleModulus::LoadKey(char* lpszFileName, unsigned short sID, BOOL bMod, BOOL bEnc, BOOL bDec, BOOL bXOR)
{
	(void)lpszFileName; (void)sID; (void)bMod; (void)bEnc; (void)bDec; (void)bXOR;
	return FALSE;
}
BOOL CSimpleModulus::LoadKeyFromBuffer(BYTE* pbyBuffer, BOOL bMod, BOOL bEnc, BOOL bDec, BOOL bXOR)
{
	(void)pbyBuffer; (void)bMod; (void)bEnc; (void)bDec; (void)bXOR;
	return FALSE;
}

#if !defined(MU_ANDROID_HAS_ZZZOPENGLUTIL_RUNTIME)
float PerspectiveX = 640.0f;
float PerspectiveY = 480.0f;
int OpenglWindowWidth = 640;
int OpenglWindowHeight = 480;
unsigned int WindowWidth = 640;
unsigned int WindowHeight = 480;
vec3_t CollisionPosition = { 0.0f, 0.0f, 0.0f };
double FPS = 0.0;
double FPS_AVG = 0.0;
float FPS_ANIMATION_FACTOR = 1.0f;
double WorldTime = 0.0;
bool CameraTopViewEnable = false;
float CameraViewNear = 20.0f;
float CameraViewFar = 2000.0f;
float CameraFOV = 35.0f;
vec3_t CameraPosition = { 0.0f, 0.0f, 0.0f };
vec3_t CameraAngle = { 0.0f, 0.0f, 0.0f };
float CameraMatrix[3][4] = { { 0.0f } };
float g_fCameraCustomDistance = 0.0f;
bool FogEnable = false;
bool TextureEnable = true;
bool DepthTestEnable = true;
bool CullFaceEnable = false;
bool DepthMaskEnable = true;
vec3_t MousePosition = { 0.0f, 0.0f, 0.0f };
vec3_t MouseTarget = { 0.0f, 0.0f, 0.0f };
int MouseX = 0;
int MouseY = 0;
int BackMouseX = 0;
int BackMouseY = 0;
bool MouseLButton = false;
bool MouseLButtonPop = false;
bool MouseLButtonPush = false;
bool MouseRButton = false;
bool MouseRButtonPop = false;
bool MouseRButtonPush = false;
bool MouseLButtonDBClick = false;
bool MouseMButton = false;
bool MouseMButtonPop = false;
bool MouseMButtonPush = false;
int MouseWheel = 0;
DWORD MouseRButtonPress = 0;
char GrabFileName[MAX_PATH] = { 0 };
bool GrabEnable = false;
float CameraZoom = 0.0f;
float AngleX3D = 0.0f;
float AngleY3D = 0.0f;
float AngleZ3D = 0.0f;
float AngleRL = 0.0f;
bool BlockClick = false;
bool LockPlayerWalk = false;
#endif

#if !defined(MU_ANDROID_HAS_UICONTROLS_RUNTIME)
float g_fScreenRate_x = 1.0f;
float g_fScreenRate_y = 1.0f;
int g_iWidthEx = 5;
#endif

#if !defined(MU_ANDROID_HAS_ZZZSCENE_RUNTIME)
int MenuStateCurrent = MENU_SERVER_LIST;
int MenuStateNext = MENU_SERVER_LIST;
int SceneFlag = LOG_IN_SCENE;
int MoveSceneFrame = 0;
int ErrorMessage = 0;
bool InitServerList = false;
bool InitLogIn = false;
bool InitLoading = false;
bool InitCharacterScene = false;
bool InitMainScene = false;
bool EnableMainRender = false;
char* szServerIpAddress = g_legacy_server_ip_storage;
unsigned short g_ServerPort = 44405;
int g_iLengthAuthorityCode = 20;
int CreateAccount = 0;
int SelectedHero = -1;
#endif

#if !defined(MU_ANDROID_HAS_ZZZINTERFACE_RUNTIME)
int SelectedCharacter = -1;
#endif

CHARACTER* CharactersClient = NULL;
CHARACTER CharacterView = {};
CHARACTER* Hero = NULL;

#if !defined(MU_ANDROID_HAS_WINMAIN_RUNTIME)
float Time_Effect = 0.0f;
bool ashies = false;
int weather = 0;
HWND g_hWnd = reinterpret_cast<HWND>(1);
HINSTANCE g_hInst = NULL;
HDC g_hDC = NULL;
HGLRC g_hRC = NULL;
HFONT g_hFont = reinterpret_cast<HFONT>(1);
HFONT g_hFontBold = reinterpret_cast<HFONT>(1);
HFONT g_hFontBig = reinterpret_cast<HFONT>(1);
HFONT g_hFixFont = reinterpret_cast<HFONT>(1);
bool Destroy = false;
int RandomTable[100] = { 0 };
char m_ID[11] = { 0 };
char m_Version[11] = { 0 };
char m_ExeVersion[11] = { 0 };
int m_SoundOnOff = 1;
int m_MusicOnOff = 1;
int m_Resolution = 0;
int m_nColorDepth = 1;
int g_iRenderTextType = 0;
int m_CameraOnOff = 0;
int g_iChatInputType = 1;
char g_aszMLSelection[MAX_LANGUAGE_NAME_LENGTH] = "Eng";
std::string g_strSelectedML = "Eng";
BOOL g_bUseWindowMode = TRUE;
bool g_bWndActive = true;
CUIManager* g_pUIManager = NULL;
CUIMapName* g_pUIMapName = NULL;
CTimer* g_pTimer = new CTimer();
CUITextInputBox* g_pSinglePasswdInputBox = NULL;
CUITextInputBox* g_pSingleTextInputBox = NULL;
CChatRoomSocketList* g_pChatRoomSocketList = NULL;

// --- Winmain.cpp function stubs (Android does not compile Winmain.cpp) ---
bool g_bEnterPressed = false;
int g_iNoMouseTime = 0;

void CheckHack(void)
{
}

DWORD GetCheckSum(WORD wKey)
{
	(void)wKey;
	return 0;
}

void CloseMainExe(void)
{
}

GLvoid KillGLWindow(GLvoid)
{
	platform::ShutdownRenderBackend();
}

// Winmain.h declares 'extern void DestroyWindow();' (no-arg variant)
// This is separate from the Win32 API DestroyWindow(HWND) stubbed in Win32SecondaryStubs.h
void DestroyWindow()
{
}

#endif

namespace platform
{
	void InitializeLegacyClientRuntime()
	{
		static const int kRandomTableSize = 100;
		for (int index = 0; index < kRandomTableSize; ++index)
		{
			RandomTable[index] = (index * 37) % 360;
		}

		SetLegacyClientSceneFlag(LOG_IN_SCENE);
		g_bWndActive = true;
		g_hWnd = reinterpret_cast<HWND>(1);
		CInput::Instance().Create(g_hWnd, static_cast<long>(WindowWidth), static_cast<long>(WindowHeight));
	}

	void ApplyLegacyClientRuntimeConfig(const ClientRuntimeConfigState* config)
	{
		if (config == NULL)
		{
			return;
		}

		WindowWidth = config->window_width > 0 ? config->window_width : WindowWidth;
		WindowHeight = config->window_height > 0 ? config->window_height : WindowHeight;
		OpenglWindowWidth = static_cast<int>(WindowWidth);
		OpenglWindowHeight = static_cast<int>(WindowHeight);
		g_fScreenRate_x = config->screen_rate_x > 0.0f ? config->screen_rate_x : 1.0f;
		g_fScreenRate_y = config->screen_rate_y > 0.0f ? config->screen_rate_y : 1.0f;
		m_SoundOnOff = config->sound_on;
		m_MusicOnOff = config->music_on;
		m_Resolution = config->resolution;
		m_nColorDepth = config->color_depth;
		g_iRenderTextType = config->render_text_type;
		g_iChatInputType = config->chat_input_type;
		g_bUseWindowMode = config->window_mode ? TRUE : FALSE;
		CInput::Instance().AndroidConfigure(g_hWnd, static_cast<long>(WindowWidth), static_cast<long>(WindowHeight));

		CopyCString(m_ID, 11, config->player_id.c_str());
		CopyCString(g_aszMLSelection, sizeof(g_aszMLSelection), config->language.c_str());
		g_strSelectedML = g_aszMLSelection;
	}

	void ApplyLegacyClientIniConfig(const ClientIniConfigState* config)
	{
		if (config == NULL)
		{
			return;
		}

		CopyCString(m_Version, sizeof(m_Version), config->login_version.c_str());
		if (m_ExeVersion[0] == '\0')
		{
			CopyCString(m_ExeVersion, sizeof(m_ExeVersion), config->login_version.c_str());
		}
	}

	void ApplyLegacyClientCoreBootstrap(const CoreGameBootstrapState* state)
	{
		if (state == NULL)
		{
			return;
		}

		CopyCString(g_legacy_server_ip_storage, sizeof(g_legacy_server_ip_storage), state->ip_address.c_str());
		szServerIpAddress = g_legacy_server_ip_storage;
		g_ServerPort = state->ip_address_port;

		if (!state->client_version.empty())
		{
			CopyVersionDigits(state->client_version);
			if (m_Version[0] == '\0')
			{
				CopyCString(m_Version, sizeof(m_Version), state->client_version.c_str());
			}
			if (m_ExeVersion[0] == '\0')
			{
				CopyCString(m_ExeVersion, sizeof(m_ExeVersion), state->client_version.c_str());
			}
		}

		ZeroMemory(Serial, sizeof(Serial));
		const size_t serial_copy_size = (std::min)(state->client_serial.size(), sizeof(Serial) - 1);
		memcpy(Serial, state->client_serial.c_str(), serial_copy_size);
	}

	void SyncLegacyClientMouseState(const GameMouseState* mouse_state)
	{
		if (mouse_state == NULL)
		{
			return;
		}

		MouseX = mouse_state->x;
		MouseY = mouse_state->y;
		BackMouseX = mouse_state->back_x;
		BackMouseY = mouse_state->back_y;
		MouseLButton = mouse_state->left_button;
		MouseLButtonPop = mouse_state->left_button_pop;
		MouseLButtonPush = mouse_state->left_button_push;
		MouseRButton = mouse_state->right_button;
		MouseRButtonPop = mouse_state->right_button_pop;
		MouseRButtonPush = mouse_state->right_button_push;
		MouseLButtonDBClick = mouse_state->left_button_double_click;
		MouseMButton = mouse_state->middle_button;
		MouseMButtonPop = mouse_state->middle_button_pop;
		MouseMButtonPush = mouse_state->middle_button_push;
		MouseWheel = mouse_state->wheel;

		const long input_width = CInput::Instance().GetScreenWidth();
		const long input_height = CInput::Instance().GetScreenHeight();
		POINT cursor = {
			static_cast<LONG>(ScaleLegacyCursorAxis(mouse_state->x, input_width, WindowWidth, g_fScreenRate_x)),
			static_cast<LONG>(ScaleLegacyCursorAxis(mouse_state->y, input_height, WindowHeight, g_fScreenRate_y))
		};
		const long scaled_dx = static_cast<long>(cursor.x) - CInput::Instance().GetCursorX();
		const long scaled_dy = static_cast<long>(cursor.y) - CInput::Instance().GetCursorY();
		CInput::Instance().AndroidSyncMouseState(
			cursor,
			scaled_dx,
			scaled_dy,
			mouse_state->left_button_push,
			mouse_state->left_button,
			mouse_state->left_button_pop,
			mouse_state->left_button_double_click,
			mouse_state->right_button_push,
			mouse_state->right_button,
			mouse_state->right_button_pop,
			false,
			mouse_state->middle_button_push,
			mouse_state->middle_button,
			mouse_state->middle_button_pop,
			false);
	}

	void AdvanceLegacyClientMouseState(GameMouseState* mouse_state)
	{
		if (mouse_state == NULL)
		{
			return;
		}

		mouse_state->back_x = mouse_state->x;
		mouse_state->back_y = mouse_state->y;
		mouse_state->left_button_push = false;
		mouse_state->left_button_pop = false;
		mouse_state->right_button_push = false;
		mouse_state->right_button_pop = false;
		mouse_state->middle_button_push = false;
		mouse_state->middle_button_pop = false;
		mouse_state->left_button_double_click = false;
		mouse_state->wheel = 0;

		const long input_width = CInput::Instance().GetScreenWidth();
		const long input_height = CInput::Instance().GetScreenHeight();
		POINT cursor = {
			static_cast<LONG>(ScaleLegacyCursorAxis(mouse_state->x, input_width, WindowWidth, g_fScreenRate_x)),
			static_cast<LONG>(ScaleLegacyCursorAxis(mouse_state->y, input_height, WindowHeight, g_fScreenRate_y))
		};
		CInput::Instance().AndroidSyncMouseState(
			cursor,
			0L,
			0L,
			false,
			mouse_state->left_button,
			false,
			false,
			false,
			mouse_state->right_button,
			false,
			false,
			false,
			mouse_state->middle_button,
			false,
			false);
	}

	void SetLegacyClientSceneFlag(int scene_flag)
	{
		SceneFlag = scene_flag;
	}
}

#if !defined(MU_ANDROID_HAS_ZZZOPENGLUTIL_RUNTIME)
void EnableAlphaTest(bool DepthMake)
{
	platform::RenderBackend& render_backend = platform::GetRenderBackend();
	render_backend.SetTextureEnabled(true);
	render_backend.SetAlphaTestEnabled(true);
	render_backend.SetAlphaFunction(GL_GREATER, 0.25f);
	render_backend.SetBlendState(false, GL_ONE, GL_ZERO);
	render_backend.SetCurrentColor(1.0f, 1.0f, 1.0f, 1.0f);
	render_backend.SetDepthMaskEnabled(true);
	render_backend.SetDepthTestEnabled(DepthMake);
}
#endif

#endif

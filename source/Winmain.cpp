///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Platform/GameClientConfig.h"
#include "Platform/GameConfigBootstrap.h"

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include <dpapi.h>
#include <locale.h>
#include <zmouse.h>
#include <cmath>
#include "UIWindows.h"
#include "UIManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzOpenData.h"
#include "ZzzScene.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "zzzLodTerrain.h"
#include "DSPlaySound.h"
#include "wsclientinline.h"
#include "Resource.h"
#include <imm.h>
#include "zzzpath.h"
#include "Nprotect.h"
#include "Local.h"
#include "PersonalShopTitleImp.h"
#include "./Utilities/Log/ErrorReport.h"
#include "UIMapName.h"		// rozy
#include "./ExternalObject/leaf/ExceptionHandler.h"
#include "./Utilities/Dump/CrashReporter.h"
#include "./Utilities/Log/muConsoleDebug.h"
//#include "./Utilities/CpuUsage.h"
#include "ProtocolSend.h"
#include "ProtectSysKey.h"

#include "CBTMessageBox.h"
#include "./ExternalObject/leaf/regkey.h"

#include "CSChaosCastle.h"
#include "GMHellas.h"
#include <io.h>
#include "Input.h"
#include "./Time/Timer.h"
#include "UIMng.h"

#ifdef MOVIE_DIRECTSHOW
#include <dshow.h>
#include "MovieScene.h"
#endif // MOVIE_DIRECTSHOW
#include "GameCensorship.h"

#include "w_MapHeaders.h"

#include "w_PetProcess.h"

#include <ThemidaInclude.h>
#include "ItemPosition.h"
#include "ItemSize.h"
#include "Descriptions.h"
#include "MultiLanguage.h"
#include "Widescreen.h"
#include "Console.h"
#include "Visuals.h"
#include "DarkSpirit.h"
#include "DisableExcellent.h"
#include "CustomBow.h"
#include "CustomWing.h"
#include "CustomEffects.h"
#include "CustomSetEffect.h"
#include "CustomItemFloor.h"
#include "CustomItemForce.h"
#include "RenderModel.h"
#include "CustomJewel.h"
#include "ServerName.h"
#include "CreateFont.h"
#include "MessageColor.h"
#include "Controller.h"
#include "Monsters.h"
#include "MonsterName.h"
#include "MonsterGlow.h"
#include "MonsterEffect.h"
#include "Platform/GameAssetPath.h"
#include "Platform/GameLanguageAssetBootstrap.h"
#include "Platform/GameMouseInput.h"
#include "Platform/GameClientRuntimeConfig.h"
#include "Platform/RenderBackend.h"
#include "Platform/Win32PlatformWindow.h"
#include "CustomCape.h"
#include "CounterItem.h"
#include "CharacterList.h"
#include "Patente.h"
#include "ElementSlots.h"
#include "TradeX.h"
#include "CreateFont.h"
#ifdef NEW_MUHELPER_ON
#include "CAIController.h"
#endif
#include "WindowTray.h"

CUIMercenaryInputBox * g_pMercenaryInputBox = NULL;
CUITextInputBox * g_pSingleTextInputBox = NULL;
CUITextInputBox * g_pSinglePasswdInputBox = NULL;
int g_iChatInputType = 1;
extern BOOL g_bIMEBlock;

CChatRoomSocketList * g_pChatRoomSocketList = NULL;

CMultiLanguage *pMultiLanguage = NULL;

extern DWORD g_dwTopWindow;

#ifdef MOVIE_DIRECTSHOW
CMovieScene*	g_pMovieScene = NULL;
#endif // MOVIE_DIRECTSHOW

CUIManager* g_pUIManager = NULL;
CUIMapName* g_pUIMapName = NULL;		// rozy

namespace
{
	platform::Win32PlatformWindow g_MainPlatformWindow;
}

float Time_Effect = 0;
bool ashies = false;
int weather = rand()%3;

HWND      g_hWnd  = NULL;
HINSTANCE g_hInst = NULL;
HDC       g_hDC   = NULL;
HGLRC     g_hRC   = NULL;
HFONT     g_hFont = NULL;
HFONT     g_hFontBold = NULL;
HFONT     g_hFontBig = NULL;
HFONT     g_hFixFont = NULL;
HFONT     g_hNewFonts[10];

CTimer* g_pTimer = new CTimer();    // performance counter.
bool      Destroy = false;
bool      ActiveIME = false;

BYTE*				RendomMemoryDump;
ITEM_ATTRIBUTE*		ItemAttRibuteMemoryDump;
CHARACTER*			CharacterMemoryDump;

int       RandomTable[100];

char TextMu[]       = "mu.exe";

CErrorReport g_ErrorReport;

BOOL g_bMinimizedEnabled = FALSE;
int g_iScreenSaverOldValue = 60*15;

extern float g_fScreenRate_x;	// ※
extern float g_fScreenRate_y;

#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
BOOL g_bUseWindowMode = TRUE;
#endif

char Mp3FileName[256];

#pragma comment(lib, "wzAudio.lib")
#include <wzAudio.h>

void StopMp3(char *Name, BOOL bEnforce)
{
    if(!m_MusicOnOff && !bEnforce) return;

	if(Mp3FileName[0] != NULL)
	{
		if(strcmp(Name, Mp3FileName) == 0) {
			wzAudioStop();
			Mp3FileName[0] = NULL;
		}
	}
}

void PlayMp3(char *Name, BOOL bEnforce )
{
	if(Destroy) return;
    if(!m_MusicOnOff && !bEnforce) return;

	if(strcmp(Name,Mp3FileName) == 0) 
	{
		return;
	}
	else 
	{
		wzAudioPlay(Name, 1);
		strcpy(Mp3FileName,Name);
	}
}

bool IsEndMp3()
{
	if (100 == wzAudioGetStreamOffsetRange())
		return true;
	return false;
}

int GetMp3PlayPosition()
{
	return wzAudioGetStreamOffsetRange();
}

extern int  LogIn;
extern char LogInID[];

void CheckHack( void)
{
	#ifdef NEW_PROTOCOL_SYSTEM
		gProtocolSend.SendCheckOnline();
	#else
		SendCheck();
	#endif
}

GLvoid KillGLWindow(GLvoid)								
{
	platform::ShutdownRenderBackend();
	const platform::GlContextResult result = g_MainPlatformWindow.DestroyLegacyOpenGLContext();
	g_hDC = g_MainPlatformWindow.GetDeviceContext();
	g_hRC = g_MainPlatformWindow.GetRenderContext();

	if (!result.ok)
	{
		g_ErrorReport.Write("GL - %s\r\n", result.step);
		MessageBox(NULL, result.step, "Error", MB_OK | MB_ICONINFORMATION);
	}

#if (defined WINDOWMODE)
	if (g_bUseWindowMode == FALSE)
	{
		ChangeDisplaySettings(NULL,0);
		ShowCursor(TRUE);
	}
#else
#ifdef ENABLE_FULLSCREEN
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
	if (g_bUseWindowMode == FALSE)
#endif	// USER_WINDOW_MODE
	{
		ChangeDisplaySettings(NULL,0);
		ShowCursor(TRUE);
	}
#endif //ENABLE_FULLSCREEN
#endif	//WINDOWMODE(#else)
}


BOOL GetFileNameOfFilePath( char *lpszFile, char *lpszPath)
{
	int iFind = ( int)'\\';
	char *lpFound = lpszPath;
	char *lpOld = lpFound;
	while ( lpFound)
	{
		lpOld = lpFound;
		lpFound = strchr( lpFound + 1, iFind);
	}

	if ( strchr( lpszPath, iFind))
	{
		strcpy( lpszFile, lpOld + 1);
	}
	else
	{
		strcpy( lpszFile, lpOld);
	}

	BOOL bCheck = TRUE;
	for ( char *lpTemp = lpszFile; bCheck; ++lpTemp)
	{
		switch ( *lpTemp)
		{
		case '\"':
		case '\\':
		case '/':
		case ' ':
			*lpTemp = '\0';
		case '\0':
			bCheck = FALSE;
			break;
		}
	}

	return ( TRUE);
}

HANDLE g_hMainExe = INVALID_HANDLE_VALUE;

BOOL OpenMainExe( void)
{
#ifdef _DEBUG
	return ( TRUE);
#endif
	char lpszFile[MAX_PATH];
	char *lpszCommandLine = GetCommandLine();
	GetFileNameOfFilePath( lpszFile, lpszCommandLine);

	g_hMainExe = CreateFile( ( char*)lpszFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, 0);
	
	return ( INVALID_HANDLE_VALUE != g_hMainExe);
}

void CloseMainExe( void)
{
	CloseHandle( g_hMainExe);
}

WORD DecryptCheckSumKey( WORD wSource)
{
	WORD wAcc = wSource ^ 0xB479;
	return ( ( wAcc >> 10) << 4) | ( wAcc & 0xF);
}

DWORD GenerateCheckSum( BYTE *pbyBuffer, DWORD dwSize, WORD wKey)
{
	DWORD dwKey = ( DWORD)wKey;
	DWORD dwResult = dwKey << 9;
	for ( DWORD dwChecked = 0; dwChecked <= dwSize - 4; dwChecked += 4)
	{
		DWORD dwTemp;
		memcpy( &dwTemp, pbyBuffer + dwChecked, sizeof ( DWORD));

		switch ( ( dwChecked / 4 + wKey) % 3)
		{
		case 0:
			dwResult ^= dwTemp;
			break;
		case 1:
			dwResult += dwTemp;
			break;
		case 2:
			dwResult <<= ( dwTemp % 11);
			dwResult ^= dwTemp;
			break;
		}

		if ( 0 == ( dwChecked % 4))
		{
			dwResult ^= ( ( dwKey + dwResult) >> ( ( dwChecked / 4) % 16 + 3));
		}
	}

	return ( dwResult);
}

DWORD GetCheckSum( WORD wKey)
{
	wKey = DecryptCheckSumKey( wKey);

	char lpszFile[MAX_PATH];

	strcpy( lpszFile, "data\\local\\Gameguard.csr");

	HANDLE hFile = CreateFile( ( char*)lpszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if ( INVALID_HANDLE_VALUE == hFile)
	{
		return ( 0);
	}
	
	DWORD dwSize = GetFileSize( hFile, NULL);
	BYTE *pbyBuffer = new BYTE [dwSize];
	DWORD dwNumber;
	ReadFile( hFile, pbyBuffer, dwSize, &dwNumber, 0);
	CloseHandle( hFile);
	
	DWORD dwCheckSum = GenerateCheckSum(pbyBuffer, dwSize, wKey);
	delete [] pbyBuffer;
	
	return (dwCheckSum);
}


BOOL GetFileVersion( char *lpszFileName, WORD *pwVersion)
{
	DWORD dwHandle;
	DWORD dwLen = GetFileVersionInfoSize( lpszFileName, &dwHandle);
	if ( dwLen <= 0)
	{
		return ( FALSE);
	}

	BYTE *pbyData = new BYTE [dwLen];
	if ( !GetFileVersionInfo( lpszFileName, dwHandle, dwLen, pbyData))
	{
		delete [] pbyData;
		return ( FALSE);
	}

	VS_FIXEDFILEINFO *pffi;
	UINT uLen;
	if ( !VerQueryValue( pbyData, "\\", ( LPVOID*)&pffi, &uLen))
	{
		delete [] pbyData;
		return ( FALSE);
	}

	pwVersion[0] = HIWORD( pffi->dwFileVersionMS);
	pwVersion[1] = LOWORD( pffi->dwFileVersionMS);
	pwVersion[2] = HIWORD( pffi->dwFileVersionLS);
	pwVersion[3] = LOWORD( pffi->dwFileVersionLS);

	delete [] pbyData;
	return ( TRUE);
}

extern PATH     *path;

void DestroyWindow()
{
	//. save volume level
	leaf::CRegKey regkey;
	regkey.SetKey(leaf::CRegKey::_HKEY_CURRENT_USER, "SOFTWARE\\Webzen\\Mu\\Config");
	regkey.WriteDword("VolumeLevel", g_pOption->GetVolumeLevel());

	CUIMng::Instance().Release();

#ifdef MOVIE_DIRECTSHOW
	if(g_pMovieScene)
	{
		g_pMovieScene->Destroy();
	}
#endif // MOVIE_DIRECTSHOW

	//. release font handle
	if(g_hFont)
		DeleteObject((HGDIOBJ)g_hFont);

	if(g_hFontBold)
		DeleteObject((HGDIOBJ)g_hFontBold);

	if(g_hFontBig)
		DeleteObject((HGDIOBJ)g_hFontBig);

	if(g_hFixFont)
		::DeleteObject((HGDIOBJ)g_hFixFont);
	
	ReleaseCharacters();

    if ( path!=NULL )
    {
	    delete path;
    }
	SAFE_DELETE(GateAttribute);

	for ( int i = 0; i < MAX_SKILLS; ++i)
	{
	}
	SAFE_DELETE(SkillAttribute);

	SAFE_DELETE(CharacterMachine);

    DeleteWaterTerrain ();

#ifdef MOVIE_DIRECTSHOW
	if(SceneFlag != MOVIE_SCENE)
#endif // MOVIE_DIRECTSHOW
	{
		gMapManager.DeleteObjects();

		// Object.
		for(int i=MODEL_LOGO;i<MAX_MODELS;i++)
		{
			Models[i].Release();
		}

		// Bitmap
		Bitmaps.UnloadAllImages();
	}

	SAFE_DELETE_ARRAY( CharacterMemoryDump );
	SAFE_DELETE_ARRAY( ItemAttRibuteMemoryDump );
	SAFE_DELETE_ARRAY( RendomMemoryDump );
	SAFE_DELETE_ARRAY( ModelsDump );
	
#ifdef DYNAMIC_FRUSTRUM
	DeleteAllFrustrum();
#endif //DYNAMIC_FRUSTRUM

	SAFE_DELETE(g_pMercenaryInputBox);
	SAFE_DELETE(g_pSingleTextInputBox);
	SAFE_DELETE(g_pSinglePasswdInputBox);

	SAFE_DELETE(g_pChatRoomSocketList);
	SAFE_DELETE(g_pUIMapName);	// rozy
	SAFE_DELETE( g_pTimer );
	SAFE_DELETE(g_pUIManager);
	 
#ifdef MOVIE_DIRECTSHOW
	SAFE_DELETE(g_pMovieScene);
#endif // MOVIE_DIRECTSHOW

	SAFE_DELETE(pMultiLanguage);
	BoostRest( g_BuffSystem );
	BoostRest( g_MapProcess );
	BoostRest( g_petProcess );

	g_ErrorReport.Write( "Destroy" );
	 
	HWND shWnd = FindWindow(NULL, "MuPlayer");
	if(shWnd)
		SendMessage(shWnd, WM_DESTROY, 0, 0);
}
void DestroySound()
{
	for(int i=0;i<MAX_BUFFER;i++)
		ReleaseBuffer(i);

	FreeDirectSound();
	wzAudioDestroy();
}

int g_iInactiveTime = 0;
int g_iNoMouseTime = 0;
int g_iInactiveWarning = 0;
bool g_bWndActive = false;
bool HangulDelete = false;
int Hangul = 0;
bool g_bEnterPressed = false;

int g_iMousePopPosition_x = 0;
int g_iMousePopPosition_y = 0;

namespace
{
	platform::GameMouseState ReadGlobalMouseState()
	{
		platform::GameMouseState state = {};
		state.x = MouseX;
		state.y = MouseY;
		state.back_x = BackMouseX;
		state.back_y = BackMouseY;
		state.left_button = MouseLButton;
		state.left_button_pop = MouseLButtonPop;
		state.left_button_push = MouseLButtonPush;
		state.right_button = MouseRButton;
		state.right_button_pop = MouseRButtonPop;
		state.right_button_push = MouseRButtonPush;
		state.left_button_double_click = MouseLButtonDBClick;
		state.middle_button = MouseMButton;
		state.middle_button_pop = MouseMButtonPop;
		state.middle_button_push = MouseMButtonPush;
		state.wheel = MouseWheel;
		state.left_pop_x = g_iMousePopPosition_x;
		state.left_pop_y = g_iMousePopPosition_y;
		return state;
	}

	void WriteGlobalMouseState(const platform::GameMouseState& state)
	{
		MouseX = state.x;
		MouseY = state.y;
		BackMouseX = state.back_x;
		BackMouseY = state.back_y;
		MouseLButton = state.left_button;
		MouseLButtonPop = state.left_button_pop;
		MouseLButtonPush = state.left_button_push;
		MouseRButton = state.right_button;
		MouseRButtonPop = state.right_button_pop;
		MouseRButtonPush = state.right_button_push;
		MouseLButtonDBClick = state.left_button_double_click;
		MouseMButton = state.middle_button;
		MouseMButtonPop = state.middle_button_pop;
		MouseMButtonPush = state.middle_button_push;
		MouseWheel = state.wheel;
		g_iMousePopPosition_x = state.left_pop_x;
		g_iMousePopPosition_y = state.left_pop_y;
	}

	platform::GameMouseMetrics CreateCurrentWindowMouseMetrics()
	{
		return platform::CreateGameMouseMetrics(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight), g_fScreenRate_y);
	}
}

extern int TimeRemain;
extern bool EnableFastInput;
void MainScene(HDC hDC);

LONG FAR PASCAL WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch (msg)
    {
	case TRAYMODE_ICON_MESSAGE:
	{
		switch (lParam)
		{
		case WM_LBUTTONDBLCLK:
			gTrayMode.SwitchState();
			break;
		}
	}
	case WM_SYSKEYDOWN:
		{
			return 0;
		}
		break;
#if defined PROTECT_SYSTEMKEY && defined NDEBUG
#ifndef FOR_WORK
	case WM_SYSCOMMAND:
		{
			if(wParam == SC_KEYMENU || wParam == SC_SCREENSAVE)
			{
				return 0;
			}
		}
		break;
#endif // !FOR_WORK
#endif // PROTECT_SYSTEMKEY && NDEBUG
    case WM_ACTIVATE:
		if(LOWORD(wParam) == WA_INACTIVE)
		{
#ifdef ACTIVE_FOCUS_OUT
			if (g_bUseWindowMode == FALSE)
#endif	// ACTIVE_FOCUS_OUT
				g_bWndActive = false;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
			if (g_bUseWindowMode == TRUE)
			{
				platform::GameMouseState mouse_state = ReadGlobalMouseState();
				platform::ApplyInactiveWindowMouseState(&mouse_state);
				WriteGlobalMouseState(mouse_state);
			}
#endif
		}
		else
		{
			g_bWndActive = true;
		}
		break;
	case WM_TIMER :
		//MessageBox(NULL,GlobalText[16],"Error",MB_OK);
		switch( wParam )
		{
		case HACK_TIMER:
			// PKD_ADD_BINARY_PROTECTION
			VM_START
			CheckHack();
			VM_END
			break;
		case WINDOWMINIMIZED_TIMER:
			PostMessage(g_hWnd, WM_CLOSE, 0, 0);
			break;
		case CHATCONNECT_TIMER:
			g_pFriendMenu->SendChatRoomConnectCheck();
			break;
		case SLIDEHELP_TIMER:
			if(g_bWndActive)
			{
				if(g_pSlideHelpMgr)
					g_pSlideHelpMgr->CreateSlideText();
			}
			break;
		}
		break;
	case WM_USER_MEMORYHACK:
		//SetTimer( g_hWnd, WINDOWMINIMIZED_TIMER, 1*1000, NULL);
		KillGLWindow();
		break;
	case WM_NPROTECT_EXIT_TWO:
		SendHackingChecked( 0x04, 0);
		SetTimer( g_hWnd, WINDOWMINIMIZED_TIMER, 1*1000, NULL);
		MessageBox(NULL,GlobalText[16],"Error",MB_OK);
		break;
	case WM_ASYNCSELECTMSG :
		switch( WSAGETSELECTEVENT( lParam ) )
		{
		case FD_CONNECT :
			break;
		case FD_READ :
			SocketClient.nRecv();
			break;
		case FD_WRITE :
			SocketClient.FDWriteSend();
			break;
		case FD_CLOSE :
			g_pChatListBox->AddText("", GlobalText[3], SEASON3B::TYPE_SYSTEM_MESSAGE);
#ifdef CONSOLE_DEBUG
			switch(WSAGETSELECTERROR(lParam))
			{
			case WSAECONNRESET:
				g_ConsoleDebug->Write(MCD_ERROR, "The connection was reset by the remote side.");
				g_ErrorReport.Write("The connection was reset by the remote side.\r\n");
				g_ErrorReport.WriteCurrentTime();
				break;
			case WSAECONNABORTED:
				g_ConsoleDebug->Write(MCD_ERROR, "The connection was terminated due to a time-out or other failure.");
				g_ErrorReport.Write("The connection was terminated due to a time-out or other failure.\r\n");
				g_ErrorReport.WriteCurrentTime();
				break;
			}
#endif // CONSOLE_DEBUG
			SocketClient.Close();

			#ifdef NEW_PROTOCOL_SYSTEM
				gProtocolSend.DisconnectServer();
			#endif	

			CUIMng::Instance().PopUpMsgWin(MESSAGE_SERVER_LOST);
			break;
		}
		break;
	case WM_CTLCOLOREDIT:
		SetBkColor((HDC)wParam, RGB(0, 0, 0));
		SetTextColor((HDC)wParam, RGB(255, 255, 255));
		return (LRESULT)GetStockObject(BLACK_BRUSH);
		break;
	case WM_ERASEBKGND:
		return TRUE;
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps ;
			HDC hDC = BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps) ;
		}
		return 0;
		break;
	case WM_DESTROY:
		{
			Destroy = true;
			SocketClient.Close();

			#ifdef NEW_PROTOCOL_SYSTEM
				gProtocolSend.DisconnectServer();
			#endif	

			DestroySound();
			//DestroyWindow();
			KillGLWindow();	
			CloseMainExe();
			PostQuitMessage(0);
		}
		break;
    case WM_SETCURSOR:
        ShowCursor(false);
		break;
#if (defined WINDOWMODE)
	case WM_SIZE:
		if ( SIZE_MINIMIZED == wParam && g_bUseWindowMode == FALSE )
		{
			if ( !( g_bMinimizedEnabled))
			{
				DWORD dwMess[SIZE_ENCRYPTION_KEY];
				for ( int i = 0; i < SIZE_ENCRYPTION_KEY; ++i)
				{
					dwMess[i] = GetTickCount();
				}
				g_SimpleModulusCS.LoadKeyFromBuffer( ( BYTE*)dwMess, FALSE, FALSE, FALSE, TRUE);
			}
		}
		break;
#else
#ifdef NDEBUG
#ifndef FOR_WORK
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
	case WM_SIZE:
		if ( SIZE_MINIMIZED == wParam
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
			&& g_bUseWindowMode == FALSE
#endif
			)
		{
			if ( !( g_bMinimizedEnabled))
			{
				SendHackingChecked( 0x05, 0);
				DWORD dwMess[SIZE_ENCRYPTION_KEY];
				for ( int i = 0; i < SIZE_ENCRYPTION_KEY; ++i)
				{
					dwMess[i] = GetTickCount();
				}
				g_SimpleModulusCS.LoadKeyFromBuffer( ( BYTE*)dwMess, FALSE, FALSE, FALSE, TRUE);
			}
		}
		break;
#endif
#endif
#endif
#endif	//WINDOWMODE(#else)
//-----------------------------
	default:
		if (msg >= WM_CHATROOMMSG_BEGIN && msg < WM_CHATROOMMSG_END)
			g_pChatRoomSocketList->ProcessSocketMessage(msg - WM_CHATROOMMSG_BEGIN, WSAGETSELECTEVENT(lParam));
		break;
	}

	platform::GameMouseState mouse_state = ReadGlobalMouseState();
	platform::PrepareGameMouseStateForMessage(&mouse_state);
    switch (msg)
	{
	case WM_MOUSEMOVE:
		{
			platform::UpdateGameMousePositionFromWindowPixels(&mouse_state, CreateCurrentWindowMouseMetrics(), static_cast<float>(LOWORD(lParam)), static_cast<float>(HIWORD(lParam)));
		}
		break;
	case WM_LBUTTONDOWN:
		g_iNoMouseTime = 0;
		platform::SetGameMouseLeftButtonDown(&mouse_state);
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
		SetCapture(g_hWnd);
#endif
		break;
	case WM_LBUTTONUP:
		g_iNoMouseTime = 0;
		platform::SetGameMouseLeftButtonUp(&mouse_state);
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
		ReleaseCapture();
#endif
		break;
	case WM_RBUTTONDOWN:
		g_iNoMouseTime = 0;
		platform::SetGameMouseRightButtonDown(&mouse_state);
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
		SetCapture(g_hWnd);
#endif
		break;
	case WM_RBUTTONUP:
		g_iNoMouseTime = 0;
		platform::SetGameMouseRightButtonUp(&mouse_state);
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
		ReleaseCapture();
#endif
		break;
    case WM_LBUTTONDBLCLK:
		g_iNoMouseTime = 0;
		platform::SetGameMouseLeftDoubleClick(&mouse_state);
		break;
	case WM_MBUTTONDOWN:
		g_iNoMouseTime = 0;
		platform::SetGameMouseMiddleButtonDown(&mouse_state);
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
		SetCapture(g_hWnd);
#endif
		break;
	case WM_MBUTTONUP:
		g_iNoMouseTime = 0;
		platform::SetGameMouseMiddleButtonUp(&mouse_state);
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
		ReleaseCapture();
#endif
		break;
	case WM_MOUSEWHEEL:
        {
			platform::SetGameMouseWheelDelta(&mouse_state, static_cast<short>(HIWORD(wParam)) / WHEEL_DELTA);
        }
        break;
	case WM_IME_NOTIFY:
		{
			if (g_iChatInputType == 1)
			{
				switch (wParam)
				{
				case IMN_SETCONVERSIONMODE:
					if (GetFocus() == g_hWnd)
					{
						CheckTextInputBoxIME(IME_CONVERSIONMODE);
					}
					break;
				case IMN_SETSENTENCEMODE:
					if (GetFocus() == g_hWnd)
					{
						CheckTextInputBoxIME(IME_SENTENCEMODE);
					}
					break;
				default:
					break;
				}
			}
		}
		break;
	case WM_CHAR:
		{
			switch(wParam)
			{
			case VK_RETURN:
				{
					SetEnterPressed( true );
				}
				break;
			}
		}
		break;
    }

	WriteGlobalMouseState(mouse_state);

	if( g_BuffSystem ) {
		LRESULT result;
		TheBuffStateSystem().HandleWindowMessage( msg, wParam, lParam, result );
	}

    return DefWindowProc(hwnd,msg,wParam,lParam);
}

bool CreateOpenglWindow()
{
	const platform::GlContextResult result = g_MainPlatformWindow.CreateLegacyOpenGLContext();
	g_hDC = g_MainPlatformWindow.GetDeviceContext();
	g_hRC = g_MainPlatformWindow.GetRenderContext();

	if (!result.ok)
	{
		g_ErrorReport.Write("%s - ErrorCode : %d\r\n", result.step, result.error_code);
		KillGLWindow();
		MessageBox(NULL, GlobalText[4], result.step, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	platform::InitializeRenderBackend();
	g_ErrorReport.Write("RenderBackend - %s\r\n", platform::GetRenderBackendTypeName(platform::GetActiveRenderBackendType()));
	g_MainPlatformWindow.ShowAndFocus(SW_SHOW);
	return true;
}

HWND StartWindow(HINSTANCE hCurrentInst, int nCmdShow)
{
	(void)nCmdShow;

	platform::WindowConfig config;
	config.instance = hCurrentInst;
	config.wnd_proc = WndProc;
	config.window_name = gProtect->m_MainInfo.WindowName;
	config.width = WindowWidth;
	config.height = WindowHeight;
	config.windowed = (g_bUseWindowMode == TRUE);
	config.icon_resource_id = IDI_ICON1;

	const platform::WindowCreationResult result = g_MainPlatformWindow.Create(config);
	if (!result.ok)
	{
		g_ErrorReport.Write("%s - ErrorCode : %d\r\n", result.step, result.error_code);
		MessageBox(NULL, GlobalText[4], result.step, MB_OK | MB_ICONEXCLAMATION);
		return NULL;
	}

	return g_MainPlatformWindow.GetHandle();
}

char m_ID[11];
char m_Version[11];
char m_ExeVersion[11];
int  m_SoundOnOff;
int  m_MusicOnOff;
int  m_Resolution;
int	m_nColorDepth;
int	g_iRenderTextType = 0;
int	m_CameraOnOff;

char g_aszMLSelection[MAX_LANGUAGE_NAME_LENGTH] = {'\0'};
std::string g_strSelectedML = "";

BOOL OpenInitFile()
{
	gController.Load();

	//char szTemp[50];
	char szIniFilePath[256+20]="";
	char szCurrentDir[256];

	GetCurrentDirectory(256, szCurrentDir);

	strcpy(szIniFilePath, szCurrentDir);
	if( szCurrentDir[strlen(szCurrentDir)-1] == '\\' ) 
		strcat(szIniFilePath, "config.ini");
	else strcat(szIniFilePath, "\\config.ini");

	platform::ClientIniConfigState client_ini_state;
	m_Version[0] = '\0';
	if (platform::LoadClientIniConfig(szIniFilePath, &client_ini_state))
	{
		strcpy_s(m_Version, _countof(m_Version), client_ini_state.login_version.c_str());
	}

	szServerIpAddress = new char[32];

	memset(szServerIpAddress, 0, sizeof(char) * 32);
	memcpy(szServerIpAddress, gProtect->m_MainInfo.IpAddress, sizeof(char) * 32);

	g_ServerPort = gProtect->m_MainInfo.IpAddressPort;

	Version[0] = gProtect->m_MainInfo.ClientVersion[0] + 1;
	Version[1] = gProtect->m_MainInfo.ClientVersion[2] + 2;
	Version[2] = gProtect->m_MainInfo.ClientVersion[3] + 3;
	Version[3] = gProtect->m_MainInfo.ClientVersion[5] + 4;
	Version[4] = gProtect->m_MainInfo.ClientVersion[6] + 5;

	memcpy(Serial, gProtect->m_MainInfo.ClientSerial, sizeof(Serial));

	char *lpszCommandLine = GetCommandLine();
	char lpszFile[MAX_PATH];
	if ( GetFileNameOfFilePath( lpszFile, lpszCommandLine))
	{
		WORD wVersion[4];
		if ( GetFileVersion( lpszFile, wVersion))
		{
			sprintf( m_ExeVersion, "%d.%02d", wVersion[0], wVersion[1]);
			if ( wVersion[2] > 0)
			{
                char lpszMinorVersion[3] = "a";
                if ( wVersion[2]>26 )
                {
                    lpszMinorVersion[0] = 'A';
				    lpszMinorVersion[0] += ( wVersion[2] - 27 );
                    lpszMinorVersion[1] = '+';
                }
                else
                {
				    lpszMinorVersion[0] += ( wVersion[2] - 1);
                }
				strcat( m_ExeVersion, lpszMinorVersion);
			}
		}
		else
		{
			strcpy( m_ExeVersion, m_Version);
		}
	}
	else
	{
		strcpy( m_ExeVersion, m_Version);
	}

//#ifdef _DEBUG

	platform::ClientRuntimeConfigState runtime_config_state;
	platform::InitializeClientRuntimeConfig(NULL, &runtime_config_state);

	platform::LanguageAssetBootstrapState language_asset_state;
	if (platform::InitializeLanguageAssetBootstrap(runtime_config_state.language.c_str(), &language_asset_state))
	{
		runtime_config_state.language = language_asset_state.resolved_language;
	}

	strcpy_s(m_ID, _countof(m_ID), runtime_config_state.player_id.c_str());
	m_SoundOnOff = runtime_config_state.sound_on;
	m_MusicOnOff = runtime_config_state.music_on;
	m_Resolution = runtime_config_state.resolution;
	m_nColorDepth = runtime_config_state.color_depth;
	g_iRenderTextType = runtime_config_state.render_text_type;
	g_iChatInputType = runtime_config_state.chat_input_type;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
	g_bUseWindowMode = runtime_config_state.window_mode ? TRUE : FALSE;
#endif // USER_WINDOW_MODE
	strcpy_s(g_aszMLSelection, _countof(g_aszMLSelection), runtime_config_state.language.c_str());
	g_strSelectedML = g_aszMLSelection;
	WindowWidth = runtime_config_state.window_width;
	WindowHeight = runtime_config_state.window_height;
	g_fScreenRate_x = runtime_config_state.screen_rate_x;
	g_fScreenRate_y = runtime_config_state.screen_rate_y;

	GWidescreen.Init();

	return TRUE;
}

BOOL Util_CheckOption( char *lpszCommandLine, unsigned char cOption, char *lpszString)
{
	unsigned char cComp[2];
	cComp[0] = cOption; cComp[1] = cOption;
	if ( islower( ( int)cOption))
	{
		cComp[1] = toupper( ( int)cOption);
	}
	else if ( isupper( ( int)cOption))
	{
		cComp[1] = tolower( ( int)cOption);
	}

	int nFind = ( int)'/';
	unsigned char *lpFound = ( unsigned char*)lpszCommandLine;
	while ( lpFound)
	{
		lpFound = ( unsigned char*)strchr( ( char*)( lpFound + 1), nFind);
		if ( lpFound && ( *( lpFound + 1) == cComp[0] || *( lpFound + 1) == cComp[1]))
		{	// 발견
			if ( lpszString)
			{
				int nCount = 0;
				for ( unsigned char *lpSeek = lpFound + 2; *lpSeek != ' ' && *lpSeek != '\0'; lpSeek++)
				{
					nCount++;
				}

				memcpy( lpszString, lpFound + 2, nCount);
				lpszString[nCount] = '\0';
			}
			return ( TRUE);
		}
	}

	return ( FALSE);
}

BOOL UpdateFile( char *lpszOld, char *lpszNew)
{
	SetFileAttributes(lpszOld, FILE_ATTRIBUTE_NORMAL);
	SetFileAttributes(lpszNew, FILE_ATTRIBUTE_NORMAL);

	DWORD dwStartTickCount = ::GetTickCount();
	while(::GetTickCount() - dwStartTickCount < 5000) {
		if ( CopyFile( lpszOld, lpszNew, FALSE))
		{	// 성공
			DeleteFile( lpszOld);
			return ( TRUE);
		}
	}
	g_ErrorReport.Write("%s to %s CopyFile Error : %d\r\n", lpszNew, lpszOld, GetLastError());
	return ( FALSE);
}

#include <tlhelp32.h>

BOOL KillExeProcess( char *lpszExe)
{
	HANDLE hProcessSnap = NULL; 
    BOOL bRet = FALSE; 
    PROCESSENTRY32 pe32 = { 0 }; 
 
    //  Take a snapshot of all processes in the system. 

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

    if (hProcessSnap == INVALID_HANDLE_VALUE) 
        return (FALSE); 
 
    //  Fill in the size of the structure before using it. 

    pe32.dwSize = sizeof(PROCESSENTRY32); 
 
    //  Walk the snapshot of the processes, and for each process, 
    //  display information. 

    if (Process32First(hProcessSnap, &pe32)) 
    {
        do 
        { 
			if(stricmp(pe32.szExeFile, lpszExe) == 0)
			{
				HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);

				if(process)
				{
					TerminateProcess(process, 0);
				}
			}
        } while (Process32Next(hProcessSnap, &pe32)); 
        bRet = TRUE; 
    } 
    else 
        bRet = FALSE;    // could not walk the list of processes 
 
    // Do not forget to clean up the snapshot object. 

    CloseHandle (hProcessSnap);

	return bRet;
}

char g_lpszCmdURL[50];
BOOL GetConnectServerInfo( PSTR szCmdLine, char *lpszURL, WORD *pwPort)
{
	char lpszTemp[256] = {0, };
	if( Util_CheckOption( szCmdLine, 'y', lpszTemp))
	{
		BYTE bySuffle[] = { 0x0C, 0x07, 0x03, 0x13 };

		for(int i=0; i<(int)strlen(lpszTemp); i++)
			lpszTemp[i] -= bySuffle[i%4];
		strcpy(lpszURL, lpszTemp);

		if( Util_CheckOption( szCmdLine, 'z', lpszTemp)) 
		{
			for(int j=0; j<(int)strlen(lpszTemp); j++)
				lpszTemp[j] -= bySuffle[j%4];
			*pwPort = atoi( lpszTemp);
		}
		g_ErrorReport.Write("[Virtual Connection] Connect IP : %s, Port : %d\r\n", lpszURL, *pwPort);
		return (TRUE);
	}
	if ( !Util_CheckOption( szCmdLine, 'u', lpszTemp))
	{
		return ( FALSE);
	}
	strcpy( lpszURL, lpszTemp);
	if ( !Util_CheckOption( szCmdLine, 'p', lpszTemp))
	{
		return ( FALSE);
	}
	*pwPort = atoi( lpszTemp);

	return ( TRUE);
}


extern int TimeRemain;
BOOL g_bInactiveTimeChecked = FALSE;
void MoveObject(OBJECT *o);

#include <dbghelp.h>
#include <Utilities/CpuUsage.h>

#pragma comment(lib,"dbghelp.lib")

bool ExceptionCallback(_EXCEPTION_POINTERS* pExceptionInfo )
{
	char path[MAX_PATH];

	SYSTEMTIME SystemTime;

	GetLocalTime(&SystemTime);

	wsprintf(path, "%d-%d-%d_%dh%dm%ds.dmp", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);

	HANDLE file = CreateFile(path, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	if (file != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION mdei;

		mdei.ThreadId = GetCurrentThreadId();

		mdei.ExceptionPointers = pExceptionInfo;

		mdei.ClientPointers = 0;

		if (MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, (MINIDUMP_TYPE)(MiniDumpScanMemory + MiniDumpWithIndirectlyReferencedMemory), &mdei, 0, 0) != 0)
		{
			CloseHandle(file);
			return EXCEPTION_EXECUTE_HANDLER;
		}
	}
	CloseHandle(file);

	return EXCEPTION_CONTINUE_SEARCH;
}

double CPU_AVG = 0.0;
void RecordCpuUsage()
{
	constexpr int max_recordings = 60;
	double CPU_Recordings[max_recordings] = { 0.0 };
	double currentAvg = 0.0;
	double sum = 0.0;
	int count = 0;
	int numFilled = 0;
	auto lastUpdateTime = std::chrono::steady_clock::now();

	while (!Destroy)
	{
		double currentUsage = CpuUsage::Instance()->GetUsage();

		currentUsage = max(0.0, min(100.0, currentUsage));

		// Subtract the old value to maintain the sum
		sum -= CPU_Recordings[count];

		sum += currentUsage;

		CPU_Recordings[count] = currentUsage;

		// Update the count (wrap around when full - FIFO behavior)
		count = (count + 1) % max_recordings;

		if (numFilled < max_recordings)
		{
			numFilled++;
		}

		// Calculate the current average
		currentAvg = sum / numFilled;

		// Update the CPU_AVG every 250 ms
		auto currentTime = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime).count() >= 250)
		{
			CPU_AVG = currentAvg;
			lastUpdateTime = currentTime;
		}

		// Sleep to match a 60Hz frame rate as the basis
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow)
{
	MSG msg;
	char enc1_path[_MAX_PATH] = { 0 };
	char dec2_path[_MAX_PATH] = { 0 };
	platform::CoreGameBootstrapState core_bootstrap_state;

	gController.Instance = hInstance;

	//leaf::AttachExceptionHandler(ExceptionCallback);

	gProtect = new CProtect;

	if (!platform::InitializeCoreGameProtect(gProtect, &core_bootstrap_state))
	{
		const char* error_message = core_bootstrap_state.error_message.empty()
			? "Erro ao localizar a pasta Data do cliente."
			: core_bootstrap_state.error_message.c_str();
		MessageBox(0, error_message, "Erro de inicializa豫o!", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	if (core_bootstrap_state.enc1_path.size() >= _countof(enc1_path) ||
		core_bootstrap_state.dec2_path.size() >= _countof(dec2_path))
	{
		MessageBox(0, "Erro de configura豫o. Caminhos do bootstrap excederam o limite do cliente.", "Erro de inicializa豫o!", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	strcpy_s(enc1_path, _countof(enc1_path), core_bootstrap_state.enc1_path.c_str());
	strcpy_s(dec2_path, _countof(dec2_path), core_bootstrap_state.dec2_path.c_str());

	gTrayMode.Load();

	gCustomJewel.Load(gProtect->m_CustomJewel.CustomJewelInfo);

	gItemManager.Init();

	gHelperSystem.Init();

	gDarkSpirit.Init();

	gCustomItemPosition.Init();

	gCustomItemSize.Init();

	gDescriptions.Init();

	Visuals.Init();

	gDisableExcellent.Init();

	gCustomBow.Init();

	gCustomWing.Init();

	gCustomEffects.Init();

	gCCustomSetEffect.Init();

	gCustomItemFloor.Init();

	gCustomItemForce.Init();

	gRenderModel.Init();

	gCustomServerName.Init();

	gCreateFont.Init();

	gMessageColor.Init();

	gMonsters.Init();

	gMonsterName.Init();

	gMonsterGlow.Init();

	gMonsterEffect.Init();

	gCustomCape.Init();

	gJewelStack.Init();

	gCounterItem.Init();

	gCharacterList.Init();

	gPatente.Init();

	gControlSlot.Init();

	char lpszExeVersion[256] = "unknown";

	char *lpszCommandLine = GetCommandLine();
	char lpszFile[MAX_PATH];
	WORD wVersion[4] = { 0,};
	if ( GetFileNameOfFilePath( lpszFile, lpszCommandLine))
	{
		if ( GetFileVersion( lpszFile, wVersion))
		{
			sprintf( lpszExeVersion, "%d.%02d", wVersion[0], wVersion[1]);
			if ( wVersion[2] > 0)
			{
				char lpszMinorVersion[2] = "a";
				lpszMinorVersion[0] += ( wVersion[2] - 1);
				strcat( lpszExeVersion, lpszMinorVersion);
			}
		}
	}

#ifdef CONSOLE
	Console.Init();
#endif
	g_ErrorReport.Write( "\r\n");
	g_ErrorReport.WriteLogBegin();
	g_ErrorReport.AddSeparator();
	g_ErrorReport.Write( "Mu online %s (%s) executed. (%d.%d.%d.%d)\r\n", lpszExeVersion, "Eng", wVersion[0], wVersion[1], wVersion[2], wVersion[3]);
	g_ErrorReport.Write( "> Game data root = %s\r\n", platform::GetGameAssetRoot());

	g_ConsoleDebug->Write(MCD_NORMAL, "Mu Online (Version: %d.%d.%d.%d)", wVersion[0], wVersion[1], wVersion[2], wVersion[3]);

	g_ErrorReport.WriteCurrentTime();
	ER_SystemInfo si;
	ZeroMemory( &si, sizeof ( ER_SystemInfo));
	GetSystemInfo( &si);
	g_ErrorReport.AddSeparator();
	g_ErrorReport.WriteSystemInfo( &si);
	g_ErrorReport.AddSeparator();

	// PKD_ADD_BINARY_PROTECTION
	VM_START
	WORD wPortNumber;	
	if ( GetConnectServerInfo( szCmdLine, g_lpszCmdURL, &wPortNumber))
	{
		szServerIpAddress = g_lpszCmdURL;
		g_ServerPort = wPortNumber;
	}
	VM_END

	if ( !OpenMainExe())
	{
		return false;
	}

	// PKD_ADD_BINARY_PROTECTION
	VM_START
	g_SimpleModulusCS.LoadEncryptionKey(enc1_path);
	g_SimpleModulusSC.LoadDecryptionKey(dec2_path);
	VM_END

	g_ErrorReport.Write( "> To read config.ini.\r\n");

	if( OpenInitFile() == FALSE )
	{
		g_ErrorReport.Write( "config.ini read error\r\n");
		return false;
	}

	pMultiLanguage = new CMultiLanguage(g_strSelectedML);

	if (g_iChatInputType == 1)
		ShowCursor(FALSE);

	g_ErrorReport.Write( "> Enum display settings.\r\n");
	DEVMODE DevMode;
	DEVMODE* pDevmodes;
	int nModes = 0;
	while (EnumDisplaySettings(NULL, nModes, &DevMode)) nModes++;
	pDevmodes = new DEVMODE[nModes+1];
	nModes = 0;
	while (EnumDisplaySettings(NULL, nModes, &pDevmodes[nModes])) nModes++;

	DWORD dwBitsPerPel = 16;
	for(int n1=0; n1<nModes; n1++)
	{
		if(pDevmodes[n1].dmBitsPerPel == 16 && m_nColorDepth == 0) {
			dwBitsPerPel = 16; break;
		}
		if(pDevmodes[n1].dmBitsPerPel == 24 && m_nColorDepth == 1) {
			dwBitsPerPel = 24; break;
		}
		if(pDevmodes[n1].dmBitsPerPel == 32 && m_nColorDepth == 1) {
			dwBitsPerPel = 32; break;
		}
	}

#ifdef ENABLE_FULLSCREEN
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
	if (g_bUseWindowMode == FALSE)
#endif	// USER_WINDOW_MODE
	{
		for(int n2=0; n2<nModes; n2++)
		{
			if(pDevmodes[n2].dmPelsWidth==WindowWidth && pDevmodes[n2].dmPelsHeight==WindowHeight && pDevmodes[n2].dmBitsPerPel == dwBitsPerPel)
			{
				g_ErrorReport.Write( "> Change display setting %dx%d.\r\n", pDevmodes[n2].dmPelsWidth, pDevmodes[n2].dmPelsHeight);
				ChangeDisplaySettings(&pDevmodes[n2],0);
				break;
			}
		}
	}
#endif //ENABLE_FULLSCREEN

	delete [] pDevmodes;

	g_ErrorReport.Write( "> Screen size = %d x %d.\r\n", WindowWidth, WindowHeight);

    g_hInst = hInstance;
    g_hWnd = StartWindow(hInstance,nCmdShow);
	if (g_hWnd == NULL)
	{
		return FALSE;
	}
	g_ErrorReport.Write( "> Start window success.\r\n");

    if ( !CreateOpenglWindow())
	{
		return FALSE;
	}

	g_ErrorReport.Write( "> OpenGL init success.\r\n");
	g_ErrorReport.AddSeparator();
	//g_ErrorReport.WriteOpenGLInfo();
	g_ErrorReport.AddSeparator();
	g_ErrorReport.WriteSoundCardInfo(); 

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

	//g_ErrorReport.WriteImeInfo( g_hWnd);
	g_ErrorReport.AddSeparator();
	
	switch(WindowWidth)
	{
		case 640:  FontHeight = 12; break;
		case 800:  FontHeight = 13; break;
		case 1024: FontHeight = 13; break;
		case 1200: FontHeight = 16; break;
		case 1280: FontHeight = 16; break;
		case 1365: FontHeight = 16; break;
		case 1400: FontHeight = 16; break;
		case 1600: FontHeight = 16; break;
		case 1920: FontHeight = 17; break;
		default: FontHeight = 16; break;
	}
	
	SetTargetFps(-1); // unlimited

	FontHeight = static_cast<int>(std::ceil(12 + ((WindowHeight - 480) / 200.f)));
	
	int nFixFontHeight = WindowHeight <= 600 ? 14 : 15;
	int nFixFontSize;
	int iFontSize;

	iFontSize = FontHeight - 1;
	nFixFontSize = nFixFontHeight - 1;

	gCreateFont.SetFont(iFontSize, FontHeight, nFixFontSize, nFixFontHeight);

	//g_hFont = CreateFont(iFontSize + gCreateFont.m_fontInfo[0].AddSize, 0, 0, 0, (gCreateFont.m_fontInfo[0].Bold == 1 ? FW_BOLD : FW_NORMAL), 0, 0, 0, gCreateFont.m_fontInfo[0].CharSet, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, gCreateFont.m_fontInfo[0].Name.c_str());
	//g_hFontBold = CreateFont(iFontSize + gCreateFont.m_fontInfo[1].AddSize, 0, 0, 0, (gCreateFont.m_fontInfo[1].Bold == 1 ? FW_BOLD : FW_NORMAL), 0, 0, 0, gCreateFont.m_fontInfo[1].CharSet, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, gCreateFont.m_fontInfo[1].Name.c_str());
	//g_hFontBig = CreateFont((iFontSize * 2) + gCreateFont.m_fontInfo[2].AddSize, 0, 0, 0, (gCreateFont.m_fontInfo[2].Bold == 1 ? FW_BOLD : FW_NORMAL), 0, 0, 0, gCreateFont.m_fontInfo[2].CharSet, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, gCreateFont.m_fontInfo[2].Name.c_str());
	//g_hFixFont = CreateFont(nFixFontSize + gCreateFont.m_fontInfo[3].AddSize, 0, 0, 0, (gCreateFont.m_fontInfo[3].Bold == 1 ? FW_BOLD : FW_NORMAL), 0, 0, 0, gCreateFont.m_fontInfo[3].CharSet, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, gCreateFont.m_fontInfo[3].Name.c_str());
	
	setlocale( LC_ALL, "english");

	CInput::Instance().Create(g_hWnd, WindowWidth, WindowHeight);

	g_pNewUISystem->Create();

	if(m_MusicOnOff)
	{
		wzAudioCreate(g_hWnd);
		wzAudioOption(WZAOPT_STOPBEFOREPLAY, 1);
	}

	if(m_SoundOnOff)
	{
		InitDirectSound(g_hWnd);
		leaf::CRegKey regkey;
		regkey.SetKey(leaf::CRegKey::_HKEY_CURRENT_USER, "SOFTWARE\\Webzen\\Mu\\Config");
		DWORD value;
		if(!regkey.ReadDword("VolumeLevel", value))
		{
			value = 5;	//. default setting
			regkey.WriteDword("VolumeLevel", value);
		}
		if(value<0 || value>=10)
			value = 5;
		
		g_pOption->SetVolumeLevel(int(value));
		SetEffectVolumeLevel(g_pOption->GetVolumeLevel());
	}

	SetTimer(g_hWnd, HACK_TIMER, 20*1000, NULL);

	srand((unsigned)time(NULL));
	for(int i=0;i<100;i++)
		RandomTable[i] = rand()%360;

	//memorydump[0]
	RendomMemoryDump = new BYTE [rand()%100+1];


	GateAttribute				= new GATE_ATTRIBUTE [MAX_GATES];
	SkillAttribute				= new SKILL_ATTRIBUTE[MAX_SKILLS];

	//memorydump[1]
	ItemAttRibuteMemoryDump		= new ITEM_ATTRIBUTE [MAX_ITEM+1024];
	ItemAttribute				= ((ITEM_ATTRIBUTE*)ItemAttRibuteMemoryDump)+rand()%1024;

	//memorydump[2]
	CharacterMemoryDump			= new CHARACTER      [MAX_CHARACTERS_CLIENT+1+128];
	CharactersClient			= ((CHARACTER*)CharacterMemoryDump)+rand()%128;
	CharacterMachine			= new CHARACTER_MACHINE;

	memset(GateAttribute       ,0,sizeof(GATE_ATTRIBUTE   )*(MAX_GATES              ));
	memset(ItemAttribute       ,0,sizeof(ITEM_ATTRIBUTE   )*(MAX_ITEM               ));
	memset(SkillAttribute      ,0,sizeof(SKILL_ATTRIBUTE  )*(MAX_SKILLS             ));
	memset(CharacterMachine    ,0,sizeof(CHARACTER_MACHINE));

    CharacterAttribute   = &CharacterMachine->Character;
    CharacterMachine->Init();
	Hero = &CharactersClient[0];	

	if (g_iChatInputType == 1)
	{
		 g_pMercenaryInputBox = new CUIMercenaryInputBox;
		 g_pSingleTextInputBox = new CUITextInputBox;
		 g_pSinglePasswdInputBox = new CUITextInputBox;
	}
	
	g_pChatRoomSocketList = new CChatRoomSocketList;
	g_pUIManager = new CUIManager;
	g_pUIMapName = new CUIMapName;	// rozy
	g_pTimer = new CTimer();

#ifdef MOVIE_DIRECTSHOW
	g_pMovieScene = new CMovieScene;
#endif // MOVIE_DIRECTSHOW

#ifdef NEW_MUHELPER_ON
	pAIController = new CAIController(Hero);
#endif

	g_BuffSystem = BuffStateSystem::Make();

	g_MapProcess = MapProcess::Make();

	g_petProcess = PetProcess::Make();

	CUIMng::Instance().Create();

	if (g_iChatInputType == 1)
	{
		g_pMercenaryInputBox->Init(g_hWnd);
		g_pSingleTextInputBox->Init(g_hWnd, 200, 20);
		g_pSinglePasswdInputBox->Init(g_hWnd, 200, 20, 9, TRUE);
		g_pSingleTextInputBox->SetState(UISTATE_HIDE);
		g_pSinglePasswdInputBox->SetState(UISTATE_HIDE);

		g_pMercenaryInputBox->SetFont(g_hFont);
		g_pSingleTextInputBox->SetFont(g_hFont);
		g_pSinglePasswdInputBox->SetFont(g_hFont);

		g_bIMEBlock = FALSE;
		HIMC  hIMC = ImmGetContext(g_hWnd);
		ImmSetConversionStatus(hIMC, IME_CMODE_ALPHANUMERIC, IME_SMODE_NONE );
		ImmReleaseContext(g_hWnd, hIMC);
		SaveIMEStatus();
		g_bIMEBlock = TRUE;
	}
#if (defined WINDOWMODE)
	if (g_bUseWindowMode == FALSE)
	{
		int nOldVal;
		SystemParametersInfo(SPI_SCREENSAVERRUNNING, 1, &nOldVal, 0);
		SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &g_iScreenSaverOldValue, 0);
		SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, 300*60, NULL, 0);
	}
#else
#ifdef NDEBUG
#ifndef FOR_WORK
#ifdef ACTIVE_FOCUS_OUT
	if (g_bUseWindowMode == FALSE)
	{
#endif	// ACTIVE_FOCUS_OUT
		int nOldVal; // 값이 들어갈 필요가 없음
		SystemParametersInfo(SPI_SCREENSAVERRUNNING, 1, &nOldVal, 0);  // 단축키를 못쓰게 함
		SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &g_iScreenSaverOldValue, 0);  // 스크린세이버 차단
		SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, 300*60, NULL, 0);  // 스크린세이버 차단
#ifdef ACTIVE_FOCUS_OUT
	}
#endif	// ACTIVE_FOCUS_OUT
#endif
#endif
#endif	//WINDOWMODE(#else)

#ifdef SAVE_PACKET
	DeleteFile( PACKET_SAVE_FILE);
#endif

#if defined PROTECT_SYSTEMKEY && defined NDEBUG
#ifndef FOR_WORK
	ProtectSysKey::AttachProtectSysKey(g_hInst, g_hWnd);
#endif // !FOR_WORK
#endif // PROTECT_SYSTEMKEY && NDEBUG

	constexpr auto target_resolution = 1;
	auto precise = timeBeginPeriod(target_resolution);

	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0))
			{
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (CheckRenderNextFrame())
			{
				//Scene
#if (defined WINDOWMODE)
				if (g_bUseWindowMode || g_bWndActive)
				{
					RenderScene(g_hDC);
				}
#ifndef FOR_WORK
				else if (g_bUseWindowMode == FALSE)
				{
					SetForegroundWindow(g_hWnd);
					SetFocus(g_hWnd);

					if (g_iInactiveWarning > 1)
					{
						SetTimer(g_hWnd, WINDOWMINIMIZED_TIMER, 1 * 1000, NULL);
						PostMessage(g_hWnd, WM_CLOSE, 0, 0);
					}
					else
					{
						g_iInactiveWarning++;
						g_bMinimizedEnabled = TRUE;
						ShowWindow(g_hWnd, SW_MINIMIZE);
						g_bMinimizedEnabled = FALSE;
						ShowWindow(g_hWnd, SW_MAXIMIZE);
					}
				}
#endif//FOR_WORK
#else//WINDOWMODE
				if (g_bWndActive)
					Scene(g_hDC);

#endif	//WINDOWMODE(#else)
			}
			else
			{
				if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
				{
					WaitForNextActivity(precise == TIMERR_NOERROR);
				}
			}
		}
	#ifdef NEW_PROTOCOL_SYSTEM
		if(SceneFlag < CHARACTER_SCENE)
			ProtocolCompiler();

		g_pChatRoomSocketList->ProtocolCompile();
		gProtocolSend.RecvMessage();
	#else
		ProtocolCompiler();
		g_pChatRoomSocketList->ProtocolCompile();
	#endif

		
    } // while( 1 )

	if (precise == TIMERR_NOERROR)
	{
		timeEndPeriod(target_resolution);
	}

	std::thread cpuUsageRecorder(RecordCpuUsage);
	
	DestroyWindow();

    return msg.wParam;
}

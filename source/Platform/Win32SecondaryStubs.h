#pragma once

#if defined(__ANDROID__)

#include "Platform/AndroidWin32Compat.h"
#include "Platform/AndroidTextRenderer.h"

#ifndef HIMC
typedef void* HIMC;
#endif

#ifndef IME_CMODE_ALPHANUMERIC
#define IME_CMODE_ALPHANUMERIC 0x0000
#endif

#ifndef IME_SMODE_NONE
#define IME_SMODE_NONE 0x0000
#endif

#ifndef IME_SMODE_AUTOMATIC
#define IME_SMODE_AUTOMATIC 0x0004
#endif

#ifndef IME_CMODE_NATIVE
#define IME_CMODE_NATIVE 0x0001
#endif

#ifndef IME_CONVERSIONMODE
#define IME_CONVERSIONMODE 0x0001
#endif

#ifndef IME_SENTENCEMODE
#define IME_SENTENCEMODE 0x0002
#endif

#ifndef WM_IME_CONTROL
#define WM_IME_CONTROL 0x0283
#endif

#ifndef IMC_SETCOMPOSITIONWINDOW
#define IMC_SETCOMPOSITIONWINDOW 0x000C
#endif

#ifndef PM_NOREMOVE
#define PM_NOREMOVE 0x0000
#endif

#ifndef PM_REMOVE
#define PM_REMOVE 0x0001
#endif

#ifndef TIMERR_NOERROR
#define TIMERR_NOERROR 0
#endif

#ifndef HACK_TIMER
#define HACK_TIMER 1
#endif

#ifndef WINDOWMINIMIZED_TIMER
#define WINDOWMINIMIZED_TIMER 2
#endif

#ifndef SPI_SCREENSAVERRUNNING
#define SPI_SCREENSAVERRUNNING 0x0061
#endif

#ifndef SPI_GETSCREENSAVETIMEOUT
#define SPI_GETSCREENSAVETIMEOUT 0x000E
#endif

#ifndef SPI_SETSCREENSAVETIMEOUT
#define SPI_SETSCREENSAVETIMEOUT 0x000F
#endif

#ifndef GWL_STYLE
#define GWL_STYLE (-16)
#endif

#ifndef FW_NORMAL
#define FW_NORMAL 400
#endif

#ifndef WM_TIMER
#define WM_TIMER 0x0113
#endif

#ifndef WM_KEYDOWN
#define WM_KEYDOWN 0x0100
#endif

#ifndef WM_KEYUP
#define WM_KEYUP 0x0101
#endif

#ifndef WM_CHAR
#define WM_CHAR 0x0102
#endif

#ifndef WM_LBUTTONDOWN
#define WM_LBUTTONDOWN 0x0201
#endif

#ifndef WM_LBUTTONUP
#define WM_LBUTTONUP 0x0202
#endif

#ifndef WM_RBUTTONDOWN
#define WM_RBUTTONDOWN 0x0204
#endif

#ifndef WM_RBUTTONUP
#define WM_RBUTTONUP 0x0205
#endif

#ifndef WM_MOUSEMOVE
#define WM_MOUSEMOVE 0x0200
#endif

#ifndef WM_MBUTTONDOWN
#define WM_MBUTTONDOWN 0x0207
#endif

#ifndef WM_MBUTTONUP
#define WM_MBUTTONUP 0x0208
#endif

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

#ifndef WM_SIZE
#define WM_SIZE 0x0005
#endif

#ifndef WM_PAINT
#define WM_PAINT 0x000F
#endif

#ifndef WM_ACTIVATE
#define WM_ACTIVATE 0x0006
#endif

#ifndef WM_COMMAND
#define WM_COMMAND 0x0111
#endif

#ifndef WM_SYSCOMMAND
#define WM_SYSCOMMAND 0x0112
#endif

#ifndef WM_SETFOCUS
#define WM_SETFOCUS 0x0007
#endif

#ifndef WM_KILLFOCUS
#define WM_KILLFOCUS 0x0008
#endif

#ifndef WM_IME_COMPOSITION
#define WM_IME_COMPOSITION 0x010F
#endif

#ifndef WM_IME_STARTCOMPOSITION
#define WM_IME_STARTCOMPOSITION 0x010D
#endif

#ifndef WM_IME_ENDCOMPOSITION
#define WM_IME_ENDCOMPOSITION 0x010E
#endif

#ifndef WM_INPUTLANGCHANGE
#define WM_INPUTLANGCHANGE 0x0051
#endif

#ifndef GCS_RESULTSTR
#define GCS_RESULTSTR 0x0800
#endif

#ifndef GCS_COMPSTR
#define GCS_COMPSTR 0x0008
#endif

#ifndef CFS_POINT
#define CFS_POINT 0x0002
#endif

typedef struct tagCOMPOSITIONFORM
{
	DWORD dwStyle;
	POINT ptCurrentPos;
	RECT rcArea;
} COMPOSITIONFORM, *LPCOMPOSITIONFORM;

typedef struct tagMSG
{
	HWND hwnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
	DWORD time;
	POINT pt;
} MSG;

typedef struct _SYSTEMTIME
{
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME;

inline SHORT GetAsyncKeyState(int vKey)
{
	(void)vKey;
	return 0;
}

inline SHORT GetKeyState(int vKey)
{
	(void)vKey;
	return 0;
}

inline HIMC ImmGetContext(HWND hWnd)
{
	(void)hWnd;
	return nullptr;
}

inline BOOL ImmReleaseContext(HWND hWnd, HIMC hIMC)
{
	(void)hWnd;
	(void)hIMC;
	return TRUE;
}

inline BOOL ImmSetConversionStatus(HIMC hIMC, DWORD fdwConversion, DWORD fdwSentence)
{
	(void)hIMC;
	(void)fdwConversion;
	(void)fdwSentence;
	return TRUE;
}

inline BOOL ImmGetConversionStatus(HIMC hIMC, DWORD* lpfdwConversion, DWORD* lpfdwSentence)
{
	(void)hIMC;
	if (lpfdwConversion) *lpfdwConversion = IME_CMODE_ALPHANUMERIC;
	if (lpfdwSentence) *lpfdwSentence = IME_SMODE_NONE;
	return TRUE;
}

inline LONG ImmGetCompositionStringA(HIMC hIMC, DWORD dwIndex, LPVOID lpBuf, DWORD dwBufLen)
{
	(void)hIMC;
	(void)dwIndex;
	(void)lpBuf;
	(void)dwBufLen;
	return 0;
}

inline BOOL ImmSetCompositionWindow(HIMC hIMC, LPCOMPOSITIONFORM lpCompForm)
{
	(void)hIMC;
	(void)lpCompForm;
	return TRUE;
}

inline HWND ImmGetDefaultIMEWnd(HWND hWnd)
{
	(void)hWnd;
	return nullptr;
}

inline UINT_PTR SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, void* lpTimerFunc)
{
	(void)hWnd;
	(void)uElapse;
	(void)lpTimerFunc;
	return nIDEvent;
}

inline BOOL KillTimer(HWND hWnd, UINT_PTR uIDEvent)
{
	(void)hWnd;
	(void)uIDEvent;
	return TRUE;
}

inline LONG ChangeDisplaySettings(void* lpDevMode, DWORD dwFlags)
{
	(void)lpDevMode;
	(void)dwFlags;
	return 0;
}

inline BOOL ShowCursor(BOOL bShow)
{
	(void)bShow;
	return 0;
}

inline BOOL SystemParametersInfo(UINT uiAction, UINT uiParam, void* pvParam, UINT fWinIni)
{
	(void)uiAction;
	(void)uiParam;
	(void)pvParam;
	(void)fWinIni;
	return TRUE;
}

inline void GetLocalTime(SYSTEMTIME* lpSystemTime)
{
	if (lpSystemTime != nullptr)
	{
		time_t now = time(nullptr);
		struct tm* lt = localtime(&now);
		if (lt != nullptr)
		{
			lpSystemTime->wYear = static_cast<WORD>(lt->tm_year + 1900);
			lpSystemTime->wMonth = static_cast<WORD>(lt->tm_mon + 1);
			lpSystemTime->wDayOfWeek = static_cast<WORD>(lt->tm_wday);
			lpSystemTime->wDay = static_cast<WORD>(lt->tm_mday);
			lpSystemTime->wHour = static_cast<WORD>(lt->tm_hour);
			lpSystemTime->wMinute = static_cast<WORD>(lt->tm_min);
			lpSystemTime->wSecond = static_cast<WORD>(lt->tm_sec);
			lpSystemTime->wMilliseconds = 0;
		}
		else
		{
			memset(lpSystemTime, 0, sizeof(SYSTEMTIME));
		}
	}
}

inline void GetSystemInfo(void* lpSystemInfo)
{
	if (lpSystemInfo != nullptr)
	{
		memset(lpSystemInfo, 0, 128);
	}
}

inline BOOL PeekMessage(MSG* lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	(void)lpMsg;
	(void)hWnd;
	(void)wMsgFilterMin;
	(void)wMsgFilterMax;
	(void)wRemoveMsg;
	return FALSE;
}

inline BOOL GetMessage(MSG* lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
	(void)lpMsg;
	(void)hWnd;
	(void)wMsgFilterMin;
	(void)wMsgFilterMax;
	return FALSE;
}

inline BOOL TranslateMessage(const MSG* lpMsg)
{
	(void)lpMsg;
	return FALSE;
}

inline LRESULT DispatchMessage(const MSG* lpMsg)
{
	(void)lpMsg;
	return 0;
}

inline UINT timeBeginPeriod(UINT uPeriod)
{
	(void)uPeriod;
	return TIMERR_NOERROR;
}

inline UINT timeEndPeriod(UINT uPeriod)
{
	(void)uPeriod;
	return TIMERR_NOERROR;
}

inline BOOL EnumDisplaySettings(void* lpszDeviceName, DWORD iModeNum, void* lpDevMode)
{
	(void)lpszDeviceName;
	(void)iModeNum;
	(void)lpDevMode;
	return FALSE;
}

inline void SetForegroundWindow(HWND hWnd)
{
	(void)hWnd;
}

inline HWND SetFocus(HWND hWnd)
{
	(void)hWnd;
	return hWnd;
}

inline BOOL ShowWindow(HWND hWnd, int nCmdShow)
{
	(void)hWnd;
	(void)nCmdShow;
	return TRUE;
}

inline BOOL UpdateWindow(HWND hWnd)
{
	(void)hWnd;
	return TRUE;
}

inline BOOL InvalidateRect(HWND hWnd, const RECT* lpRect, BOOL bErase)
{
	(void)hWnd;
	(void)lpRect;
	(void)bErase;
	return TRUE;
}

inline BOOL DestroyWindow(HWND hWnd)
{
	(void)hWnd;
	return TRUE;
}

inline BOOL GetClientRect(HWND hWnd, RECT* lpRect)
{
	(void)hWnd;
	if (lpRect != nullptr)
	{
		lpRect->left = 0;
		lpRect->top = 0;
		lpRect->right = 640;
		lpRect->bottom = 480;
	}
	return TRUE;
}

inline BOOL MoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
	(void)hWnd;
	(void)X;
	(void)Y;
	(void)nWidth;
	(void)nHeight;
	(void)bRepaint;
	return TRUE;
}

// SaveIMEStatus: real implementation is in UIControls.cpp
// Do NOT provide inline stub here — it causes ODR conflicts.
// The declaration is in UIControls.h.

inline int _access(const char* path, int mode)
{
	return access(path, mode);
}

#ifndef _INC_IO
#define _INC_IO
#endif

// Win32 hook constants and types
#ifndef ULONG_PTR
typedef unsigned long ULONG_PTR;
#endif

#ifndef HHOOK
typedef void* HHOOK;
#endif

#ifndef WH_MOUSE
#define WH_MOUSE 7
#endif
#ifndef WH_KEYBOARD
#define WH_KEYBOARD 2
#endif
#ifndef HC_ACTION
#define HC_ACTION 0
#endif

typedef struct tagMOUSEHOOKSTRUCTEX
{
	POINT pt;
	HWND hwnd;
	UINT wHitTestCode;
	ULONG_PTR dwExtraInfo;
	DWORD mouseData;
} MOUSEHOOKSTRUCTEX;

inline HHOOK SetWindowsHookEx(int idHook, void* lpfn, HINSTANCE hmod, DWORD dwThreadId)
{
	(void)idHook;
	(void)lpfn;
	(void)hmod;
	(void)dwThreadId;
	return nullptr;
}

inline BOOL UnhookWindowsHookEx(HHOOK hhk)
{
	(void)hhk;
	return TRUE;
}

inline LRESULT CallNextHookEx(HHOOK hhk, int nCode, WPARAM wParam, LPARAM lParam)
{
	(void)hhk;
	(void)nCode;
	(void)wParam;
	(void)lParam;
	return 0;
}

inline HWND GetForegroundWindow()
{
	return nullptr;
}

inline DWORD GetCurrentThreadId()
{
	return static_cast<DWORD>(pthread_self());
}

// strsafe.h replacement
#ifndef _STRSAFE_H_INCLUDED_
#define _STRSAFE_H_INCLUDED_
#ifndef STRSAFE_MAX_CCH
#define STRSAFE_MAX_CCH 2147483647
#endif
inline int StringCchPrintfA(char* pszDest, size_t cchDest, const char* pszFormat, ...)
{
	va_list args;
	va_start(args, pszFormat);
	int r = vsnprintf(pszDest, cchDest, pszFormat, args);
	va_end(args);
	return r >= 0 ? 0 : -1;
}
#define StringCchPrintf StringCchPrintfA
inline int StringCchCopyA(char* pszDest, size_t cchDest, const char* pszSrc)
{
	if (pszDest == NULL || cchDest == 0) return -1;
	strncpy(pszDest, pszSrc ? pszSrc : "", cchDest - 1);
	pszDest[cchDest - 1] = '\0';
	return 0;
}
#define StringCchCopy StringCchCopyA
#endif

// --- _mbclen (MSVC multibyte character length) ---
// Android bionic libc may not expose mblen(); use a simple byte-length check instead.
inline size_t _mbclen(const unsigned char* c)
{
	if (!c || *c == 0) return 0;
	// Simple single-byte assumption (UTF-8 lead byte detection for safety)
	if (*c < 0x80) return 1;
	if ((*c & 0xE0) == 0xC0) return 2;
	if ((*c & 0xF0) == 0xE0) return 3;
	if ((*c & 0xF8) == 0xF0) return 4;
	return 1;
}

// --- MultiByteToWideChar / WideCharToMultiByte ---
#include <wchar.h>
#include <stdlib.h>
inline int MultiByteToWideChar(UINT codePage, DWORD dwFlags, LPCSTR lpMultiByteStr,
	int cbMultiByte, wchar_t* lpWideCharStr, int cchWideChar)
{
	(void)codePage; (void)dwFlags;
	if (!lpMultiByteStr) return 0;
	size_t srcLen = (cbMultiByte == -1) ? (strlen(lpMultiByteStr) + 1) : (size_t)cbMultiByte;
	if (cchWideChar == 0) {
		// query required size
		return (int)srcLen; // approximate 1:1 for ASCII/Latin
	}
	size_t i = 0;
	for (i = 0; i < srcLen && i < (size_t)cchWideChar; i++) {
		lpWideCharStr[i] = (wchar_t)(unsigned char)lpMultiByteStr[i];
	}
	return (int)i;
}

inline int WideCharToMultiByte(UINT codePage, DWORD dwFlags, LPCWSTR lpWideCharStr,
	int cchWideChar, char* lpMultiByteStr, int cbMultiByte, const char* lpDefaultChar, BOOL* lpUsedDefaultChar)
{
	(void)codePage; (void)dwFlags; (void)lpDefaultChar; (void)lpUsedDefaultChar;
	if (!lpWideCharStr) return 0;
	size_t srcLen = (cchWideChar == -1) ? (wcslen(lpWideCharStr) + 1) : (size_t)cchWideChar;
	if (cbMultiByte == 0) {
		return (int)srcLen;
	}
	size_t i = 0;
	for (i = 0; i < srcLen && i < (size_t)cbMultiByte; i++) {
		wchar_t wc = lpWideCharStr[i];
		lpMultiByteStr[i] = (wc < 128) ? (char)wc : '?';
	}
	return (int)i;
}

// --- GetTextExtentPoint32W / TextOutW (AndroidTextRenderer) ---
inline BOOL GetTextExtentPoint32W(HDC hdc, LPCWSTR lpString, int c, LPSIZE lpSize)
{
	if (!lpSize) return FALSE;
	return platform::android_text::GetTextExtent(hdc, lpString, c, &lpSize->cx, &lpSize->cy) ? TRUE : FALSE;
}

inline BOOL TextOutW(HDC hdc, int x, int y, LPCWSTR lpString, int c)
{
	return platform::android_text::TextOut(hdc, x, y, lpString, c) ? TRUE : FALSE;
}

// --- SetCursorPos (no-op on Android) ---
inline BOOL SetCursorPos(int X, int Y)
{
	(void)X; (void)Y;
	return TRUE;
}

// --- INTERNET_MAX_URL_LENGTH ---
#ifndef INTERNET_MAX_URL_LENGTH
#define INTERNET_MAX_URL_LENGTH 2083
#endif

// --- TCHAR (narrow char on Android) ---
#ifndef _TCHAR_DEFINED
typedef char TCHAR;
#define _TCHAR_DEFINED
#endif
#ifndef LPTSTR
typedef char* LPTSTR;
#endif
#ifndef LPCTSTR
typedef const char* LPCTSTR;
#endif

// --- _ultoa_s (MSVC secure unsigned long to string) ---
#include <cstdio>
inline int _ultoa_s(unsigned long value, char* buffer, size_t sizeOfBuffer, int radix)
{
	if (!buffer || sizeOfBuffer == 0) return -1;
	if (radix == 10) {
		snprintf(buffer, sizeOfBuffer, "%lu", value);
	} else if (radix == 16) {
		snprintf(buffer, sizeOfBuffer, "%lx", value);
	} else if (radix == 8) {
		snprintf(buffer, sizeOfBuffer, "%lo", value);
	} else {
		snprintf(buffer, sizeOfBuffer, "%lu", value);
	}
	return 0;
}
// MSVC template overload: _ultoa_s(value, char (&buf)[N], radix)
template <size_t N>
inline int _ultoa_s(unsigned long value, char (&buffer)[N], int radix)
{
	return _ultoa_s(value, buffer, N, radix);
}

// --- UCHAR typedef ---
#ifndef _UCHAR_DEFINED
typedef unsigned char UCHAR;
#define _UCHAR_DEFINED
#endif

// --- Edit control messages ---
#ifndef EM_GETSEL
#define EM_GETSEL       0x00B0
#endif
#ifndef EM_SETSEL
#define EM_SETSEL       0x00B1
#endif
#ifndef EM_REPLACESEL
#define EM_REPLACESEL   0x00C2
#endif
#ifndef EM_GETLINE
#define EM_GETLINE      0x00C4
#endif
#ifndef EM_LIMITTEXT
#define EM_LIMITTEXT    0x00C5
#endif
#ifndef EM_SETLIMITTEXT
#define EM_SETLIMITTEXT EM_LIMITTEXT
#endif
#ifndef EM_SETPASSWORDCHAR
#define EM_SETPASSWORDCHAR 0x00CC
#endif
#ifndef EM_LINESCROLL
#define EM_LINESCROLL   0x00CF
#endif
#ifndef EM_GETLINECOUNT
#define EM_GETLINECOUNT 0x00BA
#endif
#ifndef EM_SCROLL
#define EM_SCROLL       0x00B5
#endif

// --- Edit control styles (supplement to AndroidWin32Compat.h) ---
#ifndef ES_NUMBER
#define ES_NUMBER       0x2000L
#endif

// --- Window styles (supplement to AndroidWin32Compat.h) ---
#ifndef WS_BORDER
#define WS_BORDER       0x00800000L
#endif
#ifndef WS_TABSTOP
#define WS_TABSTOP      0x00010000L
#endif

// --- Scrollbar constants ---
#ifndef SB_LINEUP
#define SB_LINEUP       0
#endif
#ifndef SB_LINEDOWN
#define SB_LINEDOWN     1
#endif
#ifndef SB_PAGEUP
#define SB_PAGEUP       2
#endif
#ifndef SB_PAGEDOWN
#define SB_PAGEDOWN     3
#endif

// --- Window messages (supplement) ---
#ifndef WM_SETFONT
#define WM_SETFONT      0x0030
#endif
#ifndef WM_IME_CHAR
#define WM_IME_CHAR     0x0286
#endif

// --- ImmGetCompositionString: map undecorated name to A-suffix stub ---
#ifndef ImmGetCompositionString
#define ImmGetCompositionString ImmGetCompositionStringA
#endif

// --- SendMessageW / PostMessageW: map to existing SendMessage / PostMessage ---
inline LRESULT SendMessageW(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return SendMessage(hWnd, msg, wParam, lParam);
}

inline BOOL PostMessageW(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return PostMessage(hWnd, msg, wParam, lParam);
}

// --- MCI_SEQ_MAPPER (Windows multimedia constant) ---
#ifndef MCI_SEQ_MAPPER
#define MCI_SEQ_MAPPER 0xFFFF
#endif

// --- GDI types for UIControls.cpp ---
#ifndef HGLOBAL
typedef void* HGLOBAL;
#endif

typedef struct tagRGBQUAD
{
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
} RGBQUAD;

typedef struct tagPALETTEENTRY
{
	BYTE peRed;
	BYTE peGreen;
	BYTE peBlue;
	BYTE peFlags;
} PALETTEENTRY;

typedef struct tagBITMAPINFO
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD bmiColors[1];
} BITMAPINFO;

#ifndef DIB_RGB_COLORS
#define DIB_RGB_COLORS 0
#endif

#ifndef GWL_WNDPROC
#define GWL_WNDPROC (-4)
#endif

#ifndef SB_VERT
#define SB_VERT 1
#endif

#ifndef WM_SYSKEYDOWN
#define WM_SYSKEYDOWN 0x0104
#endif

#ifndef WM_IME_NOTIFY
#define WM_IME_NOTIFY 0x0282
#endif

#ifndef WM_ERASEBKGND
#define WM_ERASEBKGND 0x0014
#endif

#ifndef IMN_SETOPENSTATUS
#define IMN_SETOPENSTATUS 0x0008
#endif

#ifndef CFS_FORCE_POSITION
#define CFS_FORCE_POSITION 0x0020
#endif

#ifndef CF_TEXT
#define CF_TEXT 1
#endif

// --- GDI function stubs (with AndroidTextRenderer for text rendering) ---
inline HDC CreateCompatibleDC(HDC hdc)
{
	(void)hdc;
	return (HDC)platform::android_text::CreateDC();
}

inline HGDIOBJ SelectObject(HDC hdc, HGDIOBJ h)
{
	return (HGDIOBJ)platform::android_text::DC_SelectObject(hdc, h);
}

inline BOOL DeleteDC(HDC hdc)
{
	platform::android_text::FreeDC(hdc);
	return TRUE;
}

inline HDC GetDC(HWND hWnd)
{
	(void)hWnd;
	// Return a non-null DC so callers like CUIRenderTextOriginal::Create() succeed.
	// This DC is temporary — the caller typically uses it only to create a compatible DC.
	return (HDC)platform::android_text::CreateDC();
}

inline int ReleaseDC(HWND hWnd, HDC hDC)
{
	(void)hWnd;
	platform::android_text::FreeDC(hDC);
	return 1;
}

inline COLORREF SetBkColor(HDC hdc, COLORREF color)
{
	platform::android_text::DC_SetBkColor(hdc, color);
	return 0;
}

inline COLORREF SetTextColor(HDC hdc, COLORREF color)
{
	platform::android_text::DC_SetTextColor(hdc, color);
	return 0;
}

inline BOOL GetCaretPos(LPPOINT lpPoint)
{
	(void)lpPoint;
	if (lpPoint) { lpPoint->x = 0; lpPoint->y = 0; }
	return FALSE;
}

inline BOOL IsWindowVisible(HWND hWnd)
{
	(void)hWnd;
	return FALSE;
}

inline int GetWindowText(HWND hWnd, LPSTR lpString, int nMaxCount)
{
	(void)hWnd;
	if (lpString && nMaxCount > 0) lpString[0] = '\0';
	return 0;
}

inline int GetWindowTextW(HWND hWnd, LPWSTR lpString, int nMaxCount)
{
	(void)hWnd;
	if (lpString && nMaxCount > 0) lpString[0] = L'\0';
	return 0;
}

inline BOOL SetWindowTextW(HWND hWnd, LPCWSTR lpString)
{
	(void)hWnd; (void)lpString;
	return FALSE;
}

inline LRESULT CallWindowProcW(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	(void)lpPrevWndFunc; (void)hWnd; (void)Msg; (void)wParam; (void)lParam;
	return 0;
}

inline HBITMAP CreateDIBSection(HDC hdc, const BITMAPINFO* pbmi, UINT usage, void** ppvBits, HANDLE hSection, DWORD offset)
{
	(void)hdc; (void)usage; (void)hSection; (void)offset;
	if (ppvBits)
	{
		if (pbmi)
		{
			int width = pbmi->bmiHeader.biWidth;
			int height = pbmi->bmiHeader.biHeight < 0 ? -pbmi->bmiHeader.biHeight : pbmi->bmiHeader.biHeight;
			int bpp = pbmi->bmiHeader.biBitCount;
			if (width > 0 && height > 0 && bpp > 0)
			{
				int pitch = ((width * bpp + 31) / 32) * 4;
				size_t bufferSize = (size_t)pitch * height;
				*ppvBits = calloc(1, bufferSize);
				if (*ppvBits)
				{
					// Register the DIB so SelectObject can associate it with a DC
					platform::android_text::RegisterDIB(*ppvBits, width, height, bpp);
					return (HBITMAP)*ppvBits;
				}
			}
		}
		*ppvBits = nullptr;
	}
	return (HBITMAP)nullptr;
}

// --- Clipboard function stubs ---
inline BOOL OpenClipboard(HWND hWndNewOwner)
{
	(void)hWndNewOwner;
	return FALSE;
}

inline HANDLE GetClipboardData(UINT uFormat)
{
	(void)uFormat;
	return nullptr;
}

inline BOOL CloseClipboard()
{
	return FALSE;
}

inline LPVOID GlobalLock(HGLOBAL hMem)
{
	(void)hMem;
	return nullptr;
}

inline BOOL GlobalUnlock(HGLOBAL hMem)
{
	(void)hMem;
	return FALSE;
}

// --- IME function stubs ---
inline BOOL ImmGetCompositionWindow(HIMC hIMC, LPCOMPOSITIONFORM lpCompForm)
{
	(void)hIMC; (void)lpCompForm;
	return FALSE;
}

// --- Scrollbar function stubs ---
inline int GetScrollPos(HWND hWnd, int nBar)
{
	(void)hWnd; (void)nBar;
	return 0;
}

inline int SetScrollPos(HWND hWnd, int nBar, int nPos, BOOL bRedraw)
{
	(void)hWnd; (void)nBar; (void)nPos; (void)bRedraw;
	return 0;
}

// --- Win32 TCHAR-generic macros for file APIs ---
#ifndef GetFileAttributes
#define GetFileAttributes GetFileAttributesA
#endif

#ifndef GetModuleFileName
#define GetModuleFileName GetModuleFileNameA
#endif

// --- CreateDirectory / DeleteFile / CopyFile (POSIX replacements) ---
#include <sys/stat.h>
inline BOOL CreateDirectoryA(const char* lpPathName, void* lpSecurityAttributes)
{
	(void)lpSecurityAttributes;
	if (!lpPathName) return FALSE;
	return (mkdir(lpPathName, 0755) == 0 || errno == EEXIST) ? TRUE : FALSE;
}
#ifndef CreateDirectory
#define CreateDirectory CreateDirectoryA
#endif

inline BOOL DeleteFileA(const char* lpFileName)
{
	if (!lpFileName) return FALSE;
	return (remove(lpFileName) == 0) ? TRUE : FALSE;
}
#ifndef DeleteFile
#define DeleteFile DeleteFileA
#endif

// --- URLDownloadToFile (no-op on Android — shop banner downloads disabled) ---
#ifndef S_OK
#define S_OK 0L
#endif
inline long URLDownloadToFile(void* pCaller, const char* szURL, const char* szFileName, DWORD dwReserved, void* lpfnCB)
{
	(void)pCaller; (void)szURL; (void)szFileName; (void)dwReserved; (void)lpfnCB;
	return S_OK;
}

// --- GetCommandLine (returns empty string on Android) ---
inline const char* GetCommandLineA() { return ""; }
#ifndef GetCommandLine
#define GetCommandLine GetCommandLineA
#endif

// --- GetLastError (Win32 error code) ---
#ifndef _WIN32_GETLASTERROR_DEFINED
#define _WIN32_GETLASTERROR_DEFINED
inline DWORD GetLastError() { return 0; }
#endif

// --- Win32 thread synchronization constants ---
#ifndef INFINITE
#define INFINITE 0xFFFFFFFF
#endif
#ifndef WAIT_TIMEOUT
#define WAIT_TIMEOUT 0x00000102L
#endif
#ifndef WAIT_OBJECT_0
#define WAIT_OBJECT_0 0x00000000L
#endif

inline DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds)
{
	(void)hHandle; (void)dwMilliseconds;
	return WAIT_OBJECT_0; // Always "signaled" since _beginthreadex runs synchronously
}

// --- _beginthreadex (MSVC threading — no-op on Android) ---
#include <pthread.h>
typedef unsigned int (__stdcall *_beginthreadex_proc_type)(void*);
inline uintptr_t _beginthreadex(void* security, unsigned stack_size,
	_beginthreadex_proc_type start_address, void* arglist,
	unsigned initflag, unsigned int* thrdaddr)
{
	(void)security; (void)stack_size; (void)initflag;
	pthread_t tid;
	struct ThreadWrapper {
		_beginthreadex_proc_type func;
		void* arg;
	};
	// Simple synchronous fallback — shop downloads are non-functional on Android anyway
	if (start_address) start_address(arglist);
	if (thrdaddr) *thrdaddr = 0;
	return 0; // Returns 0 = INVALID_HANDLE_VALUE-equivalent for this use case
}

// --- process.h replacement (provide _beginthreadex) ---
#ifndef _INC_PROCESS
#define _INC_PROCESS
#endif

// Anti-cheat/hackshield stubs (hanguo_check family)
inline void hanguo_check3() {}

#endif

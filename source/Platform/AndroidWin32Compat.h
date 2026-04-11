#pragma once

#if defined(__ANDROID__)

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <wchar.h>

#ifndef __int8
#define __int8 char
#endif

#ifndef __int16
#define __int16 short
#endif

#ifndef __int32
#define __int32 int
#endif

#ifndef __int64
#define __int64 long long
#endif

#ifndef WINAPI
#define WINAPI
#endif

#ifndef CALLBACK
#define CALLBACK
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

#ifndef PASCAL
#define PASCAL
#endif

#ifndef FAR
#define FAR
#endif

#ifndef CONST
#define CONST const
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef __stdcall
#define __stdcall
#endif

#ifndef __cdecl
#define __cdecl
#endif

#ifndef __forceinline
#define __forceinline inline
#endif

typedef int BOOL;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef int16_t SHORT;
typedef uint32_t UINT;
typedef intptr_t INT_PTR;
typedef uintptr_t UINT_PTR;
typedef intptr_t LONG_PTR;
typedef uintptr_t DWORD_PTR;
typedef wchar_t WCHAR;
typedef char CHAR;
typedef float FLOAT;
typedef int INT;
typedef void VOID;
typedef void* HANDLE;
typedef void* HWND;
typedef void* HINSTANCE;
typedef void* HDC;
typedef void* HGLRC;
typedef void* HGDIOBJ;
typedef void* HBITMAP;
typedef void* HICON;
typedef void* HCURSOR;
typedef void* HMENU;
typedef void* HFONT;
typedef void* HMODULE;
typedef void* FARPROC;
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef BYTE* LPBYTE;
typedef BYTE* PBYTE;
typedef WORD* LPWORD;
typedef DWORD* LPDWORD;
typedef void* PVOID;
typedef CHAR* LPSTR;
typedef const CHAR* LPCSTR;
typedef WCHAR* LPWSTR;
typedef const WCHAR* LPCWSTR;
typedef CHAR TCHAR;
typedef CHAR* LPTSTR;
typedef const CHAR* LPCTSTR;
typedef LONG HRESULT;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
typedef DWORD COLORREF;
typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;
typedef int SOCKET;
typedef void* LPOVERLAPPED;
typedef pthread_mutex_t CRITICAL_SECTION;

typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct tagPOINT
{
	LONG x;
	LONG y;
} POINT, *LPPOINT;

typedef struct tagSIZE
{
	LONG cx;
	LONG cy;
} SIZE, *LPSIZE;

typedef struct tagRECT
{
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} RECT, *LPRECT;

typedef struct tagFILETIME
{
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} FILETIME, *LPFILETIME;

typedef union _LARGE_INTEGER
{
	struct
	{
		DWORD LowPart;
		LONG HighPart;
	};
	long long QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

#pragma pack(push, 2)
typedef struct tagBITMAPFILEHEADER
{
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER;
#pragma pack(pop)

typedef struct tagBITMAPINFOHEADER
{
	DWORD biSize;
	LONG biWidth;
	LONG biHeight;
	WORD biPlanes;
	WORD biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG biXPelsPerMeter;
	LONG biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER;

#ifndef BI_RGB
#define BI_RGB 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef CP_ACP
#define CP_ACP 0
#endif

#ifndef CP_UTF8
#define CP_UTF8 65001
#endif

#ifndef _MAX_PATH
#define _MAX_PATH MAX_PATH
#endif

#ifndef TEXT
#define TEXT(x) x
#endif

#ifndef _T
#define _T(x) x
#endif

#ifndef _MAX_DRIVE
#define _MAX_DRIVE 3
#endif

#ifndef _MAX_DIR
#define _MAX_DIR 256
#endif

#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif

#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#ifndef WSAEWOULDBLOCK
#define WSAEWOULDBLOCK EWOULDBLOCK
#endif

#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#endif

#ifndef MB_OK
#define MB_OK 0x00000000L
#endif

#ifndef MB_YESNO
#define MB_YESNO 0x00000004L
#endif

#ifndef MB_ICONERROR
#define MB_ICONERROR 0x00000010L
#endif

#ifndef MB_ICONEXCLAMATION
#define MB_ICONEXCLAMATION 0x00000030L
#endif

#ifndef MB_ICONINFORMATION
#define MB_ICONINFORMATION 0x00000040L
#endif

#ifndef WM_CLOSE
#define WM_CLOSE 0x0010
#endif

#ifndef WM_DESTROY
#define WM_DESTROY 0x0002
#endif

#ifndef IDOK
#define IDOK 1
#endif

#ifndef IDYES
#define IDYES 6
#endif

#ifndef SWP_NOMOVE
#define SWP_NOMOVE 0x0002
#endif

#ifndef SWP_NOZORDER
#define SWP_NOZORDER 0x0004
#endif

#ifndef SW_HIDE
#define SW_HIDE 0
#endif

#ifndef SW_SHOW
#define SW_SHOW 5
#endif

#ifndef SW_NORMAL
#define SW_NORMAL 1
#endif

#ifndef GWL_USERDATA
#define GWL_USERDATA (-21)
#endif

#ifndef FW_BOLD
#define FW_BOLD 700
#endif

#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET 1
#endif

#ifndef OUT_DEFAULT_PRECIS
#define OUT_DEFAULT_PRECIS 0
#endif

#ifndef CLIP_DEFAULT_PRECIS
#define CLIP_DEFAULT_PRECIS 0
#endif

#ifndef NONANTIALIASED_QUALITY
#define NONANTIALIASED_QUALITY 3
#endif

#ifndef DEFAULT_PITCH
#define DEFAULT_PITCH 0
#endif

#ifndef FF_DONTCARE
#define FF_DONTCARE 0
#endif

#ifndef VK_LBUTTON
#define VK_LBUTTON 0x01
#endif
#ifndef VK_RBUTTON
#define VK_RBUTTON 0x02
#endif
#ifndef VK_MBUTTON
#define VK_MBUTTON 0x04
#endif
#ifndef VK_BACK
#define VK_BACK 0x08
#endif
#ifndef VK_TAB
#define VK_TAB 0x09
#endif
#ifndef VK_RETURN
#define VK_RETURN 0x0D
#endif
#ifndef VK_SHIFT
#define VK_SHIFT 0x10
#endif
#ifndef VK_CONTROL
#define VK_CONTROL 0x11
#endif
#ifndef VK_MENU
#define VK_MENU 0x12
#endif
#ifndef VK_ESCAPE
#define VK_ESCAPE 0x1B
#endif
#ifndef VK_SPACE
#define VK_SPACE 0x20
#endif
#ifndef VK_LEFT
#define VK_LEFT 0x25
#endif
#ifndef VK_UP
#define VK_UP 0x26
#endif
#ifndef VK_RIGHT
#define VK_RIGHT 0x27
#endif
#ifndef VK_DOWN
#define VK_DOWN 0x28
#endif
#ifndef VK_PRIOR
#define VK_PRIOR 0x21
#endif
#ifndef VK_NEXT
#define VK_NEXT 0x22
#endif
#ifndef VK_END
#define VK_END 0x23
#endif
#ifndef VK_HOME
#define VK_HOME 0x24
#endif
#ifndef VK_SNAPSHOT
#define VK_SNAPSHOT 0x2C
#endif
#ifndef VK_DELETE
#define VK_DELETE 0x2E
#endif
#ifndef VK_INSERT
#define VK_INSERT 0x2D
#endif
#ifndef VK_NUMPAD0
#define VK_NUMPAD0 0x60
#endif
#ifndef VK_F1
#define VK_F1 0x70
#endif
#ifndef VK_F2
#define VK_F2 0x71
#endif
#ifndef VK_F3
#define VK_F3 0x72
#endif
#ifndef VK_F4
#define VK_F4 0x73
#endif
#ifndef VK_F5
#define VK_F5 0x74
#endif
#ifndef VK_F6
#define VK_F6 0x75
#endif
#ifndef VK_F7
#define VK_F7 0x76
#endif
#ifndef VK_F8
#define VK_F8 0x77
#endif
#ifndef VK_F9
#define VK_F9 0x78
#endif
#ifndef VK_F10
#define VK_F10 0x79
#endif
#ifndef VK_F11
#define VK_F11 0x7A
#endif
#ifndef VK_F12
#define VK_F12 0x7B
#endif
#ifndef VK_F13
#define VK_F13 0x7C
#endif
#ifndef VK_F14
#define VK_F14 0x7D
#endif
#ifndef VK_F15
#define VK_F15 0x7E
#endif

#ifndef WS_CHILD
#define WS_CHILD 0x40000000L
#endif
#ifndef WS_VISIBLE
#define WS_VISIBLE 0x10000000L
#endif
#ifndef WS_VSCROLL
#define WS_VSCROLL 0x00200000L
#endif
#ifndef ES_LEFT
#define ES_LEFT 0x0000L
#endif
#ifndef ES_CENTER
#define ES_CENTER 0x0001L
#endif
#ifndef ES_RIGHT
#define ES_RIGHT 0x0002L
#endif
#ifndef ES_MULTILINE
#define ES_MULTILINE 0x0004L
#endif
#ifndef ES_PASSWORD
#define ES_PASSWORD 0x0020L
#endif
#ifndef ES_AUTOHSCROLL
#define ES_AUTOHSCROLL 0x0080L
#endif
#ifndef ES_READONLY
#define ES_READONLY 0x0800L
#endif
#ifndef ES_AUTOVSCROLL
#define ES_AUTOVSCROLL 0x0040L
#endif

#ifndef RGB
#define RGB(r, g, b) ((COLORREF)(((BYTE)(r)) | ((WORD)((BYTE)(g)) << 8) | (((DWORD)(BYTE)(b)) << 16)))
#endif

#ifndef RGBA
#define RGBA(r, g, b, a) ((((DWORD)(a)) << 24) | ((DWORD)(r)) | (((DWORD)(g)) << 8) | (((DWORD)(b)) << 16))
#endif

#ifndef LOBYTE
#define LOBYTE(w) ((BYTE)((DWORD_PTR)(w) & 0xFF))
#endif
#ifndef HIBYTE
#define HIBYTE(w) ((BYTE)((DWORD_PTR)(w) >> 8))
#endif
#ifndef LOWORD
#define LOWORD(l) ((WORD)((DWORD_PTR)(l) & 0xFFFF))
#endif
#ifndef HIWORD
#define HIWORD(l) ((WORD)(((DWORD_PTR)(l) >> 16) & 0xFFFF))
#endif
#ifndef MAKEWORD
#define MAKEWORD(a, b) ((WORD)(((BYTE)((DWORD_PTR)(a) & 0xFF)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xFF))) << 8))
#endif
#ifndef MAKELONG
#define MAKELONG(a, b) ((LONG)(((WORD)((DWORD_PTR)(a) & 0xFFFF)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xFFFF))) << 16))
#endif

#ifndef GL_CLAMP
#define GL_CLAMP GL_CLAMP_TO_EDGE
#endif

#ifndef _countof
#define _countof(array) (sizeof(array) / sizeof((array)[0]))
#endif

#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef strcmpi
#define strcmpi strcasecmp
#endif
#ifndef _stricmp
#define _stricmp strcasecmp
#endif
#ifndef _strcmpi
#define _strcmpi strcasecmp
#endif
#ifndef _strnicmp
#define _strnicmp strncasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif

#ifndef _ASSERT
#define _ASSERT(expression) assert(expression)
#endif

#include <ctype.h>
inline char* _strupr(char* str)
{
	if (str != NULL)
	{
		for (char* p = str; *p; ++p)
		{
			*p = (char)toupper((unsigned char)*p);
		}
	}
	return str;
}

inline void ZeroMemory(void* destination, size_t size)
{
	memset(destination, 0, size);
}

inline void CopyMemory(void* destination, const void* source, size_t size)
{
	memcpy(destination, source, size);
}

inline void MoveMemory(void* destination, const void* source, size_t size)
{
	memmove(destination, source, size);
}

inline void FillMemory(void* destination, size_t size, int value)
{
	memset(destination, value, size);
}

inline BOOL SetRect(RECT* rect, int left, int top, int right, int bottom)
{
	if (rect == NULL)
	{
		return FALSE;
	}
	rect->left = left;
	rect->top = top;
	rect->right = right;
	rect->bottom = bottom;
	return TRUE;
}

inline BOOL OffsetRect(RECT* rect, int dx, int dy)
{
	if (rect == NULL)
	{
		return FALSE;
	}
	rect->left += dx;
	rect->right += dx;
	rect->top += dy;
	rect->bottom += dy;
	return TRUE;
}

inline BOOL PtInRect(const RECT* rect, POINT point)
{
	if (rect == NULL)
	{
		return FALSE;
	}
	return point.x >= rect->left && point.x < rect->right && point.y >= rect->top && point.y < rect->bottom;
}

inline BOOL IsBadReadPtr(const void* pointer, UINT_PTR size)
{
	(void)size;
	return pointer == NULL ? TRUE : FALSE;
}

inline BOOL IntersectRect(RECT* destination, const RECT* source1, const RECT* source2)
{
	if (destination == NULL || source1 == NULL || source2 == NULL)
	{
		return FALSE;
	}

	destination->left = source1->left > source2->left ? source1->left : source2->left;
	destination->top = source1->top > source2->top ? source1->top : source2->top;
	destination->right = source1->right < source2->right ? source1->right : source2->right;
	destination->bottom = source1->bottom < source2->bottom ? source1->bottom : source2->bottom;

	if (destination->left < destination->right && destination->top < destination->bottom)
	{
		return TRUE;
	}

	SetRect(destination, 0, 0, 0, 0);
	return FALSE;
}

inline void InitializeCriticalSection(CRITICAL_SECTION* critical_section)
{
	if (critical_section != NULL)
	{
		pthread_mutex_init(critical_section, NULL);
	}
}

inline void DeleteCriticalSection(CRITICAL_SECTION* critical_section)
{
	if (critical_section != NULL)
	{
		pthread_mutex_destroy(critical_section);
	}
}

inline void EnterCriticalSection(CRITICAL_SECTION* critical_section)
{
	if (critical_section != NULL)
	{
		pthread_mutex_lock(critical_section);
	}
}

inline void LeaveCriticalSection(CRITICAL_SECTION* critical_section)
{
	if (critical_section != NULL)
	{
		pthread_mutex_unlock(critical_section);
	}
}

inline int lstrlenA(LPCSTR text)
{
	return text != NULL ? static_cast<int>(strlen(text)) : 0;
}

inline int lstrlenW(LPCWSTR text)
{
	return text != NULL ? static_cast<int>(wcslen(text)) : 0;
}

#ifndef lstrlen
#define lstrlen lstrlenA
#endif

char* itoa(int value, char* buffer, int radix);
int wsprintfA(char* buffer, const char* format, ...);
int wsprintfW(wchar_t* buffer, const wchar_t* format, ...);

#ifndef wsprintf
#define wsprintf wsprintfA
#endif

DWORD GetTickCount(void);
void Sleep(DWORD milliseconds);
DWORD GetDoubleClickTime(void);
BOOL GetCursorPos(LPPOINT point);
BOOL ScreenToClient(HWND window, LPPOINT point);
HWND GetActiveWindow(void);
HWND GetFocus(void);
void* CreateWindowW(const wchar_t* class_name, const wchar_t* window_name, DWORD style, int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE instance, LPVOID parameter);
BOOL SetWindowPos(HWND window, HWND insert_after, int x, int y, int width, int height, UINT flags);
LONG_PTR GetWindowLongW(HWND window, int index);
LONG_PTR SetWindowLongW(HWND window, int index, LONG_PTR value);
int MessageBoxA(HWND window, const char* text, const char* caption, unsigned int type);
int MessageBoxW(HWND window, const wchar_t* text, const wchar_t* caption, unsigned int type);
HINSTANCE ShellExecuteA(HWND window, const char* operation, const char* file, const char* parameters, const char* directory, int show_command);
void ExitProcess(UINT exit_code);
BOOL PostMessage(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
LRESULT SendMessage(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
DWORD GetFileAttributesA(const char* path);
DWORD GetCurrentDirectoryA(DWORD buffer_length, char* buffer);
DWORD GetModuleFileNameA(HMODULE module, char* buffer, DWORD buffer_length);
DWORD GetPrivateProfileStringA(const char* section, const char* key, const char* default_value, char* returned_string, DWORD returned_string_size, const char* file_name);
BOOL GetTextExtentPointA(HDC hdc, LPCSTR text, int length, LPSIZE size);
BOOL GetTextExtentPoint32A(HDC hdc, LPCSTR text, int length, LPSIZE size);
BOOL TextOutA(HDC hdc, int x, int y, LPCSTR text, int length);
HFONT CreateFontA(int height, int width, int escapement, int orientation, int weight, DWORD italic, DWORD underline, DWORD strike_out, DWORD char_set, DWORD output_precision, DWORD clip_precision, DWORD quality, DWORD pitch_and_family, const char* face_name);
BOOL DeleteObject(HGDIOBJ object);
int WSAGetLastError(void);
void _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext);

#ifndef GetPrivateProfileString
#define GetPrivateProfileString GetPrivateProfileStringA
#endif

#ifndef CreateFont
#define CreateFont CreateFontA
#endif

#ifndef MessageBox
#define MessageBox MessageBoxA
#endif

#ifndef ShellExecute
#define ShellExecute ShellExecuteA
#endif

#ifndef GetCurrentDirectory
#define GetCurrentDirectory GetCurrentDirectoryA
#endif

// Win32 file API constants
#ifndef GENERIC_READ
#define GENERIC_READ 0x80000000L
#endif
#ifndef GENERIC_WRITE
#define GENERIC_WRITE 0x40000000L
#endif
#ifndef OPEN_EXISTING
#define OPEN_EXISTING 3
#endif
#ifndef FILE_ATTRIBUTE_NORMAL
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#endif
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#endif

// CreateFile stub — always returns INVALID_HANDLE_VALUE on Android
inline HANDLE CreateFileA(const char*, DWORD, DWORD, void*, DWORD, DWORD, HANDLE)
{
    return INVALID_HANDLE_VALUE;
}
inline BOOL CloseHandle(HANDLE) { return TRUE; }

#ifndef CreateFile
#define CreateFile CreateFileA
#endif

// MSVC sprintf_s: map to snprintf, adding sizeof(buf) as size.
// For the 3-arg form (sprintf_s(buf, sizeof(buf), fmt, ...)),
// callers must be changed to just sprintf(buf, fmt, ...) or use snprintf directly.
#ifndef sprintf_s
#define sprintf_s(buf, ...) snprintf(buf, sizeof(buf), __VA_ARGS__)
#endif

#ifndef _countof
#define _countof(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef strcpy_s
inline int strcpy_s(char* dest, size_t dest_size, const char* src)
{
	if (dest == NULL || dest_size == 0) return -1;
	strncpy(dest, src ? src : "", dest_size - 1);
	dest[dest_size - 1] = '\0';
	return 0;
}
// MSVC 2-arg template overload: strcpy_s(charArray, src) deduces size from array
template<size_t N>
inline int strcpy_s(char (&dest)[N], const char* src)
{
	return strcpy_s(dest, N, src);
}
#endif

#ifndef strcat_s
inline int strcat_s(char* dest, size_t dest_size, const char* src)
{
	if (dest == NULL || dest_size == 0) return -1;
	size_t len = strlen(dest);
	if (len < dest_size - 1)
	{
		strncat(dest, src ? src : "", dest_size - len - 1);
	}
	return 0;
}
inline int strcat_s(char* dest, const char* src)
{
	return strcat_s(dest, strlen(dest) + strlen(src ? src : "") + 1, src);
}
#endif

#ifndef wvsprintf
#define wvsprintf(buf, fmt, args) vsnprintf(buf, 1024, fmt, args)
#endif

#ifndef _snprintf_s
#define _snprintf_s(buf, size, count, ...) snprintf(buf, size, __VA_ARGS__)
#endif

namespace platform
{
	void SetLegacyClientAppDataRoot(const char* absolute_root_path);
	const char* GetLegacyClientAppDataRoot();
	void SetLegacyClientCursorPosition(long x, long y);
	void SetLegacyClientDoubleClickTimeMs(unsigned int milliseconds);
}

#endif

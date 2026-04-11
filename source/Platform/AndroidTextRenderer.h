#pragma once
#ifdef __ANDROID__

#include <cstddef>

namespace platform {
namespace android_text {

// Initialize text renderer — loads system fonts. Safe to call multiple times.
void Init();

// Font management
void* CreateFontHandle(int pixel_height, int weight, const char* face_name);
void  DeleteFontHandle(void* font_handle);
bool  IsFontHandle(void* handle);

// DC management
void* CreateDC();
void  FreeDC(void* dc);

// DC operations — SelectObject returns previous object of same type
void* DC_SelectObject(void* dc, void* gdi_object);
void  DC_SetTextColor(void* dc, unsigned int colorref);
void  DC_SetBkColor(void* dc, unsigned int colorref);

// Register a DIB section so SelectObject can associate it with a DC
void RegisterDIB(void* bitmap_handle, int width, int height, int bpp);
bool IsDIB(void* handle);

// Text rendering — renders white-on-black text into the DC's selected bitmap
bool TextOut(void* dc, int x, int y, const wchar_t* text, int len);

// Text measurement — out_cx/out_cy are int32_t* (matching Win32 LONG/SIZE on Android)
bool GetTextExtent(void* dc, const wchar_t* text, int len, int* out_cx, int* out_cy);

} // namespace android_text
} // namespace platform

#endif // __ANDROID__

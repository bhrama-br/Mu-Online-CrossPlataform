#ifdef __ANDROID__

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>
#include <android/log.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include <stb_truetype.h>

#include "Platform/AndroidTextRenderer.h"

#define LOG_TAG "MUAndroidText"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace {

// ---------------------------------------------------------------------------
// Font data management
// ---------------------------------------------------------------------------

struct FontData {
	unsigned char* ttf_buffer;
	size_t ttf_size;
	stbtt_fontinfo info;
	bool valid;
};

struct AndroidFont {
	FontData* data;       // Shared TTF data (regular or bold)
	float scale;          // stbtt_ScaleForPixelHeight result
	int pixel_height;     // Requested height
	int ascent;           // Scaled ascent
	int descent;          // Scaled descent
	int line_gap;         // Scaled line gap
};

struct DIBInfo {
	int width;
	int height;
	int bpp;
	int pitch;
};

struct AndroidDC {
	unsigned char* bitmap_buffer;
	int bmp_width;
	int bmp_height;
	int bmp_bpp;
	int bmp_pitch;
	AndroidFont* font;
	unsigned int text_color;  // COLORREF (R | G<<8 | B<<16)
	unsigned int bk_color;
};

// Global font data (regular + bold variants)
FontData g_font_regular = {};
FontData g_font_bold = {};
bool g_initialized = false;

// Track created fonts and DIBs for type identification
std::unordered_set<void*> g_fonts;
std::unordered_map<void*, DIBInfo> g_dibs;

// Default font used when no proper font is selected
AndroidFont g_default_font = {};

// ---------------------------------------------------------------------------
// Load a TTF file from disk
// ---------------------------------------------------------------------------
bool LoadFontFile(const char* path, FontData* out)
{
	FILE* f = fopen(path, "rb");
	if (!f) return false;

	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (sz <= 0 || sz > 16 * 1024 * 1024) {
		fclose(f);
		return false;
	}

	out->ttf_buffer = (unsigned char*)malloc(sz);
	if (!out->ttf_buffer) {
		fclose(f);
		return false;
	}
	out->ttf_size = sz;
	size_t read = fread(out->ttf_buffer, 1, sz, f);
	fclose(f);

	if ((long)read != sz) {
		free(out->ttf_buffer);
		out->ttf_buffer = nullptr;
		return false;
	}

	if (!stbtt_InitFont(&out->info, out->ttf_buffer, stbtt_GetFontOffsetForIndex(out->ttf_buffer, 0))) {
		free(out->ttf_buffer);
		out->ttf_buffer = nullptr;
		return false;
	}

	out->valid = true;
	return true;
}

// ---------------------------------------------------------------------------
// Try loading fonts from common Android system paths
// ---------------------------------------------------------------------------
void TryLoadSystemFonts()
{
	// Regular font — try in order of preference
	static const char* regular_paths[] = {
		"/system/fonts/Roboto-Regular.ttf",
		"/system/fonts/DroidSans.ttf",
		"/system/fonts/NotoSans-Regular.ttf",
		nullptr
	};

	for (const char** p = regular_paths; *p; ++p) {
		if (LoadFontFile(*p, &g_font_regular)) {
			LOGI("Loaded regular font: %s", *p);
			break;
		}
	}

	if (!g_font_regular.valid) {
		LOGE("Failed to load any regular system font!");
	}

	// Bold font
	static const char* bold_paths[] = {
		"/system/fonts/Roboto-Bold.ttf",
		"/system/fonts/DroidSans-Bold.ttf",
		"/system/fonts/NotoSans-Bold.ttf",
		nullptr
	};

	for (const char** p = bold_paths; *p; ++p) {
		if (LoadFontFile(*p, &g_font_bold)) {
			LOGI("Loaded bold font: %s", *p);
			break;
		}
	}

	// Fall back to regular for bold if needed
	if (!g_font_bold.valid && g_font_regular.valid) {
		g_font_bold = g_font_regular;
		LOGI("Using regular font as bold fallback");
	}
}

// ---------------------------------------------------------------------------
// Create an AndroidFont from FontData at a specific pixel height
// ---------------------------------------------------------------------------
void InitFontAtSize(AndroidFont* af, FontData* data, int pixel_height)
{
	af->data = data;
	af->pixel_height = pixel_height > 0 ? pixel_height : 16;
	af->scale = stbtt_ScaleForPixelHeight(&data->info, (float)af->pixel_height);

	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&data->info, &ascent, &descent, &line_gap);
	af->ascent = (int)(ascent * af->scale + 0.5f);
	af->descent = (int)(descent * af->scale + 0.5f);
	af->line_gap = (int)(line_gap * af->scale + 0.5f);
}

} // anonymous namespace

namespace platform {
namespace android_text {

// ---------------------------------------------------------------------------
// Init
// ---------------------------------------------------------------------------
void Init()
{
	if (g_initialized) return;
	g_initialized = true;

	TryLoadSystemFonts();

	// Create a default font at 16px
	if (g_font_regular.valid) {
		InitFontAtSize(&g_default_font, &g_font_regular, 16);
	}

	LOGI("AndroidTextRenderer initialized");
}

// ---------------------------------------------------------------------------
// Font management
// ---------------------------------------------------------------------------
void* CreateFontHandle(int pixel_height, int weight, const char* face_name)
{
	Init();

	// weight >= 700 = bold (FW_BOLD = 700)
	FontData* data = (weight >= 700 && g_font_bold.valid)
		? &g_font_bold
		: (g_font_regular.valid ? &g_font_regular : nullptr);

	if (!data) {
		// No fonts loaded — return a tagged dummy
		return reinterpret_cast<void*>(1);
	}

	auto* af = new AndroidFont();
	InitFontAtSize(af, data, pixel_height);

	g_fonts.insert(af);
	return af;
}

void DeleteFontHandle(void* font_handle)
{
	if (!font_handle || font_handle == reinterpret_cast<void*>(1)) return;

	auto it = g_fonts.find(font_handle);
	if (it != g_fonts.end()) {
		delete static_cast<AndroidFont*>(font_handle);
		g_fonts.erase(it);
	}
}

bool IsFontHandle(void* handle)
{
	return g_fonts.count(handle) > 0;
}

// ---------------------------------------------------------------------------
// DIB registration
// ---------------------------------------------------------------------------
void RegisterDIB(void* bitmap_handle, int width, int height, int bpp)
{
	if (!bitmap_handle) return;
	DIBInfo info;
	info.width = width;
	info.height = height < 0 ? -height : height;
	info.bpp = bpp;
	info.pitch = ((width * bpp + 31) / 32) * 4;
	g_dibs[bitmap_handle] = info;
}

bool IsDIB(void* handle)
{
	return g_dibs.count(handle) > 0;
}

// ---------------------------------------------------------------------------
// DC management
// ---------------------------------------------------------------------------
void* CreateDC()
{
	Init();

	auto* dc = new AndroidDC();
	dc->bitmap_buffer = nullptr;
	dc->bmp_width = 0;
	dc->bmp_height = 0;
	dc->bmp_bpp = 24;
	dc->bmp_pitch = 0;
	dc->font = &g_default_font;
	dc->text_color = 0x00FFFFFF;  // White
	dc->bk_color = 0x00000000;    // Black
	return dc;
}

void FreeDC(void* dc)
{
	if (!dc) return;
	delete static_cast<AndroidDC*>(dc);
}

// ---------------------------------------------------------------------------
// DC operations
// ---------------------------------------------------------------------------
void* DC_SelectObject(void* dc_handle, void* gdi_object)
{
	if (!dc_handle || !gdi_object) return nullptr;
	auto* dc = static_cast<AndroidDC*>(dc_handle);

	// Check if it's a font
	if (IsFontHandle(gdi_object)) {
		void* old = dc->font;
		dc->font = static_cast<AndroidFont*>(gdi_object);
		return old;
	}

	// Check if it's a registered DIB (bitmap)
	auto it = g_dibs.find(gdi_object);
	if (it != g_dibs.end()) {
		void* old = dc->bitmap_buffer;
		dc->bitmap_buffer = static_cast<unsigned char*>(gdi_object);
		dc->bmp_width = it->second.width;
		dc->bmp_height = it->second.height;
		dc->bmp_bpp = it->second.bpp;
		dc->bmp_pitch = it->second.pitch;
		return old;
	}

	// Unknown GDI object type — might be an old-style dummy font handle
	// Treat (HFONT)1 as the default font
	if (gdi_object == reinterpret_cast<void*>(1)) {
		void* old = dc->font;
		dc->font = &g_default_font;
		return old;
	}

	return nullptr;
}

void DC_SetTextColor(void* dc_handle, unsigned int colorref)
{
	if (!dc_handle) return;
	static_cast<AndroidDC*>(dc_handle)->text_color = colorref;
}

void DC_SetBkColor(void* dc_handle, unsigned int colorref)
{
	if (!dc_handle) return;
	static_cast<AndroidDC*>(dc_handle)->bk_color = colorref;
}

// ---------------------------------------------------------------------------
// Text rendering — renders white-on-black text into DC's bitmap buffer
// ---------------------------------------------------------------------------
bool TextOut(void* dc_handle, int x, int y, const wchar_t* text, int len)
{
	if (!dc_handle || !text || len <= 0) return false;

	auto* dc = static_cast<AndroidDC*>(dc_handle);
	if (!dc->bitmap_buffer || dc->bmp_width <= 0 || dc->bmp_height <= 0) return false;

	AndroidFont* font = dc->font;
	if (!font || !font->data || !font->data->valid) return false;

	const int bpp_bytes = dc->bmp_bpp / 8;  // 3 for 24bpp
	const int pitch = dc->bmp_pitch;
	const int bmp_w = dc->bmp_width;
	const int bmp_h = dc->bmp_height;

	// Clear the region that will be written (black background)
	// We clear the full bitmap height for the text area
	// The original GDI TextOut also overwrites the background
	{
		int clear_h = font->ascent - font->descent + 2;
		if (clear_h > bmp_h) clear_h = bmp_h;
		for (int row = y; row < y + clear_h && row < bmp_h; ++row) {
			if (row < 0) continue;
			memset(dc->bitmap_buffer + row * pitch + x * bpp_bytes, 0,
				(bmp_w - x) * bpp_bytes < pitch ? (bmp_w - x) * bpp_bytes : pitch);
		}
	}

	// Baseline position
	int baseline_y = y + font->ascent;

	float x_pos = (float)x;

	for (int i = 0; i < len; ++i) {
		int codepoint = (int)text[i];
		if (codepoint == 0) break;

		// Get glyph metrics
		int advance, lsb;
		stbtt_GetCodepointHMetrics(&font->data->info, codepoint, &advance, &lsb);

		// Get glyph bitmap
		int gx0, gy0, gx1, gy1;
		stbtt_GetCodepointBitmapBox(&font->data->info, codepoint,
			font->scale, font->scale, &gx0, &gy0, &gx1, &gy1);

		int glyph_w = gx1 - gx0;
		int glyph_h = gy1 - gy0;

		if (glyph_w > 0 && glyph_h > 0 && glyph_w <= 256 && glyph_h <= 256) {
			// Render glyph to temporary buffer
			unsigned char glyph_bitmap[256 * 64];
			int render_w = glyph_w;
			int render_h = glyph_h > 64 ? 64 : glyph_h;

			stbtt_MakeCodepointBitmap(&font->data->info, glyph_bitmap,
				render_w, render_h, render_w,
				font->scale, font->scale, codepoint);

			// Copy glyph into DC bitmap (white on black, 24bpp)
			int dst_x = (int)(x_pos + 0.5f) + gx0;
			int dst_y = baseline_y + gy0;

			for (int gy = 0; gy < render_h; ++gy) {
				int py = dst_y + gy;
				if (py < 0 || py >= bmp_h) continue;

				for (int gx = 0; gx < render_w; ++gx) {
					int px = dst_x + gx;
					if (px < 0 || px >= bmp_w) continue;

					unsigned char alpha = glyph_bitmap[gy * render_w + gx];
					if (alpha == 0) continue;

					// Threshold to binary: the WriteText() function checks for
					// exact == 255 (matching Windows NONANTIALIASED_QUALITY).
					unsigned char val = (alpha >= 80) ? 255 : 0;
					if (val == 0) continue;

					int idx = py * pitch + px * bpp_bytes;

					if (bpp_bytes == 3) {
						// 24bpp BGR — write white where glyph has coverage
						dc->bitmap_buffer[idx + 0] = val;  // B
						dc->bitmap_buffer[idx + 1] = val;  // G
						dc->bitmap_buffer[idx + 2] = val;  // R
					} else if (bpp_bytes == 4) {
						dc->bitmap_buffer[idx + 0] = val;
						dc->bitmap_buffer[idx + 1] = val;
						dc->bitmap_buffer[idx + 2] = val;
						dc->bitmap_buffer[idx + 3] = 255;
					}
				}
			}
		}

		x_pos += advance * font->scale;

		// Kerning
		if (i + 1 < len) {
			int next_cp = (int)text[i + 1];
			int kern = stbtt_GetCodepointKernAdvance(&font->data->info, codepoint, next_cp);
			x_pos += kern * font->scale;
		}
	}

	return true;
}

// ---------------------------------------------------------------------------
// Text measurement
// ---------------------------------------------------------------------------
bool GetTextExtent(void* dc_handle, const wchar_t* text, int len, int* out_cx, int* out_cy)
{
	if (!out_cx || !out_cy) return false;

	// Default fallback
	*out_cx = len * 8;
	*out_cy = 16;

	AndroidFont* font = nullptr;

	if (dc_handle) {
		auto* dc = static_cast<AndroidDC*>(dc_handle);
		font = dc->font;
	}

	if (!font || !font->data || !font->data->valid) {
		return true;  // Return defaults
	}

	if (!text || len <= 0) {
		*out_cx = 0;
		*out_cy = font->ascent - font->descent;
		return true;
	}

	float width = 0;
	for (int i = 0; i < len; ++i) {
		int codepoint = (int)text[i];
		if (codepoint == 0) break;

		int advance, lsb;
		stbtt_GetCodepointHMetrics(&font->data->info, codepoint, &advance, &lsb);
		width += advance * font->scale;

		// Kerning
		if (i + 1 < len) {
			int next_cp = (int)text[i + 1];
			int kern = stbtt_GetCodepointKernAdvance(&font->data->info, codepoint, next_cp);
			width += kern * font->scale;
		}
	}

	*out_cx = (int)(width + 0.5f);
	*out_cy = (int)(font->ascent - font->descent);

	return true;
}

} // namespace android_text
} // namespace platform

#endif // __ANDROID__

#pragma once

#include "Platform/RenderBackend.h"

namespace platform
{
	inline float ClampLegacyCompatibleColorComponent(float value)
	{
		if (value < 0.0f)
		{
			return 0.0f;
		}

		if (value > 1.0f)
		{
			return 1.0f;
		}

		return value;
	}

	inline void SyncLegacyCompatibleCurrentColor(float r, float g, float b, float a)
	{
		GetRenderBackend().SetCurrentColor(
			ClampLegacyCompatibleColorComponent(r),
			ClampLegacyCompatibleColorComponent(g),
			ClampLegacyCompatibleColorComponent(b),
			ClampLegacyCompatibleColorComponent(a));
	}

	inline void LegacyCompatibleColor3f(float r, float g, float b)
	{
		SyncLegacyCompatibleCurrentColor(r, g, b, 1.0f);
#if !defined(__ANDROID__)
		(glColor3f)(r, g, b);
#endif
	}

	inline void LegacyCompatibleColor4f(float r, float g, float b, float a)
	{
		SyncLegacyCompatibleCurrentColor(r, g, b, a);
#if !defined(__ANDROID__)
		(glColor4f)(r, g, b, a);
#endif
	}

	inline void SyncLegacyCompatibleCurrentColor3(const float* rgb)
	{
		if (rgb == NULL)
		{
			return;
		}

		SyncLegacyCompatibleCurrentColor(rgb[0], rgb[1], rgb[2], 1.0f);
	}

	inline void LegacyCompatibleColor3fv(const float* rgb)
	{
		if (rgb == NULL)
		{
			return;
		}

		SyncLegacyCompatibleCurrentColor3(rgb);
#if !defined(__ANDROID__)
		(glColor3fv)(rgb);
#endif
	}

	inline void SyncLegacyCompatibleCurrentColor4(const float* rgba)
	{
		if (rgba == NULL)
		{
			return;
		}

		SyncLegacyCompatibleCurrentColor(rgba[0], rgba[1], rgba[2], rgba[3]);
	}

	inline void LegacyCompatibleColor4fv(const float* rgba)
	{
		if (rgba == NULL)
		{
			return;
		}

		SyncLegacyCompatibleCurrentColor4(rgba);
#if !defined(__ANDROID__)
		(glColor4fv)(rgba);
#endif
	}

	inline void LegacyCompatibleColor3ub(unsigned char r, unsigned char g, unsigned char b)
	{
		SyncLegacyCompatibleCurrentColor((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
#if !defined(__ANDROID__)
		(glColor3ub)(r, g, b);
#endif
	}

	inline void LegacyCompatibleColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		SyncLegacyCompatibleCurrentColor((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f);
#if !defined(__ANDROID__)
		(glColor4ub)(r, g, b, a);
#endif
	}

	inline void LegacyCompatibleColor3ubv(const unsigned char* rgb)
	{
		if (rgb == NULL)
		{
			return;
		}

		SyncLegacyCompatibleCurrentColor((float)rgb[0] / 255.0f, (float)rgb[1] / 255.0f, (float)rgb[2] / 255.0f, 1.0f);
#if !defined(__ANDROID__)
		(glColor3ubv)(rgb);
#endif
	}

	inline void LegacyCompatibleColor4ubv(const unsigned char* rgba)
	{
		if (rgba == NULL)
		{
			return;
		}

		SyncLegacyCompatibleCurrentColor((float)rgba[0] / 255.0f, (float)rgba[1] / 255.0f, (float)rgba[2] / 255.0f, (float)rgba[3] / 255.0f);
#if !defined(__ANDROID__)
		(glColor4ubv)(rgba);
#endif
	}
}

#define glColor3f(r, g, b) do { \
	::platform::LegacyCompatibleColor3f((float)(r), (float)(g), (float)(b)); \
} while (0)

#define glColor4f(r, g, b, a) do { \
	::platform::LegacyCompatibleColor4f((float)(r), (float)(g), (float)(b), (float)(a)); \
} while (0)

#define glColor3fv(v) do { \
	::platform::LegacyCompatibleColor3fv((const float*)(v)); \
} while (0)

#define glColor4fv(v) do { \
	::platform::LegacyCompatibleColor4fv((const float*)(v)); \
} while (0)

#define glColor3ub(r, g, b) do { \
	::platform::LegacyCompatibleColor3ub((unsigned char)(r), (unsigned char)(g), (unsigned char)(b)); \
} while (0)

#define glColor4ub(r, g, b, a) do { \
	::platform::LegacyCompatibleColor4ub((unsigned char)(r), (unsigned char)(g), (unsigned char)(b), (unsigned char)(a)); \
} while (0)

#define glColor3ubv(v) do { \
	::platform::LegacyCompatibleColor3ubv((const unsigned char*)(v)); \
} while (0)

#define glColor4ubv(v) do { \
	::platform::LegacyCompatibleColor4ubv((const unsigned char*)(v)); \
} while (0)

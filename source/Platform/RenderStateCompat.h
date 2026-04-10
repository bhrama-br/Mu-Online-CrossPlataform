#pragma once

#include "Platform/RenderBackend.h"

namespace platform
{
	inline bool TrySetLegacyCompatibleCapability(unsigned int capability, bool enabled)
	{
		switch (capability)
		{
		case GL_TEXTURE_2D:
			GetRenderBackend().SetTextureEnabled(enabled);
			return true;

		case GL_DEPTH_TEST:
			GetRenderBackend().SetDepthTestEnabled(enabled);
			return true;

		case GL_CULL_FACE:
			GetRenderBackend().SetCullFaceEnabled(enabled);
			return true;

#if defined(GL_ALPHA_TEST)
		case GL_ALPHA_TEST:
			GetRenderBackend().SetAlphaTestEnabled(enabled);
			return true;
#endif

#if defined(GL_FOG)
		case GL_FOG:
			GetRenderBackend().SetFogEnabled(enabled);
			return true;
#endif

		default:
			return false;
		}
	}

	inline void LegacyCompatibleEnable(unsigned int capability)
	{
		if (!TrySetLegacyCompatibleCapability(capability, true))
		{
#if !defined(__ANDROID__)
			(glEnable)(capability);
#endif
		}
	}

	inline void LegacyCompatibleDisable(unsigned int capability)
	{
		if (!TrySetLegacyCompatibleCapability(capability, false))
		{
#if !defined(__ANDROID__)
			(glDisable)(capability);
#endif
		}
	}

	inline void LegacyCompatibleDepthFunc(unsigned int func)
	{
		GetRenderBackend().SetDepthFunction(func);
	}
}

#define glEnable(capability) do { \
	::platform::LegacyCompatibleEnable((unsigned int)(capability)); \
} while (0)

#define glDisable(capability) do { \
	::platform::LegacyCompatibleDisable((unsigned int)(capability)); \
} while (0)

#define glDepthFunc(func) do { \
	::platform::LegacyCompatibleDepthFunc((unsigned int)(func)); \
} while (0)

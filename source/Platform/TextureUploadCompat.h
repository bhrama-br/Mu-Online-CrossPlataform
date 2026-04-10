#pragma once

namespace platform
{
	inline unsigned int ResolveCompatibleTextureFormat(int components, unsigned int fallback_format)
	{
		switch (components)
		{
		case 4:
			return GL_RGBA;

		case 3:
			return GL_RGB;

#if defined(GL_LUMINANCE_ALPHA)
		case 2:
			return GL_LUMINANCE_ALPHA;
#endif

#if defined(GL_ALPHA)
		case 1:
			return GL_ALPHA;
#endif

		default:
			break;
		}

		switch (fallback_format)
		{
		case GL_RGB:
		case GL_RGBA:
#if defined(GL_ALPHA)
		case GL_ALPHA:
#endif
#if defined(GL_LUMINANCE)
		case GL_LUMINANCE:
#endif
#if defined(GL_LUMINANCE_ALPHA)
		case GL_LUMINANCE_ALPHA:
#endif
			return fallback_format;

		default:
			return GL_RGBA;
		}
	}

	inline void LegacyCompatibleTexImage2D(unsigned int target, int level, int components, int width, int height, int border, unsigned int fallback_format, unsigned int pixel_type, const void* pixels)
	{
		const unsigned int compatible_format = ResolveCompatibleTextureFormat(components, fallback_format);
		(glTexImage2D)(target, level, compatible_format, width, height, border, compatible_format, pixel_type, pixels);
	}
}

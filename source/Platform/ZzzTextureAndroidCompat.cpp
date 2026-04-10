#include "stdafx.h"

#if defined(__ANDROID__)

#include "ZzzTexture.h"

#include "Platform/GameAssetPath.h"

#include <android/log.h>
#include <cstdio>
#include <cstring>

namespace
{
	const char* kAndroidTextureCompatTag = "MUAndroidTexture";
}

bool WriteJpeg(char* filename, int Width, int Height, unsigned char* Buffer, int quality)
{
	(void)filename;
	(void)Width;
	(void)Height;
	(void)Buffer;
	(void)quality;
	__android_log_print(ANDROID_LOG_WARN, kAndroidTextureCompatTag, "WriteJpeg is not available on Android compat path");
	return false;
}

void SaveImage(int HeaderSize, char* Ext, char* filename, BYTE* PakBuffer, int Size)
{
	(void)HeaderSize;
	(void)Ext;
	(void)filename;
	(void)PakBuffer;
	(void)Size;
}

bool OpenJpegBuffer(char* filename, float* BufferFloat)
{
	(void)filename;
	(void)BufferFloat;
	return false;
}

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
bool LoadBitmap(const char* szFileName, GLuint uiTextureIndex, GLuint uiFilter, GLuint uiWrapMode, bool bCheck, bool bFullPath)
#else
bool LoadBitmap(const char* szFileName, GLuint uiTextureIndex, GLuint uiFilter, GLuint uiWrapMode, bool bCheck)
#endif
{
	if (szFileName == NULL || szFileName[0] == '\0')
	{
		return false;
	}

	char szFullPath[MAX_PATH] = { 0 };
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
	if (bFullPath)
	{
		strncpy(szFullPath, szFileName, sizeof(szFullPath) - 1);
		szFullPath[sizeof(szFullPath) - 1] = '\0';
	}
	else
#endif
	if (!platform::CopyResolvedGameAssetPath(szFullPath, _countof(szFullPath), szFileName))
	{
		__android_log_print(ANDROID_LOG_WARN, kAndroidTextureCompatTag, "LoadBitmap path failed: %s", szFileName);
		return false;
	}

	if (bCheck)
	{
		if (!Bitmaps.LoadImage(uiTextureIndex, szFullPath, uiFilter, uiWrapMode))
		{
			__android_log_print(ANDROID_LOG_WARN, kAndroidTextureCompatTag, "LoadBitmap failed: %s", szFullPath);
			return false;
		}
		return true;
	}

	return Bitmaps.LoadImage(uiTextureIndex, szFullPath, uiFilter, uiWrapMode);
}

void DeleteBitmap(GLuint uiTextureIndex, bool bForce)
{
	Bitmaps.UnloadImage(uiTextureIndex, bForce);
}

void PopUpErrorCheckMsgBox(const char* szErrorMsg, bool bForceDestroy)
{
	(void)bForceDestroy;
	__android_log_print(
		ANDROID_LOG_ERROR,
		kAndroidTextureCompatTag,
		"%s",
		szErrorMsg != NULL ? szErrorMsg : "Texture error");
}

#endif

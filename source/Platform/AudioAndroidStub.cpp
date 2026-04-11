#include "stdafx.h"

#if defined(__ANDROID__)

#include "DSPlaySound.h"

#if !defined(MU_ANDROID_HAS_DSPLAYSOUND_RUNTIME)

HRESULT InitDirectSound(HWND hDlg)
{
	(void)hDlg;
	return 0;
}

void SetEnableSound(bool b)
{
	(void)b;
}

void FreeDirectSound()
{
}

void LoadWaveFile(int Buffer, TCHAR* strFileName, int BufferChannel, bool Enable3DSound)
{
	(void)Buffer;
	(void)strFileName;
	(void)BufferChannel;
	(void)Enable3DSound;
}

HRESULT PlayBuffer(int Buffer, OBJECT* Object, BOOL bLooped)
{
	(void)Buffer;
	(void)Object;
	(void)bLooped;
	return 0;
}

void StopBuffer(int Buffer, BOOL bResetPosition)
{
	(void)Buffer;
	(void)bResetPosition;
}

void AllStopSound(void)
{
}

void Set3DSoundPosition()
{
}

HRESULT ReleaseBuffer(int Buffer)
{
	(void)Buffer;
	return 0;
}

HRESULT RestoreBuffers(int Buffer, int Channel)
{
	(void)Buffer;
	(void)Channel;
	return 0;
}

void SetVolume(int Buffer, long vol)
{
	(void)Buffer;
	(void)vol;
}

void SetMasterVolume(long vol)
{
	(void)vol;
}

void DestroySound()
{
}

void PlayMp3(char* Name, BOOL bEnforce)
{
	(void)Name;
	(void)bEnforce;
}

void StopMp3(char* Name, BOOL bEnforce)
{
	(void)Name;
	(void)bEnforce;
}

bool IsEndMp3()
{
	return true;
}

int GetMp3PlayPosition()
{
	return 0;
}

#endif // !defined(MU_ANDROID_HAS_DSPLAYSOUND_RUNTIME)

#endif

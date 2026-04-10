#include "stdafx.h"

#if defined(__ANDROID__)

#include "./Utilities/Log/ErrorReport.h"

#include <android/log.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace
{
	const char* kAndroidErrorReportTag = "MUAndroidError";
}

CErrorReport g_ErrorReport;

CErrorReport::CErrorReport()
	: m_hFile(NULL)
	, m_iKey(0)
{
	m_lpszFileName[0] = '\0';
#ifdef ASG_ADD_MULTI_CLIENT
	m_nFileCount = 0;
#endif
}

CErrorReport::~CErrorReport()
{
}

void CErrorReport::Clear(void)
{
}

void CErrorReport::Create(char* lpszFileName)
{
	if (lpszFileName != NULL)
	{
		strncpy(m_lpszFileName, lpszFileName, MAX_PATH - 1);
		m_lpszFileName[MAX_PATH - 1] = '\0';
	}
}

void CErrorReport::Destroy(void)
{
}

void CErrorReport::CutHead(void)
{
}

char* CErrorReport::CheckHeadToCut(char* lpszBuffer, DWORD)
{
	return lpszBuffer;
}

BOOL CErrorReport::WriteFile(HANDLE, void*, DWORD, LPDWORD, LPOVERLAPPED)
{
	return TRUE;
}

void CErrorReport::WriteDebugInfoStr(char* lpszToWrite)
{
	if (lpszToWrite != NULL && lpszToWrite[0] != '\0')
	{
		__android_log_print(ANDROID_LOG_INFO, kAndroidErrorReportTag, "%s", lpszToWrite);
	}
}

void CErrorReport::Write(const char* lpszFormat, ...)
{
	if (lpszFormat == NULL)
	{
		return;
	}

	char buffer[2048] = { 0 };
	va_list args;
	va_start(args, lpszFormat);
	vsnprintf(buffer, sizeof(buffer), lpszFormat, args);
	va_end(args);
	__android_log_print(ANDROID_LOG_INFO, kAndroidErrorReportTag, "%s", buffer);
}

void CErrorReport::HexWrite(void* pBuffer, int iSize)
{
	if (pBuffer == NULL || iSize <= 0)
	{
		return;
	}

	unsigned char* bytes = static_cast<unsigned char*>(pBuffer);
	char line[1024] = { 0 };
	size_t offset = 0;
	for (int index = 0; index < iSize && offset + 4 < sizeof(line); ++index)
	{
		offset += static_cast<size_t>(snprintf(line + offset, sizeof(line) - offset, "%02X ", bytes[index]));
	}
	__android_log_print(ANDROID_LOG_INFO, kAndroidErrorReportTag, "%s", line);
}

void CErrorReport::AddSeparator(void)
{
	__android_log_print(ANDROID_LOG_INFO, kAndroidErrorReportTag, "----------------------------------------");
}

void CErrorReport::WriteLogBegin(void)
{
	__android_log_print(ANDROID_LOG_INFO, kAndroidErrorReportTag, "Log begin");
}

void CErrorReport::WriteCurrentTime(BOOL)
{
}

void CErrorReport::WriteSystemInfo(ER_SystemInfo*)
{
}

void CErrorReport::WriteOpenGLInfo(void)
{
}

void CErrorReport::WriteImeInfo(HWND)
{
}

void CErrorReport::WriteSoundCardInfo(void)
{
}

void GetSystemInfo(ER_SystemInfo* si)
{
	if (si == NULL)
	{
		return;
	}

	memset(si, 0, sizeof(ER_SystemInfo));
	strncpy(si->m_lpszCPU, "Android", MAX_LENGTH_CPUNAME - 1);
	strncpy(si->m_lpszOS, "Android", MAX_LENGTH_OSINFO - 1);
	strncpy(si->m_lpszDxVersion, "N/A", MAX_DXVERSION - 1);
}

#endif

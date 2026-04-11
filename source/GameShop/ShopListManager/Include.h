
#pragma once

#define STRSAFE_NO_DEPRECATE

#define SHOPLIST_SCRIPT_COUNT				3

#define SHOPLIST_SCRIPT_CATEGORY			"IBSCategory.txt"
#define SHOPLIST_SCRIPT_PACKAGE				"IBSPackage.txt"
#define SHOPLIST_SCRIPT_PRODUCT				"IBSProduct.txt"
#define BANNER_SCRIPT_FILENAME				"IBSBanner.txt"

#define SHOPLIST_LENGTH_CATEGORYNAME		128

#define SHOPLIST_LENGTH_PACKAGENAME			512
#define SHOPLIST_LENGTH_PACKAGEDESC			2048
#define SHOPLIST_LENGTH_PACKAGECAUTION		1024
#define SHOPLIST_LENGTH_PACKAGECASHNAME		256
#define SHOPLIST_LENGTH_PACKAGEPRICEUNIT	64

#define SHOPLIST_LENGTH_PRODUCTNAME			128
#define SHOPLIST_LENGTH_PRODUCTPROPERTYNAME 128
#define SHOPLIST_LENGTH_PRODUCTVALUE		512
#define SHOPLIST_LENGTH_PRODUCTUNITNAME		64
#define SHOPLIST_LENGTH_INGAMEPACKAGEID		20

#define BANNER_LENGTH_NAME					50

#define ERROR_TIMEOUT_BREAK					0x01
#define ERROR_FILE_SIZE_ZERO				0x02
#define ERROR_CATEGORY_OPEN_FAIL			0x03
#define ERROR_PACKAGE_OPEN_FAIL				0x04
#define ERROR_PRODUCT_OPEN_FAIL				0x05
#define ERROR_BANNER_OPEN_FAIL				0x06
#define ERROR_LOAD_SCRIPT					0x07
#define ERROR_THREAD						0x08

#if !defined(__ANDROID__)
#include <Windows.h>
#include <Wininet.h>
#include <tchar.h>
#include <crtdbg.h>
#include <strsafe.h>
#endif
#include <vector>
#include <string>
#if !defined(__ANDROID__)
#include "GameShop\ShopListManager\interface\WZResult\WZResult.h"
#include "GameShop\ShopListManager\interface\DownloadInfo.h"
#include "GameShop\ShopListManager\interface\FileDownloader.h"
#else
// Android stubs for Windows-only ShopListManager types
#ifndef MAX_ERROR_MESSAGE
#define MAX_ERROR_MESSAGE 1024
#endif

// Error code defines (from ErrorCodeDefine.h)
#define PT_FAILED      0x40000000
#define PT_EXCEPTION   (0x00000001 | PT_FAILED)
#define PT_NO_INFO     (0x00000002 | PT_FAILED)
#define PT_NO_DLL_INFO (0x00000003 | PT_FAILED)
#define PT_LOADLIBRARY (0x00000004 | PT_FAILED)
#define PT_GETPROCADDR (0x00000005 | PT_FAILED)

#define DL_FAILED                    0x20000000
#define DL_EXCEPTION                (0x00000001 | DL_FAILED)
#define DL_NO_INFO                  (0x00000002 | DL_FAILED)
#define DL_BEGIN_THREAD_CONNECTION  (0x00000003 | DL_FAILED)

typedef enum _DownloaderType { FTP, HTTP } DownloaderType;
class WZResult {
public:
	DWORD m_dwErrorCode;
	DWORD m_dwWindowErrorCode;
	char m_szErrorMessage[MAX_ERROR_MESSAGE];
	WZResult() : m_dwErrorCode(0), m_dwWindowErrorCode(0) { m_szErrorMessage[0] = '\0'; }
	BOOL IsSuccess() { return (m_dwErrorCode == 0 && m_dwWindowErrorCode == 0) ? TRUE : FALSE; }
	char* GetErrorMessage() { return m_szErrorMessage; }
	DWORD GetErrorCode() { return m_dwErrorCode; }
	void SetSuccessResult() { m_dwErrorCode = 0; m_dwWindowErrorCode = 0; }
	void BuildSuccessResult() { m_dwErrorCode = 0; m_dwWindowErrorCode = 0; }
	void BuildResult(DWORD ec, DWORD wec, const char* fmt, ...) { m_dwErrorCode = ec; m_dwWindowErrorCode = wec; (void)fmt; }
	void SetResult(DWORD ec, DWORD wec, const char* fmt, ...) { m_dwErrorCode = ec; m_dwWindowErrorCode = wec; (void)fmt; }
};
class DownloadServerInfo {
public:
	char m_szServerURL[INTERNET_MAX_URL_LENGTH];
	DownloaderType m_DownloaderType;
	DownloadServerInfo() : m_DownloaderType(HTTP) { m_szServerURL[0] = '\0'; }
	DownloaderType GetDownloaderType() { return m_DownloaderType; }
	void SetDownloaderType(DownloaderType t) { m_DownloaderType = t; }
	void SetPassiveMode(bool) {}
	void SetOverWrite(int) {}
	void SetConnectTimeout(int) {}
	void SetServerInfo(const char*, unsigned short, const char*, const char*) {}
};
class DownloadFileInfo {
public:
	char m_szRemoteFilePath[INTERNET_MAX_URL_LENGTH];
	DownloadFileInfo() { m_szRemoteFilePath[0] = '\0'; }
	const char* GetRemoteFilePath() { return m_szRemoteFilePath; }
	void SetFilePath(const char*, const char*, const char*, const char*) {}
};
class IDownloaderStateEvent {};
class FileDownloader {
public:
	FileDownloader(IDownloaderStateEvent*, DownloadServerInfo*, DownloadFileInfo*) {}
	void Break() {}
	WZResult DownloadFile() { WZResult r; r.m_dwErrorCode = 1; return r; }
};
#endif

#if !defined (INVALID_FILE_ATTRIBUTES) 
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1) 
#endif

//#ifdef _DEBUG
//	#pragma  comment(lib, "FileDownloader.lib")
//#else
//	#pragma  comment(lib, "FileDownloader.lib")
//#endif	


enum FTP_SERVICE_MODE {FTP_MODE_ACTIVE, FTP_MODE_PASSIVE};
enum FILE_ENCODE
{
	FE_ANSI,
	FE_UTF8,
	FE_UNICODE
};

class CListVersionInfo
{
public:
	unsigned short Zone;
	unsigned short year;
	unsigned short yearId;
};

class CListManagerInfo
{
public:
	DownloaderType		m_DownloaderType;
	std::string			m_strServerIP;
	unsigned short		m_nPortNum;
	std::string			m_strUserID;
	std::string			m_strPWD;
	std::string			m_strRemotePath;
	FTP_SERVICE_MODE	m_ftpMode;
	std::string			m_strLocalPath;
	DWORD				m_dwDownloadMaxTime;

	CListVersionInfo	m_Version;
};


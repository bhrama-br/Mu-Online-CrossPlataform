#pragma once

#if defined(__ANDROID__)

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

#include "Platform/AndroidWin32Compat.h"

#ifndef SD_SEND
#define SD_SEND SHUT_WR
#endif

#ifndef SD_RECEIVE
#define SD_RECEIVE SHUT_RD
#endif

#ifndef SD_BOTH
#define SD_BOTH SHUT_RDWR
#endif

#ifndef INADDR_NONE
#define INADDR_NONE ((unsigned long)0xFFFFFFFF)
#endif

#ifndef PF_INET
#define PF_INET AF_INET
#endif

#ifndef LPSOCKADDR
typedef struct sockaddr* LPSOCKADDR;
#endif

#ifndef LINGER
typedef struct linger LINGER;
#endif

struct WSADATA
{
	WORD wVersion;
	WORD wHighVersion;
	unsigned short iMaxSockets;
	unsigned short iMaxUdpDg;
	char* lpVendorInfo;
	char szDescription[257];
	char szSystemStatus[129];
};

inline int WSAStartup(WORD version_requested, WSADATA* wsa_data)
{
	if (wsa_data != nullptr)
	{
		memset(wsa_data, 0, sizeof(WSADATA));
		wsa_data->wVersion = version_requested;
		wsa_data->wHighVersion = version_requested;
		wsa_data->iMaxSockets = 1024;
	}
	return 0;
}

inline int WSACleanup()
{
	return 0;
}

inline int WSAAsyncSelect(SOCKET s, HWND hWnd, unsigned int wMsg, long lEvent)
{
	(void)hWnd;
	(void)wMsg;
	(void)lEvent;

	int flags = fcntl(s, F_GETFL, 0);
	if (flags < 0)
	{
		return -1;
	}
	if (fcntl(s, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		return -1;
	}
	return 0;
}

inline int closesocket(SOCKET s)
{
	return close(s);
}

#ifndef FD_READ
#define FD_READ 0x01
#endif
#ifndef FD_WRITE
#define FD_WRITE 0x02
#endif
#ifndef FD_CONNECT
#define FD_CONNECT 0x10
#endif
#ifndef FD_CLOSE
#define FD_CLOSE 0x20
#endif

#ifndef WM_USER
#define WM_USER 0x0400
#endif

#endif

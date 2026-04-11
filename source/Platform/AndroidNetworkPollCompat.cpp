#include "stdafx.h"

#if defined(__ANDROID__)

#include "Platform/AndroidNetworkPollCompat.h"
#include "Platform/WinsockAndroidCompat.h"
#include "WSctlc.h"

#include <android/log.h>
#include <poll.h>

namespace
{
	const char* kLogTag = "mu_android_net";
}

namespace platform
{
	void PollSocketIO(CWsctlc& socket_client)
	{
		SOCKET sock = socket_client.GetSocket();
		if (sock == INVALID_SOCKET)
		{
			return;
		}

		struct pollfd pfd = {};
		pfd.fd = sock;
		pfd.events = POLLIN | POLLOUT;

		int result = poll(&pfd, 1, 0);
		if (result <= 0)
		{
			return;
		}

		if (pfd.revents & POLLIN)
		{
			socket_client.nRecv();
		}

		if (pfd.revents & POLLOUT)
		{
			socket_client.FDWriteSend();
		}

		if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
		{
			__android_log_print(ANDROID_LOG_WARN, kLogTag,
				"Socket error event: revents=0x%x", pfd.revents);
			socket_client.Close();
		}
	}
}

#endif

#pragma once

#if defined(__ANDROID__)

class CWsctlc;

namespace platform
{
	void PollSocketIO(CWsctlc& socket_client);
}

#endif

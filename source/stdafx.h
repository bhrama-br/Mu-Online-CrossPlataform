// stdafx.h : include file for standard system include files,
#pragma once

//warining
#pragma warning( disable : 4067 ) 
#pragma warning( disable : 4786 ) 
#pragma warning( disable : 4800 ) 
#pragma warning( disable : 4996 ) 
#pragma warning( disable : 4244 )
#pragma warning( disable : 4237 )
#pragma warning( disable : 4305 )
#pragma warning( disable : 4503 ) 
#pragma warning( disable : 4267 ) 
#pragma warning( disable : 4091 ) 
#pragma warning( disable : 4819 )
#pragma warning( disable : 4505 )
#pragma warning( disable : 4100 )
#pragma warning( disable : 4127 )
#pragma warning( disable : 4702 )
#pragma warning( disable : 4838 )
#pragma warning( disable : 5208 )
//#pragma warning( disable : 4482 )
//#pragma warning( disable : 4700 )
//#pragma warning( disable : 4748 )
//#pragma warning( disable : 4786 )
#pragma warning( disable : 28159 )
#pragma warning( disable : 26812 )

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN	
	
#ifndef _USE_32BIT_TIME_T
	#define _USE_32BIT_TIME_T
#endif //_USE_32BIT_TIME_T

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#pragma warning( push, 3 )

#if defined(__ANDROID__)
#include "Platform/AndroidWin32Compat.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <memory.h>
#include <netdb.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "Platform/WinsockAndroidCompat.h"
#include "Platform/Win32SecondaryStubs.h"
#else
#include <windows.h>

//windows
#include <WinSock2.h>
#include <mmsystem.h>
#include <shellapi.h>

//c runtime
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>
#include <mbstring.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>
#include <conio.h>
#endif

#include <string>
#include <list>
#include <map>
#include <deque>
#include <algorithm>
#include <vector>
#include <queue>
#include <thread>


#pragma warning( pop )

//opengl
#if defined(__ANDROID__)
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "Platform/GLFixedFunctionStubs.h"
#else
#include <gl/glew.h>
#include <gl/gl.h>
#endif

//patch
//winmain
#include "Winmain.h"
#include "Defined_Global.h"

//client
#include "_define.h"
#include "_enum.h"
#include "_types.h"
#include "_struct.h"	
#if defined(__ANDROID__)
#include "Platform/LegacyAndroidGameCompat.h"
#endif
#include "w_WindowMessageHandler.h"
#include "_GlobalFunctions.h"
#if defined(__ANDROID__)
#include "NProtocol.h"
#endif
#include "_TextureIndex.h"	
#include "InfoHelperFunctions.h"
#include "UIDefaultBase.h"
#include "NewUICommon.h"
#include "./Math/ZzzMathLib.h"
#include "ZzzOpenglUtil.h"
#include "Widescreen.h"
#if defined(__ANDROID__)
#include "ItemManager.h"
#include "HelperSystem.h"
#endif
#include "Protect.h"
#if !defined(__ANDROID__)
#include "ItemManager.h"
#endif

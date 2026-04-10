#include "stdafx.h"

#include "Platform/Win32PlatformWindow.h"

namespace platform
{
	namespace
	{
		WindowCreationResult MakeWindowResult(bool ok, const char* step, DWORD error_code = 0)
		{
			WindowCreationResult result = { ok, error_code, step };
			return result;
		}

		GlContextResult MakeContextResult(bool ok, const char* step, DWORD error_code = 0)
		{
			GlContextResult result = { ok, error_code, step };
			return result;
		}
	}

	Win32PlatformWindow::Win32PlatformWindow()
		: m_hWnd(NULL)
		, m_hDC(NULL)
		, m_hRC(NULL)
	{
	}

	WindowCreationResult Win32PlatformWindow::Create(const WindowConfig& config)
	{
		WNDCLASSA wnd_class;
		memset(&wnd_class, 0, sizeof(wnd_class));

		wnd_class.style = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wnd_class.lpfnWndProc = config.wnd_proc;
		wnd_class.hInstance = config.instance;
		wnd_class.hIcon = config.icon_resource_id != 0 ? LoadIconA(config.instance, MAKEINTRESOURCEA(config.icon_resource_id)) : NULL;
		wnd_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		wnd_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wnd_class.lpszClassName = config.window_name;

		if (!RegisterClassA(&wnd_class))
		{
			const DWORD last_error = GetLastError();
			if (last_error != ERROR_CLASS_ALREADY_EXISTS)
			{
				return MakeWindowResult(false, "Window RegisterClass Error.", last_error);
			}
		}

		HWND handle = NULL;
		if (config.windowed)
		{
			RECT client_rect = { 0, 0, config.width, config.height };
			const DWORD window_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN;
			AdjustWindowRect(&client_rect, window_style, FALSE);

			const int window_width = client_rect.right - client_rect.left;
			const int window_height = client_rect.bottom - client_rect.top;

			handle = CreateWindowA(
				config.window_name,
				config.window_name,
				window_style,
				(GetSystemMetrics(SM_CXSCREEN) - window_width) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - window_height) / 2,
				window_width,
				window_height,
				NULL,
				NULL,
				config.instance,
				NULL);
		}
		else
		{
			handle = CreateWindowExA(
				WS_EX_TOPMOST | WS_EX_APPWINDOW,
				config.window_name,
				config.window_name,
				WS_POPUP,
				0,
				0,
				config.width,
				config.height,
				NULL,
				NULL,
				config.instance,
				NULL);
		}

		if (handle == NULL)
		{
			return MakeWindowResult(false, "Window CreateWindow Error.", GetLastError());
		}

		m_hWnd = handle;
		return MakeWindowResult(true, NULL);
	}

	GlContextResult Win32PlatformWindow::CreateLegacyOpenGLContext()
	{
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(pfd));

		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 16;
		pfd.cDepthBits = 16;

		m_hDC = GetDC(m_hWnd);
		if (m_hDC == NULL)
		{
			return MakeContextResult(false, "OpenGL Get DC Error.", GetLastError());
		}

		const GLuint pixel_format = ChoosePixelFormat(m_hDC, &pfd);
		if (pixel_format == 0)
		{
			const DWORD last_error = GetLastError();
			CleanupFailedOpenGLContext();
			return MakeContextResult(false, "OpenGL Choose Pixel Format Error.", last_error);
		}

		if (!SetPixelFormat(m_hDC, pixel_format, &pfd))
		{
			const DWORD last_error = GetLastError();
			CleanupFailedOpenGLContext();
			return MakeContextResult(false, "OpenGL Set Pixel Format Error.", last_error);
		}

		m_hRC = wglCreateContext(m_hDC);
		if (m_hRC == NULL)
		{
			const DWORD last_error = GetLastError();
			CleanupFailedOpenGLContext();
			return MakeContextResult(false, "OpenGL Create Context Error.", last_error);
		}

		if (!wglMakeCurrent(m_hDC, m_hRC))
		{
			const DWORD last_error = GetLastError();
			CleanupFailedOpenGLContext();
			return MakeContextResult(false, "OpenGL Make Current Error.", last_error);
		}

		return MakeContextResult(true, NULL);
	}

	GlContextResult Win32PlatformWindow::DestroyLegacyOpenGLContext()
	{
		GlContextResult result = MakeContextResult(true, NULL);

		if (m_hRC)
		{
			if (!wglMakeCurrent(NULL, NULL) && result.ok)
			{
				result = MakeContextResult(false, "Release Of DC And RC Failed.", GetLastError());
			}

			if (!wglDeleteContext(m_hRC) && result.ok)
			{
				result = MakeContextResult(false, "Release Rendering Context Failed.", GetLastError());
			}

			m_hRC = NULL;
		}

		if (m_hDC && !ReleaseDC(m_hWnd, m_hDC) && result.ok)
		{
			result = MakeContextResult(false, "OpenGL Release Error.", GetLastError());
		}

		m_hDC = NULL;
		return result;
	}

	void Win32PlatformWindow::ShowAndFocus(int show_command) const
	{
		ShowWindow(m_hWnd, show_command);
		SetForegroundWindow(m_hWnd);
		SetFocus(m_hWnd);
	}

	HWND Win32PlatformWindow::GetHandle() const
	{
		return m_hWnd;
	}

	HDC Win32PlatformWindow::GetDeviceContext() const
	{
		return m_hDC;
	}

	HGLRC Win32PlatformWindow::GetRenderContext() const
	{
		return m_hRC;
	}

	void Win32PlatformWindow::CleanupFailedOpenGLContext()
	{
		if (m_hRC)
		{
			wglDeleteContext(m_hRC);
			m_hRC = NULL;
		}

		if (m_hDC)
		{
			ReleaseDC(m_hWnd, m_hDC);
			m_hDC = NULL;
		}
	}
}

#pragma once

namespace platform
{
	struct WindowCreationResult
	{
		bool ok;
		DWORD error_code;
		const char* step;
	};

	struct GlContextResult
	{
		bool ok;
		DWORD error_code;
		const char* step;
	};

	struct WindowConfig
	{
		HINSTANCE instance;
		WNDPROC wnd_proc;
		const char* window_name;
		int width;
		int height;
		bool windowed;
		UINT icon_resource_id;
	};

	class Win32PlatformWindow
	{
	public:
		Win32PlatformWindow();

		WindowCreationResult Create(const WindowConfig& config);
		GlContextResult CreateLegacyOpenGLContext();
		GlContextResult DestroyLegacyOpenGLContext();

		void ShowAndFocus(int show_command) const;

		HWND GetHandle() const;
		HDC GetDeviceContext() const;
		HGLRC GetRenderContext() const;

	private:
		void CleanupFailedOpenGLContext();

	private:
		HWND m_hWnd;
		HDC m_hDC;
		HGLRC m_hRC;
	};
}

#pragma once

#if defined(__ANDROID__)

#include <EGL/egl.h>
#include <android/native_window.h>

namespace platform
{
	struct AndroidEglConfig
	{
		int red_bits;
		int green_bits;
		int blue_bits;
		int alpha_bits;
		int depth_bits;
		int stencil_bits;
		int swap_interval;
	};

	struct AndroidEglResult
	{
		bool ok;
		EGLint error_code;
		const char* step;
	};

	class AndroidEglWindow
	{
	public:
		AndroidEglWindow();

		void AttachWindow(ANativeWindow* window);
		void DetachWindow();

		AndroidEglResult Create(const AndroidEglConfig& config);
		void Destroy();

		bool SwapBuffers() const;
		bool IsReady() const;

		int GetWidth() const;
		int GetHeight() const;

		EGLDisplay GetDisplay() const;
		EGLSurface GetSurface() const;
		EGLContext GetContext() const;

	private:
		AndroidEglResult Fail(const char* step) const;
		void UpdateSurfaceSize();

	private:
		ANativeWindow* m_window;
		EGLDisplay m_display;
		EGLSurface m_surface;
		EGLContext m_context;
		int m_width;
		int m_height;
	};
}

#endif

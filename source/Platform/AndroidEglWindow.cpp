#if defined(__ANDROID__)

#include "Platform/AndroidEglWindow.h"

#include <GLES2/gl2.h>

namespace platform
{
	namespace
	{
		AndroidEglResult MakeResult(bool ok, const char* step, EGLint error_code = EGL_SUCCESS)
		{
			AndroidEglResult result = { ok, error_code, step };
			return result;
		}
	}

	AndroidEglWindow::AndroidEglWindow()
		: m_window(NULL)
		, m_display(EGL_NO_DISPLAY)
		, m_surface(EGL_NO_SURFACE)
		, m_context(EGL_NO_CONTEXT)
		, m_width(0)
		, m_height(0)
	{
	}

	void AndroidEglWindow::AttachWindow(ANativeWindow* window)
	{
		m_window = window;
	}

	void AndroidEglWindow::DetachWindow()
	{
		m_window = NULL;
		m_width = 0;
		m_height = 0;
	}

	AndroidEglResult AndroidEglWindow::Create(const AndroidEglConfig& config)
	{
		if (m_window == NULL)
		{
			return MakeResult(false, "AttachWindow", EGL_BAD_NATIVE_WINDOW);
		}

		m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if (m_display == EGL_NO_DISPLAY)
		{
			return Fail("eglGetDisplay");
		}

		if (eglInitialize(m_display, NULL, NULL) != EGL_TRUE)
		{
			return Fail("eglInitialize");
		}

		const EGLint config_attributes[] =
		{
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RED_SIZE, config.red_bits,
			EGL_GREEN_SIZE, config.green_bits,
			EGL_BLUE_SIZE, config.blue_bits,
			EGL_ALPHA_SIZE, config.alpha_bits,
			EGL_DEPTH_SIZE, config.depth_bits,
			EGL_STENCIL_SIZE, config.stencil_bits,
			EGL_NONE
		};

		EGLConfig egl_config = NULL;
		EGLint num_configs = 0;
		if (eglChooseConfig(m_display, config_attributes, &egl_config, 1, &num_configs) != EGL_TRUE || num_configs <= 0)
		{
			return Fail("eglChooseConfig");
		}

		EGLint native_visual_id = 0;
		if (eglGetConfigAttrib(m_display, egl_config, EGL_NATIVE_VISUAL_ID, &native_visual_id) != EGL_TRUE)
		{
			return Fail("eglGetConfigAttrib");
		}

		ANativeWindow_setBuffersGeometry(m_window, 0, 0, native_visual_id);

		m_surface = eglCreateWindowSurface(m_display, egl_config, m_window, NULL);
		if (m_surface == EGL_NO_SURFACE)
		{
			return Fail("eglCreateWindowSurface");
		}

		const EGLint context_attributes[] =
		{
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
		};

		m_context = eglCreateContext(m_display, egl_config, EGL_NO_CONTEXT, context_attributes);
		if (m_context == EGL_NO_CONTEXT)
		{
			return Fail("eglCreateContext");
		}

		if (eglMakeCurrent(m_display, m_surface, m_surface, m_context) != EGL_TRUE)
		{
			return Fail("eglMakeCurrent");
		}

		if (config.swap_interval >= 0)
		{
			eglSwapInterval(m_display, config.swap_interval);
		}

		UpdateSurfaceSize();
		glViewport(0, 0, m_width, m_height);

		return MakeResult(true, NULL);
	}

	void AndroidEglWindow::Destroy()
	{
		if (m_display != EGL_NO_DISPLAY)
		{
			eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

			if (m_context != EGL_NO_CONTEXT)
			{
				eglDestroyContext(m_display, m_context);
				m_context = EGL_NO_CONTEXT;
			}

			if (m_surface != EGL_NO_SURFACE)
			{
				eglDestroySurface(m_display, m_surface);
				m_surface = EGL_NO_SURFACE;
			}

			eglTerminate(m_display);
			m_display = EGL_NO_DISPLAY;
		}

		m_width = 0;
		m_height = 0;
	}

	bool AndroidEglWindow::SwapBuffers() const
	{
		if (!IsReady())
		{
			return false;
		}

		return eglSwapBuffers(m_display, m_surface) == EGL_TRUE;
	}

	bool AndroidEglWindow::IsReady() const
	{
		return m_window != NULL
			&& m_display != EGL_NO_DISPLAY
			&& m_surface != EGL_NO_SURFACE
			&& m_context != EGL_NO_CONTEXT;
	}

	int AndroidEglWindow::GetWidth() const
	{
		return m_width;
	}

	int AndroidEglWindow::GetHeight() const
	{
		return m_height;
	}

	EGLDisplay AndroidEglWindow::GetDisplay() const
	{
		return m_display;
	}

	EGLSurface AndroidEglWindow::GetSurface() const
	{
		return m_surface;
	}

	EGLContext AndroidEglWindow::GetContext() const
	{
		return m_context;
	}

	AndroidEglResult AndroidEglWindow::Fail(const char* step) const
	{
		return MakeResult(false, step, eglGetError());
	}

	void AndroidEglWindow::UpdateSurfaceSize()
	{
		EGLint width = 0;
		EGLint height = 0;
		eglQuerySurface(m_display, m_surface, EGL_WIDTH, &width);
		eglQuerySurface(m_display, m_surface, EGL_HEIGHT, &height);
		m_width = (int)width;
		m_height = (int)height;
	}
}

#endif

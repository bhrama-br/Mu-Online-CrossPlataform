#pragma once

#include "Platform/RenderBackend.h"

namespace platform
{
	bool IsOpenGLESRenderBackendAvailable();
	RenderBackend* CreateOpenGLESRenderBackend();
	void ShutdownOpenGLESRenderBackend();
}

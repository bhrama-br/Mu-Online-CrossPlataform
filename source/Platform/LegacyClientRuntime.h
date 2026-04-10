#pragma once

#if defined(__ANDROID__)

#include "Platform/GameClientConfig.h"
#include "Platform/GameClientRuntimeConfig.h"
#include "Platform/GameConfigBootstrap.h"
#include "Platform/GameMouseInput.h"

namespace platform
{
	void InitializeLegacyClientRuntime();
	void ApplyLegacyClientRuntimeConfig(const ClientRuntimeConfigState* config);
	void ApplyLegacyClientIniConfig(const ClientIniConfigState* config);
	void ApplyLegacyClientCoreBootstrap(const CoreGameBootstrapState* state);
	void SyncLegacyClientMouseState(const GameMouseState* mouse_state);
	void AdvanceLegacyClientMouseState(GameMouseState* mouse_state);
	void SetLegacyClientSceneFlag(int scene_flag);
}

#endif

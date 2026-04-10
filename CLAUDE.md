# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

RoxGaming MU Client — a cross-platform MMORPG game client (MU Online private server). C++ codebase with a legacy Win32/OpenGL desktop client being incrementally ported to Android via a platform abstraction layer.

## Build Commands

### Windows (Visual Studio 2022, v143 toolset, x86 32-bit only)

Open `Main.sln` in Visual Studio 2022. Three configurations:

- **Global Debug|Win32**: `/std:c++17`, full debug symbols, Level 3 warnings
- **Global Release|Win32**: `/std:c++14` (NOT c++17), optimized O2, all warnings disabled
- **NewDebug|Win32**: hybrid debug/release

Output binaries: `Global Debug/Main.exe` or `Global Release/Main.exe`

### Android (Gradle + CMake, ARM64/ARMv7)

```sh
cd android && ./gradlew assembleDebug
./android/run_adb_debug.sh                    # helper: build + install + launch
./gradlew -PmuClientDataDir=/path assembleDebug  # override data source
./gradlew -PmuAutoSyncClientData=false assembleDebug  # skip data sync
```

Mini-data debug build:
```sh
MU_ANDROID_MINI_DATA_DIR=/path/to/mini-data ./android/run_adb_debug.sh --clean
```

Inject runtime config:
```sh
MU_ANDROID_RUNTIME_CONFIG_INI=/path/to/client_runtime.ini ./android/run_adb_debug.sh --clean
```

Android CMake: `android/app/src/main/cpp/CMakeLists.txt` (C++17, links EGL + GLESv2)

## Architecture

### Entry Points

- **Windows**: `source/Winmain.cpp` — Win32 `WinMain()`, creates HWND + OpenGL context via wgl, runs PeekMessage/DispatchMessage game loop
- **Android**: `android/app/src/main/cpp/android_bootstrap.cpp` — NativeActivity, EGL + OpenGL ES 2.0 context. Currently a bootstrap shell; full MU client not wired yet

### Core Modules (all in `source/`)

| Module | Key Files | Purpose |
|--------|-----------|---------|
| Renderer | `ZzzScene.cpp`, `ZzzOpenglUtil.cpp`, `ZzzBMD.cpp`, `ZzzTexture.cpp` | Legacy fixed-function OpenGL (glBegin/glEnd, glMatrixMode). Loads BMD 3D models, OZT/OZJ textures |
| Game Logic | `ZzzCharacter.cpp`, `ZzzObject.cpp`, `ZzzInventory.cpp`, `ZzzLodTerrain.cpp` | Characters, NPCs, items, terrain LOD |
| UI | `ZzzInterface.cpp`, `UIWindows.h`, `UIManager.h`, `LoginWin.cpp`, `ServerSelWin.cpp`, `CharSelMainWin.cpp` | GDI-based text rendering, Win32 edit controls, IME input |
| Network | `PacketManager.cpp`, `wsclientinline.h` | Winsock + WSAAsyncSelect, packet serialization |
| Effects | `ZzzEffect.cpp`, `ZzzEffectParticle.cpp`, `ZzzEffectBitmap.cpp` | Particle systems, skill effects, blur |
| Audio | `DSPlaySound.cpp` | DirectSound (Win32 only) |
| Scripting | `Lua.cpp`, `LuaSkill.cpp`, `LuaItem.cpp`, `LuaCharacter.cpp` | Lua 5.x integration for game behaviors |
| Platform Layer | `source/Platform/` (56 files) | NEW — abstracts Win32 from game logic for mobile port |

### Platform Abstraction Layer (`source/Platform/`)

This is the active migration layer. Key abstractions:
- `RenderBackend.h` — abstract render interface
- `OpenGLESRenderBackend.h/cpp` — OpenGL ES 2.0 implementation (in progress)
- `AndroidWin32Compat.h` — Win32 API type/constant shims for Android
- `GameClientConfig.h` / `GameClientRuntimeConfig.h` — config loading
- `GameAssetPath.h` — cross-platform asset path resolution
- `GameServerBootstrap.h` / `GameConnectServerBootstrap.h` — server connection setup
- `GamePacketCryptoBootstrap.h` — packet encryption key loading
- `LegacyLoginUiRuntime.h` — login UI abstraction

### Game Flow

Login (`LoginWin`) → Server Selection (`ServerSelWin`) → Character Selection (`CharSelMainWin`) → Character Creation (`CharMakeWin`) → Gameplay Scene (`ZzzScene`)

### Data & Config Files

- `Data/Configs/Configs.xtm` — binary game config (CRC-validated)
- `Data/config.ini` — text client settings (server, language, resolution)
- `Data/client_runtime.ini` — runtime overrides (server host/port, auto-login credentials)
- `Data/Enc1.dat`, `Data/Dec2.dat` — 54-byte packet encryption keys
- `Data/Local/<lang>/` — localized text and assets
- Precompiled header: `stdafx.h` (compiled via `StdAfx.cpp`)

### Dependencies

Prebuilt libs in `Dependencies/`: ASIO (networking), Boost 1.75 (partial), JPEG, Lua 5.1/5.3, OpenGL extensions, wzAudio (MP3), SimpleModulus (crypto), ShareMemory (IPC), Themida SDK (anti-tamper).

Windows linker: `imm32 vfw32 dsound dxguid jpeglib opengl32 glu32 winmm ws2_32 glprocs wzAudio ShareMemory SimpleModulus`

## Critical Gotchas

- **C++ standard mismatch**: Global Release uses `/std:c++14`, not C++17. Do not use C++17 features in code that must compile in Release
- **32-bit x86 only on Windows**: `MachineX86` target. Pointer-to-DWORD casts and `_asm` blocks exist — these break on ARM64/x64
- **Hardcoded paths in vcxproj**: Boost at `C:\Libraries\boost_1_75_0`, DirectX SDK via `$(DXSDK_DIR)DXSDK61\`
- **Release warnings disabled**: `TurnOffAllWarnings` — do not blindly enable warnings-as-errors
- **`GM_Raklion.cpp` encoding**: Has mixed-encoding content; needs safe cleanup before any UTF-8 normalization pass
- **Legacy OpenGL**: 67 `glBegin()` calls, 915 `glColor*` calls — entire renderer is fixed-function pipeline, incompatible with OpenGL ES without migration
- **GDI text**: UI text rendered via `CreateDIBSection`/`TextOut` uploaded to OpenGL textures — not portable to mobile
- **Largest files**: `CGFxMainUi.cpp` (~146KB), `wsclientinline.h` (~65KB)

## Mobile Port Strategy

The port follows an incremental compatibility approach (not a rewrite). See `MOBILE_PORT_ASSESSMENT.md` for full analysis (in Portuguese). Current status:
- Platform abstraction layer started in `source/Platform/`
- Android bootstrap functional (EGL, asset loading, config validation, texture preview)
- Not yet wired: full game client, input, audio, networking on Android

Phased plan: (1) platform layer interfaces → (2) shader-based render migration → (3) touch-to-mouse translator → (4) audio/network abstraction → (5) Android validation, then iOS

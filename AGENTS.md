# RoxGaming MU Client

## Project Structure

- **Main.sln / Main.vcxproj**: Windows desktop client (Visual Studio 2022, v143, C++17, x86)
- **android/**: Android shell (Gradle, CMake, C++17, ARM64/ARMv7)
- **source/Platform/**: Shared platform abstraction layer (render, config, input, audio stubs)
- **Dependencies/**: Pre-built libs (asio, boost, etc.)
- **Data/**: Game assets (textures, maps, configs) - not in this repo, comes from `Client/`

## Build Commands

### Windows
Open `Main.sln` in Visual Studio 2022. Configurations:
- `Global Debug|Win32`: Full debug with warnings, `/std:c++17`
- `Global Release|Win32`: Optimized, warnings disabled, `/std:c++14`
- `NewDebug|Win32`: Hybrid debug/release config

Output: `Global Debug/Main.exe` or `Global Release/Main.exe`
Intermediate: `source/$(Configuration)/`

### Android
```sh
cd android && ./gradlew assembleDebug
# or use the helper:
./android/run_adb_debug.sh
```

Build data assets source defaults to `../../../Client/Data` (relative to android root).

Override packaged data:
```sh
./gradlew -PmuClientDataDir=/path/to/Data assembleDebug
./gradlew -PmuAutoSyncClientData=false assembleDebug   # skip auto-sync
```

## Key Architecture Notes

- Entry point: `source/Winmain.cpp` (Win32) / `android/app/src/main/cpp/` (Android bootstrap)
- Precompiled header: `stdafx.h`, compiled via `StdAfx.cpp`
- Platform layer files in `source/Platform/` abstract Win32 from game logic
- Configs: `Configs.xtm`, `CustomJewel.xtm` (binary format), `config.ini` (text)
- Network: Winsock on Win32, needs platform abstraction for mobile
- Renderer: Legacy fixed-function OpenGL (needs OpenGL ES migration for Android)
- Text: GDI-based on Windows (needs atlas/font system for mobile)
- Audio: DirectSound (Win32), needs cross-platform backend
- Scripts: Lua scripts drive character, items, effects, networking

## Android-Specific

- Package: `com.roxgaming.mu`
- Min SDK: 24, Target SDK: 34
- APK extracts game data to `/data/user/0/com.roxgaming.mu/files/Data`
- Runtime overrides via `client_runtime.ini`:
  - `ConnectServerHost`, `ConnectServerPort`
  - `GameServerHost`, `GameServerPort`
  - `AutoLoginUser`, `AutoLoginPassword`
- `source/Platform/OpenGLESRenderBackend.cpp` has initial OpenGL ES 2.0 backend stub
- Android CMake path: `android/app/src/main/cpp/CMakeLists.txt`

## Mobile Port Status

Read `MOBILE_PORT_ASSESSMENT.md` for detailed analysis. Current Android shell bootstraps OpenGL ES and loads assets but does not yet wire the full MU game client. The path forward is incremental: platform abstraction first, then render migration.

## Gotchas

- `Global Release` uses `/std:c++14` (not C++17) - be careful with modern features
- `WarningLevel` is `TurnOffAllWarnings` in Release - do not enable warnings as errors blindly
- `Main.vcxproj` has hardcoded Windows SDK and library paths (C:\Libraries\boost_1_75_0, DXSDK)
- `GM_Raklion.cpp` has mixed-encoding content that needs cleanup before UTF-8 normalization
- Android build is CMake-based (not ndk-build)

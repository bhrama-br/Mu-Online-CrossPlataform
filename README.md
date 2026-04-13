# BhramaBR MU Client (Android Port)

Port of the RoxGaming MU 5.2 client to Android.

## Project Status

This project is under active development and is not yet 100% playable.

- Android build compiles
- Part of rendering/networking is already working
- Original UI (login/server/character) is still being migrated

## Roadmap

Overall Android port progress: **~45%**

### What is already done

| Component | Progress |
|-----------|----------|
| Build System | 100% |
| Platform Layer (65 files) | 95% |
| Texture Loading (OZJ/OZT) | 100% |
| Networking/Protocol | 90% |
| 3D Terrain Rendering | 80% |

### What is still missing

| Component | Progress | Current Gap |
|-----------|----------|-------------|
| Login/Server Select UI | 30% | Not 100% yet; original UI flow still needs to be fully restored |
| Character Select UI | 20% | Original scene/UI integration still incomplete |
| Text Rendering | 40% | Needs reliable Android text rendering path |
| 3D Model Rendering | 60% | Needs broader validation for models/animations |
| Gameplay (MAIN_SCENE) | 5% | Still not fully wired/tested on Android |
| Audio | 0% | Audio backend not implemented yet |
| Soft Keyboard Input | 0% | Android IME integration missing |

### Next priority

1. Restore original login/server/character UI flow on Android.
2. Finalize text rendering for UI readability.
3. Validate character scene and transition to gameplay.

## We Need Help

Contributions are welcome to speed up the port, especially in:

- Original UI on Android
- Text rendering
- Soft keyboard input
- Audio
- Gameplay testing

## How to Run (Android)

## 1) Add client assets (required)

Copy the client `Data` folder to:

`/android/app/src/main/assets/Data`

Example:

```bash
cp -R /path/to/Client/Data ./android/app/src/main/assets/
```

Without this folder, the app cannot load essential game files.

## 2) Build the APK

```bash
cd android
./gradlew assembleDebug
```

## 3) Install and launch on device/emulator

```bash
cd ..
./android/run_adb_debug.sh
```

## 4) Check logs (optional)

```bash
adb logcat -s MUAndroidBootstrap:I mu_android_init:I mu_android_char:I MUAndroidTexture:W
```

## Relevant Structure

- `android/`: Android project (Gradle + CMake)
- `source/`: legacy code + portability layer
- `source/Platform/`: Android abstractions
- `ANDROID_PORT_README.md`: detailed technical status of the port

## Note

This project is evolving continuously. Architecture and flow may change during development.

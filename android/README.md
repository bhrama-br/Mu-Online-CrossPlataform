# Android Bootstrap

This folder is the first Android shell for the mobile port.

Current scope:
- Creates a `NativeActivity`
- Brings up an EGL window surface
- Creates an OpenGL ES 2.0 context
- Exercises the shared `RenderBackend` through a simple animated clear color
- Resolves a default game-data root from the Android app storage and checks `Data/Configs/Configs.xtm`
- Prepares packaged Android assets automatically from the original `Client/Data` when that folder exists
- Runs the shared core-config bootstrap and validates `Configs.xtm` plus `CustomJewel.xtm`
- Packages and extracts `config.ini`, then parses its shared client settings bootstrap
- Packages and extracts an optional `client_runtime.ini`, then parses the shared runtime settings bootstrap
- Runs a packet-crypto bootstrap for `Enc1.dat` and `Dec2.dat` using the shared packet manager implementation
- Validates the selected `Data/Local/<lang>` asset set and falls back to a known language when the requested one is missing
- Loads the localized `Text_*.bmd` file and exposes real UI text samples from the client data
- Loads a first real `Interface` texture set from `.OZT` files and renders a lightweight bootstrap preview with client art

Current Android data root behavior:
- If the build packaged `Data` into the APK assets, the app extracts it automatically on first launch to `<internalDataPath>/Data`
- If `android/app/src/main/assets/Data` exists, it has priority as the packaged source
- Otherwise, the Android build auto-syncs the original `../../../Client/Data` into generated APK assets
- Without packaged assets, it still prefers `<externalDataPath>/Data`
- Then it falls back to `<internalDataPath>/Data`
- Logs the resolved game-data root and whether `Configs.xtm` was found
- Uses a greener clear color when game data is present, and a warmer clear color when it is missing
- With the current app id `com.roxgaming.mu`, the typical extracted path on device is `/data/user/0/com.roxgaming.mu/files/Data`
- The legacy manual external path is still `/storage/emulated/0/Android/data/com.roxgaming.mu/files/Data`

Current limitations:
- It does not build the MU client yet
- Win32 input, audio, networking, and message-loop code are not wired here
- Asset loading from `Data` is only wired into the platform/bootstrap layer today; the full MU client is not attached yet
- `GM_Raklion.cpp` still has legacy mixed-encoding content that needs a safe cleanup pass before we do a broader UTF-8 sweep
- Packaging the full MU `Data` into the APK will make Android debug builds much heavier

Current bootstrap logging:
- Logs whether packaged `Data` was found
- Logs the resolved extracted game-data root
- Logs whether `Configs.xtm` is present
- Logs whether packaged `config.ini` was found and where it was extracted
- Logs whether packaged `client_runtime.ini` was found and where it was extracted
- Logs parsed `config.ini` values such as `ConfigVersion`, `TestVersion`, and `AutoLoginUser`
- Logs parsed runtime values such as language, resolution, window size, and window mode
- Logs runtime connect-server overrides when `client_runtime.ini` provides them
- Logs language asset resolution and whether a fallback language was needed
- Logs localized `GlobalText` samples such as `OK`, `Cancelar`, `Fechar`, and `Destinatário`
- Logs which preview textures from `Data/Interface` were loaded for the Android bootstrap
- Logs parsed values from the shared core bootstrap such as `WindowName`, connect-server IP/port, client version, and the main config CRC
- Logs packet-crypto key loading and a roundtrip encrypt/decrypt validation for `Enc1.dat` and `Dec2.dat`

ADB debug helper:
- Run `./android/run_adb_debug.sh` for the normal debug APK flow
- Run `MU_ANDROID_MINI_DATA_DIR=/absolute/path/to/mini-data ./android/run_adb_debug.sh --clean` for a lightweight debug APK that only packages the files you point to
- Run `MU_ANDROID_RUNTIME_CONFIG_INI=/absolute/path/to/client_runtime.ini ./android/run_adb_debug.sh --clean` to inject a specific runtime config into the APK for Android debugging

Runtime override keys:
- Add `ConnectServerHost=<ip-or-hostname>` under `[Runtime]` in `client_runtime.ini` to override the connect server without changing `Configs.xtm`
- Add `ConnectServerPort=<port>` under `[Runtime]` to override just the port or to pair with `ConnectServerHost`
- Add `GameServerHost=<ip-or-hostname>` under `[Runtime]` when the server list returns a private/internal game-server IP that Android cannot reach directly
- Add `GameServerPort=<port>` under `[Runtime]` to override the game-server port if needed
- Add `AutoLoginUser=<account>` under `[Runtime]` to override the account loaded from `config.ini`
- Add `AutoLoginPassword=<password>` under `[Runtime]` to override the password loaded from `config.ini`
- This is useful for Android emulator tests when you need a different endpoint than the packaged client config

When this becomes useful:
- The first Android debug APK already builds successfully
- Next step is to route real client systems and asset consumers into this target

Build knobs:
- Default source for Android packaged data is `../../../Client/Data` relative to the Android root
- Default source for packaged `config.ini` is `../../../Client/config.ini`
- Default source for packaged `client_runtime.ini` is `../../../Client/client_runtime.ini`
- Override it with `-PmuClientDataDir=/absolute/path/to/Data`
- Override `config.ini` with `-PmuClientConfigIni=/absolute/path/to/config.ini`
- Override `client_runtime.ini` with `-PmuClientRuntimeConfigIni=/absolute/path/to/client_runtime.ini`
- Disable auto-sync with `-PmuAutoSyncClientData=false`
- If you already keep `Data` in `android/app/src/main/assets/Data`, the auto-sync path is skipped automatically
- Ignore `android/app/src/main/assets/Data` for a lighter debug build with `-PmuUseManualAssetsData=false`

#!/bin/zsh
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PACKAGE_NAME="com.roxgaming.mu"
APK_PATH="$PROJECT_ROOT/android/app/build/outputs/apk/debug/app-debug.apk"
GRADLE_ARGS=("-p" "$PROJECT_ROOT/android" "assembleDebug")

if ! command -v adb >/dev/null 2>&1; then
  echo "adb not found in PATH"
  exit 1
fi

if [[ "${1:-}" == "--clean" ]]; then
  GRADLE_ARGS=("-p" "$PROJECT_ROOT/android" "clean" "assembleDebug")
  shift
fi

if [[ -n "${MU_ANDROID_MINI_DATA_DIR:-}" ]]; then
  GRADLE_ARGS+=(
    "-PmuUseManualAssetsData=false"
    "-PmuClientDataDir=${MU_ANDROID_MINI_DATA_DIR}"
  )
fi

if [[ -n "${MU_ANDROID_RUNTIME_CONFIG_INI:-}" ]]; then
  GRADLE_ARGS+=(
    "-PmuClientRuntimeConfigIni=${MU_ANDROID_RUNTIME_CONFIG_INI}"
  )
fi

echo "==> Building Android debug APK"
gradle "${GRADLE_ARGS[@]}"

echo "==> Checking adb devices"
adb devices -l

echo "==> Installing APK"
adb install -r "$APK_PATH"

echo "==> Restarting app"
adb shell am force-stop "$PACKAGE_NAME" || true
adb logcat -c
adb shell monkey -p "$PACKAGE_NAME" -c android.intent.category.LAUNCHER 1 >/dev/null
sleep 2

echo "==> Recent bootstrap logs"
adb logcat -d -s MUAndroidBootstrap AndroidRuntime DEBUG libc

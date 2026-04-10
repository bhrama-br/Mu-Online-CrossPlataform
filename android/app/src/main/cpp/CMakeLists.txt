cmake_minimum_required(VERSION 3.22.1)

project(mu_android_bootstrap LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MU_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../../)

add_library(mu_android_bootstrap SHARED
    android_bootstrap.cpp
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    ${MU_ROOT_DIR}/source/CCRC32.cpp
    ${MU_ROOT_DIR}/source/CriticalSection.cpp
    ${MU_ROOT_DIR}/source/Sprite.cpp
    ${MU_ROOT_DIR}/source/Win.cpp
    ${MU_ROOT_DIR}/source/WinEx.cpp
    ${MU_ROOT_DIR}/source/Button.cpp
    ${MU_ROOT_DIR}/source/GaugeBar.cpp
    ${MU_ROOT_DIR}/source/Input.cpp
    ${MU_ROOT_DIR}/source/PList.cpp
    ${MU_ROOT_DIR}/source/LoginMainWin.cpp
    ${MU_ROOT_DIR}/source/CharSelMainWin.cpp
    ${MU_ROOT_DIR}/source/CharMakeWin.cpp
    ${MU_ROOT_DIR}/source/LoadData.cpp
    ${MU_ROOT_DIR}/source/PacketManager.cpp
    ${MU_ROOT_DIR}/source/Protect.cpp
    ${MU_ROOT_DIR}/source/ServerSelWin.cpp
    ${MU_ROOT_DIR}/source/ServerGroup.cpp
    ${MU_ROOT_DIR}/source/ServerInfo.cpp
    ${MU_ROOT_DIR}/source/ServerListManager.cpp
    ${MU_ROOT_DIR}/source/LoginWin.cpp
    ${MU_ROOT_DIR}/source/UIMng.cpp
    ${MU_ROOT_DIR}/source/Time/Timer.cpp
    ${MU_ROOT_DIR}/source/Math/ZzzMathLib.cpp
    ${MU_ROOT_DIR}/source/TextureScript.cpp
    ${MU_ROOT_DIR}/source/ZzzOpenglUtil.cpp
    ${MU_ROOT_DIR}/source/ZzzBMD.cpp
    ${MU_ROOT_DIR}/source/ZzzOpenData.cpp
    ${MU_ROOT_DIR}/source/ZzzCharacter.cpp
    ${MU_ROOT_DIR}/source/w_ObjectInfo.cpp
    ${MU_ROOT_DIR}/source/w_CharacterInfo.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyObjectCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyCharacterSceneCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyCharacterSceneLinkCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyTerrainCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyMapManagerCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyErrorReportCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyAndroidClientStubs.cpp
    ${MU_ROOT_DIR}/source/Platform/GlobalBitmapAndroidCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/ZzzTextureAndroidCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/GameAssetPath.cpp
    ${MU_ROOT_DIR}/source/Platform/GameBootstrapTexture.cpp
    ${MU_ROOT_DIR}/source/Platform/GameClientConfig.cpp
    ${MU_ROOT_DIR}/source/Platform/GameConnectServerBootstrap.cpp
    ${MU_ROOT_DIR}/source/Platform/GameClientRuntimeConfig.cpp
    ${MU_ROOT_DIR}/source/Platform/GameConfigBootstrap.cpp
    ${MU_ROOT_DIR}/source/Platform/GameGlobalTextBootstrap.cpp
    ${MU_ROOT_DIR}/source/Platform/GameLanguageAssetBootstrap.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyClientRuntime.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyAndroidUiCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyLoginUiRuntime.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyCharacterUiRuntime.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyLoginRenderCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyLoginServerListRuntime.cpp
    ${MU_ROOT_DIR}/source/Platform/GameMouseInput.cpp
    ${MU_ROOT_DIR}/source/Platform/GamePacketCryptoBootstrap.cpp
    ${MU_ROOT_DIR}/source/Platform/GameServerBootstrap.cpp
    ${MU_ROOT_DIR}/source/Platform/AndroidWin32Compat.cpp
    ${MU_ROOT_DIR}/source/Platform/RenderBackend.cpp
    ${MU_ROOT_DIR}/source/Platform/OpenGLESRenderBackend.cpp
    ${MU_ROOT_DIR}/source/Platform/AndroidEglWindow.cpp
)

add_library(mu_android_legacy_login_ui_probe OBJECT
    ${MU_ROOT_DIR}/source/CriticalSection.cpp
    ${MU_ROOT_DIR}/source/Input.cpp
    ${MU_ROOT_DIR}/source/Win.cpp
    ${MU_ROOT_DIR}/source/Button.cpp
    ${MU_ROOT_DIR}/source/LoginMainWin.cpp
    ${MU_ROOT_DIR}/source/ServerSelWin.cpp
    ${MU_ROOT_DIR}/source/LoginWin.cpp
    ${MU_ROOT_DIR}/source/CharMakeWin.cpp
    ${MU_ROOT_DIR}/source/UIMng.cpp
)

target_include_directories(mu_android_bootstrap PRIVATE
    ${MU_ROOT_DIR}/source
    ${ANDROID_NDK}/sources/android/native_app_glue
)

target_compile_definitions(mu_android_bootstrap PRIVATE
    MU_ANDROID_HAS_ZZZCHARACTER_RUNTIME=1
    MU_ANDROID_HAS_ZZZOPENGLUTIL_RUNTIME=1
)

target_link_options(mu_android_bootstrap PRIVATE
    -Wl,--gc-sections
)

target_include_directories(mu_android_legacy_login_ui_probe PRIVATE
    ${MU_ROOT_DIR}/source
    ${ANDROID_NDK}/sources/android/native_app_glue
)

add_dependencies(mu_android_bootstrap mu_android_legacy_login_ui_probe)

find_library(android-lib android)
find_library(log-lib log)
find_library(egl-lib EGL)
find_library(glesv2-lib GLESv2)
find_library(dl-lib dl)

target_link_libraries(mu_android_bootstrap
    PRIVATE
    ${android-lib}
    ${dl-lib}
    ${log-lib}
    ${egl-lib}
    ${glesv2-lib}
)

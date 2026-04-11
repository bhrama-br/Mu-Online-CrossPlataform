# RoxGaming MU Client — Android Port Status

## Visao Geral

Port incremental do cliente MU Online (RoxGaming 5.2) de Windows (Win32/OpenGL) para Android (NativeActivity/OpenGL ES 2.0). O codigo original esta em `MainOLD/` como referencia. O port ativo esta em `source/` + `android/`.

---

## O Que Ja Foi Feito

### 1. Sistema de Build Android (COMPLETO)

- **Gradle + CMake** configurado em `android/app/src/main/cpp/CMakeLists.txt`
- **~493 arquivos fonte** compilam com sucesso (C++17, ARM64/ARMv7)
- Lua 5.1.5 compilado do source (como C)
- IJG JPEG 9e compilado do source (como C)
- Links: `EGL`, `GLESv2`, `android`, `log`, `dl`
- Build: `cd android && ./gradlew assembleDebug`
- Install+launch: `./android/run_adb_debug.sh`

### 2. Camada de Plataforma — 65 Arquivos (`source/Platform/`)

| Componente | Arquivos | Status |
|------------|----------|--------|
| EGL Window | `AndroidEglWindow.h/cpp` | FUNCIONAL |
| Render Backend GLES2 | `OpenGLESRenderBackend.h/cpp`, `RenderBackend.h/cpp` | FUNCIONAL (parcial) |
| Render State Compat | `RenderColorCompat.h`, `RenderStateCompat.h`, `GLFixedFunctionStubs.h` | FUNCIONAL |
| Tipos Win32 | `AndroidWin32Compat.h` (847 linhas) | FUNCIONAL — 100+ constantes, tipos, macros |
| Winsock Compat | `WinsockAndroidCompat.h` | FUNCIONAL — WSAStartup, closesocket, WSAAsyncSelect shimados |
| Network Poll | `AndroidNetworkPollCompat.h/cpp` | FUNCIONAL — PollSocketIO substitui WSAAsyncSelect |
| Asset Path | `GameAssetPath.h/cpp` | FUNCIONAL — resolucao cross-platform de caminhos |
| Config Loading | `GameClientConfig.h/cpp`, `GameClientRuntimeConfig.h/cpp` | FUNCIONAL |
| Config Bootstrap | `GameConfigBootstrap.h/cpp` | FUNCIONAL — CProtect + Configs.xtm |
| Packet Crypto | `GamePacketCryptoBootstrap.h/cpp` | FUNCIONAL — Enc1.dat/Dec2.dat |
| Connect Server | `GameConnectServerBootstrap.h/cpp` | FUNCIONAL |
| Game Server | `GameServerBootstrap.h/cpp` | FUNCIONAL |
| Mouse/Touch Input | `GameMouseInput.h/cpp` | FUNCIONAL — touch → mouse translation |
| Audio Stubs | `AudioAndroidStub.cpp` | FUNCIONAL (no-op) — todas funcoes de audio silenciadas |
| Globais do Winmain | `LegacyClientRuntime.h/cpp` | FUNCIONAL — g_hWnd, Destroy, RandomTable, etc. |
| Client Stubs | `LegacyAndroidClientStubs.cpp` | FUNCIONAL — CheckHack, GetCheckSum, etc. |
| Text/GDI Stubs | `Win32SecondaryStubs.h` | FUNCIONAL — CreateDIBSection, TextOut, SelectObject |
| Texture Compat | `ZzzTextureAndroidCompat.cpp`, `GlobalBitmapAndroidCompat.cpp` | FUNCIONAL |

### 3. Carregamento de Texturas (COMPLETO)

- **Formato OZJ**: Header de 24 bytes (copia dos primeiros 24 bytes do JPEG) + dados JPEG reais. Detecta offset 24 primeiro, depois offset 0 como fallback.
- **libjpeg ABI**: Header v9e (`Dependencies/jpeg-9e-src/jpeglib.h`) usado no Android para match com a lib compilada (struct size 664 bytes).
- **Case-insensitive file open**: `fopen_ci()` em `GlobalBitmap.cpp` usa `opendir`/`readdir`/`strcasecmp` para encontrar arquivos com case diferente no ext4 do Android.
- **Resultado**: ZERO falhas de carregamento de textura. Terreno 3D renderiza com texturas.

### 4. Render Pipeline (PARCIAL)

- `OpenGLESRenderBackend` traduz fixed-function GL → GLES 2.0 shaders
- `RenderScene()` chamada a cada frame via `RenderOriginalGameFrame()`
- Terrain rendering funcional com texturas
- `BeginBitmap()`/`EndBitmap()` para UI 2D overlay funcional
- `glBegin/glEnd` emulados via vertex buffer + shader pipeline

### 5. Networking (FUNCIONAL)

- `WinsockAndroidCompat.h` shima `WSAStartup`, `closesocket`, `WSAAsyncSelect`
- `PollSocketIO()` substitui model WSAAsyncSelect por `poll()` com timeout 0
- `ProtocolCompiler()` processa pacotes normalmente
- Connect server, login, character list, character create/delete: todos funcionais via bootstrap
- `CreateSocket()` funciona no Android via sockets POSIX

### 6. Bootstrap Android (`android_bootstrap.cpp` — 6648 linhas)

- Entry point `android_main()` com NativeActivity
- Pipeline de inicializacao: EGL → configs → language → global text → server list → CProtect → crypto → render backend
- Dois modos de render:
  - **Bootstrap UI**: Interface preview customizada (server browser + login form + character select)
  - **Original Game**: `RenderScene()` do ZzzScene.cpp (ativado quando todos os flags de bootstrap estao OK)
- Touch input com multi-touch, hit testing, keyboard virtual

### 7. Game Loop Original (INTEGRADO)

- `SceneFlag = WEBZEN_SCENE` → `WebzenScene()` → `OpenBasicData()` → `SceneFlag = LOG_IN_SCENE`
- `MainScene()` despacha para `NewRenderLogInScene()`, `NewRenderCharacterScene()`, `RenderMainScene()`
- `CheckRenderNextFrame()` controla frame pacing a 60 FPS
- `PollSocketIO(SocketClient)` + `ProtocolCompiler()` substituem WSAAsyncSelect

---

## O Que Falta Para Ficar 100% Jogavel

### PRIORIDADE ALTA — Telas de Login/Servidor/Personagem (COMPLETO)

#### A. Restaurar UI Original do Login/Servidor — FEITO

Todos os `#ifdef __ANDROID__` que desabilitavam o rendering da UI original foram removidos. Os arquivos agora usam o codigo original identico ao Windows:

| Arquivo | Status | Mudancas |
|---------|--------|----------|
| `UIMng.h` | RESTAURADO | Removidos todos os guards de includes e member vars |
| `UIMng.cpp` | RESTAURADO | `CreateTitleSceneUI()`, `RenderTitleSceneUI()`, `CreateLoginScene()`, `CreateCharacterScene()`, `PopUpMsgWin()`, `AddServerMsg()` — todos restaurados com implementacao original |
| `ServerSelWin.cpp` | RESTAURADO | `RenderControls()` e `UpdateWhileActive()` agora usam `SendRequestServerList()` e `SendRequestServerAddress()` originais |
| `LoginWin.cpp` | RESTAURADO | `RequestLogin()` usa `SendRequestLogIn()`, `CancelLogin()` e `ConnectConnectionServer()` usam `CreateSocket()` diretamente |
| `LoginMainWin.cpp` | RESTAURADO | `UpdateWhileActive()` usa `ShowWin()` original em vez de platform stubs |
| `CharSelMainWin.cpp` | RESTAURADO | `UpdateWhileActive()`, `DeleteCharacter()`, `UpdateDisplay()` restaurados com logica original |
| `CharMakeWin.cpp` | RESTAURADO | `RequestCreateCharacter()`, `SelectCreateCharacter()`, `UpdateCreateCharacter()`, `RenderCreateCharacter()` restaurados |
| `MsgWin.cpp` | RESTAURADO | Removidos guards que excluiam `CharSelMainWin.UpdateDisplay()` e `CharMakeWin.ShowWin()` |
| `ZzzScene.cpp` | RESTAURADO | Removidos guards em torno de CreditWin checks, VK_ESCAPE/VK_RETURN handling, double-click start game |

#### B. Compat Win32 em Arquivos UI — JA FUNCIONAL

`CreateFont()`, `DeleteObject()`, `PostMessage()`, `MessageBox()` ja estao shimados em `AndroidWin32Compat.h/cpp`. Nenhuma mudanca adicional necessaria.

#### C. Inicializacao de Globais do Login — JA FUNCIONAL

- `szServerIpAddress` e `g_ServerPort` sao setados por `ApplyLegacyClientCoreBootstrap()` a partir do `gProtect->m_MainInfo`
- `Version[]` e `Serial[]` tambem sao setados
- `gProtect` e carregado antes de `CreateLogInScene()` chamar `CreateSocket()`

### PRIORIDADE MEDIA — Rendering Completo

#### D. Rendering de Texto (IMPLEMENTADO)

- `AndroidTextRenderer.h/cpp` usando **stb_truetype.h** (rasterizador TTF single-header)
- Carrega fontes do sistema Android (`/system/fonts/Roboto-Regular.ttf` + Bold)
- DC tracking struct gerencia bitmap buffer + font + cores
- `CreateDIBSection` registra DIBs, `SelectObject` associa com DCs
- `TextOutW` renderiza glifos no buffer 24bpp, threshold alpha>=80 → 255
- `GetTextExtentPoint32W` mede texto com metricas reais do font
- Pipeline completo: TextOut → WriteText (BGR→RGBA) → UploadText (glTexImage2D) → RenderBitmap
- **TODO**: Verificar qualidade visual no dispositivo (pode precisar ajuste de threshold)

#### E. Modelos 3D BMD (PARCIAL)

- BMD loader compila e carrega modelos
- Vertex rendering via GLES 2.0 shaders
- **TODO**: Verificar que todos os modelos de personagem renderizam corretamente
- **TODO**: Skeletal animation pode ter problemas com bone transforms

#### F. Mapa/Terreno (PARCIAL)

- Terreno da login scene (world 94) renderiza com texturas
- **TODO**: Testar todos os mapas do jogo (Lorencia, Devias, Noria, etc.)
- **TODO**: Verificar lightmaps, water effects, terrain LOD

#### G. Efeitos Visuais (NAO TESTADO)

- Particulas, joints, sprites compilam
- **TODO**: Testar efeitos de skills, explosoes, weather
- **TODO**: Alpha blending modes podem precisar de ajustes no GLES 2.0

### PRIORIDADE BAIXA — Gameplay Completo

#### H. MAIN_SCENE — Gameplay (NAO IMPLEMENTADO)

- `RenderMainScene()` compila mas nunca foi testado no Android
- **TODO**: Hero movement (touch → walk)
- **TODO**: Camera controls (pinch zoom, rotate)
- **TODO**: NPC interaction (touch → talk)
- **TODO**: Monster targeting e combat
- **TODO**: Inventory management
- **TODO**: Skill bar e skill usage

#### I. Audio (NAO IMPLEMENTADO)

- Todas funcoes de audio sao no-op (silent)
- **TODO**: Implementar playback com OpenSL ES ou AAudio
- Arquivos: WAV (efeitos sonoros) + MP3 (musicas BGM)
- Funcoes a implementar: `PlayBuffer()`, `StopBuffer()`, `LoadWaveFile()`, `PlayMp3()`, `StopMp3()`

#### J. Input de Texto (NAO IMPLEMENTADO)

- Touch → mouse translation funciona
- **TODO**: Soft keyboard Android para campos de texto (login, chat)
- **TODO**: IME input para caracteres especiais
- Funcoes afetadas: `ImmGetContext()`, `ImmSetCompositionWindow()`

#### K. Threading (PARCIAL)

- `CRITICAL_SECTION` → `pthread_mutex_t` (funcional)
- **TODO**: `CreateThread()` → `std::thread` ou `pthread_create()` (~20 arquivos usam CreateThread)
- **TODO**: Verificar thread safety em operacoes de rede

#### L. Inline Assembly (PARCIAL)

- ~12 arquivos com `__asm` blocks
- **TODO**: Substituir por equivalentes C/C++ ou intrinsics ARM
- Maioria e optimization code que pode ser simplificado

#### M. CSimpleModulus Crypto (STUB)

- Atualmente pass-through (sem criptografia real)
- **TODO**: Implementar crypto real se o servidor exigir
- Arquivos: `Dependencies/SimpleModulus/`

#### N. Registry (STUB)

- `CRegKey` retorna defaults
- **TODO**: Substituir por SharedPreferences ou arquivo local se necessario

---

## Estrutura de Diretorios

```
Source/Main/
├── android/                          # Build Android
│   ├── app/src/main/
│   │   ├── cpp/
│   │   │   ├── CMakeLists.txt       # ~493 source files
│   │   │   └── android_bootstrap.cpp # Entry point (6648 linhas)
│   │   └── AndroidManifest.xml
│   ├── build.gradle
│   └── run_adb_debug.sh             # Build + install + launch
├── source/                           # Codigo fonte do jogo
│   ├── Platform/                     # Camada de abstracao (65 arquivos)
│   ├── ZzzScene.cpp                  # Game loop principal
│   ├── ZzzCharacter.cpp              # Sistema de personagens
│   ├── ZzzObject.cpp                 # Objetos do mundo
│   ├── ZzzLodTerrain.cpp             # Terreno com LOD
│   ├── ZzzTexture.cpp                # Carregamento de texturas
│   ├── ZzzOpenData.cpp               # Carregamento de dados do jogo
│   ├── GlobalBitmap.cpp              # Gerenciador de texturas (OZJ/OZT)
│   ├── WSclient.cpp                  # Protocolo de rede
│   ├── WSctlc.cpp                    # Socket client
│   ├── UIMng.cpp                     # Gerenciador de UI
│   ├── LoginWin.cpp                  # Janela de login
│   ├── ServerSelWin.cpp              # Selecao de servidor
│   ├── CharSelMainWin.cpp            # Selecao de personagem
│   └── ...                           # +700 arquivos
├── MainOLD/                          # Codigo original (referencia)
│   └── source/                       # Identico ao source/ sem Platform/
├── Dependencies/
│   ├── jpeg-9e-src/                  # JPEG 9e source
│   ├── lua-5.1.5/                    # Lua 5.1 source
│   └── include/                      # Headers legados
└── CLAUDE.md                         # Instrucoes do projeto
```

## Comandos de Build e Debug

```bash
# Build Android
cd android && ./gradlew assembleDebug

# Build + Install + Launch
./android/run_adb_debug.sh

# Build limpo com mini-data
MU_ANDROID_MINI_DATA_DIR=/path/to/data ./android/run_adb_debug.sh --clean

# Logs do bootstrap
adb logcat -s MUAndroidBootstrap:I

# Logs de personagem
adb logcat -s mu_android_char:I

# Logs de inicializacao
adb logcat -s mu_android_init:I

# Logs de textura
adb logcat -s MUAndroidTexture:W

# Todos os logs do app
adb logcat -s MUAndroidBootstrap:I mu_android_init:I mu_android_char:I MUAndroidTexture:W
```

## Resumo de Progresso

| Componente | Status | Porcentagem |
|------------|--------|-------------|
| Build System | COMPLETO | 100% |
| Platform Layer (65 files) | COMPLETO | 95% |
| Texture Loading (OZJ/OZT) | COMPLETO | 100% |
| Networking/Protocol | FUNCIONAL | 90% |
| 3D Terrain Rendering | FUNCIONAL | 80% |
| Login/Server Select UI | RESTAURADO | 90% |
| Character Select UI | RESTAURADO | 90% |
| Text Rendering | IMPLEMENTADO | 80% |
| 3D Model Rendering | PARCIAL | 60% |
| Gameplay (MAIN_SCENE) | NAO INICIADO | 5% |
| Audio | NAO IMPLEMENTADO | 0% |
| Soft Keyboard Input | NAO IMPLEMENTADO | 0% |

**Estimativa geral: ~60% do port completo**

A UI original do login/servidor/personagem foi restaurada. Text rendering implementado via stb_truetype (fontes do sistema Android). Proximo passo: testar fluxo completo no dispositivo e verificar rendering de modelos 3D.

# Port para Mobile com OpenGL ES

## Resumo

Este cliente nao eh apenas um jogo com OpenGL legado. Ele eh um cliente Windows x86 completo, com:

- loop principal em `Win32`
- contexto OpenGL criado por `wgl`
- input baseado em `WndProc`, hooks e `VK_*`
- texto e caixas de input baseados em `GDI` e `IME`
- audio via `DirectSound` e `wzAudio`
- rede baseada em `Winsock` + `WSAAsyncSelect`
- varias suposicoes de `32 bits`

Conclusao objetiva:

- converter para mobile mantendo o comportamento do PC eh viavel
- converter "direto" para OpenGL ES nao eh viavel sem antes separar a camada de plataforma
- o caminho correto eh preservar a logica do jogo e trocar a base Win32 por uma `platform layer`

## Como o cliente funciona hoje

### 1. Entrada, janela e contexto grafico

O bootstrap inteiro fica em `source/Winmain.cpp`.

- cria `HWND`, `HDC`, `HGLRC`, fontes e estado global
- registra `WndProc`
- cria a janela com `CreateWindow`
- cria o contexto com `ChoosePixelFormat`, `SetPixelFormat`, `wglCreateContext`, `wglMakeCurrent`
- roda um loop com `PeekMessage`, `GetMessage`, `DispatchMessage`

Arquivos principais:

- `source/Winmain.cpp`
- `source/ZzzScene.cpp`
- `source/ZzzOpenglUtil.cpp`

### 2. Render

O renderer atual usa OpenGL de pipeline fixo.

Indicadores encontrados no projeto:

- `glBegin`: 67 ocorrencias
- `glMatrixMode`: 49 ocorrencias
- `glPushMatrix`: 36 ocorrencias
- `glColor*`: 915 ocorrencias
- `glVertex*`: 248 ocorrencias

Isso significa que o renderer atual depende de:

- matriz fixa de OpenGL
- estados globais
- immediate mode
- funcoes nao disponiveis em OpenGL ES da forma atual

Tambem existem chamadas a `gluPerspective`, `glGetFloatv`, `glReadPixels` e manipulacao direta de textura/estado.

### 3. UI e texto

A UI nao eh apenas textura 2D. Ela mistura:

- renderizacao de texto via `GDI`
- `CreateDIBSection`
- `CreateCompatibleDC`
- `TextOut`
- upload do texto para textura OpenGL
- caixas de input baseadas em `CreateWindowW("edit", ...)`
- IME via `ImmGetContext`, `ImmSetConversionStatus`, `ImmSetCompositionWindow`

Indicadores encontrados:

- `g_pRenderText->RenderText(...)`: 955 referencias
- `CheckMouseIn(...)`: 372 referencias
- `IsPress(VK_LBUTTON)`: 79 referencias
- `IsPress(VK_ESCAPE)`: 51 referencias
- chamadas `Imm*`: 60 referencias

Ou seja: a UI inteira assume mouse, teclado e IME do Windows.

### 4. Input

O input usa varias camadas ao mesmo tempo:

- mensagens Win32 em `WndProc`
- estado global de mouse (`MouseX`, `MouseY`, `MouseLButton`, etc.)
- hooks com `SetWindowsHookEx`
- consulta de cursor com `GetCursorPos` e `ScreenToClient`
- mapeamento em cima de `VK_*`

Isso eh bom para o port porque a logica do jogo ja consome eventos abstratos como "mouse esquerdo", "scroll", "tecla". Entao podemos criar um tradutor toque -> mouse/teclas.

### 5. Audio

O projeto usa dois caminhos:

- `DirectSound` para efeitos e 3D sound
- `wzAudio` para musicas MP3

O backend atual depende de:

- `DirectSoundCreate`
- `IDirectSoundBuffer`
- `IDirectSound3DBuffer`
- `IDirectSound3DListener`

Isso nao existe no mobile do jeito atual.

### 6. Rede

A rede usa Winsock classico:

- `WSAStartup`
- `socket`
- `connect`
- `WSAAsyncSelect`
- eventos processados dentro do `WndProc`

Em mobile isso precisa sair do loop de mensagens Win32 e virar:

- socket nao bloqueante com `poll`/`select`/thread
- ou backend de rede encapsulado

### 7. Restricoes de arquitetura

O projeto hoje esta travado em Win32/x86:

- `Main.vcxproj` so tem configuracoes `Win32`
- `TargetMachine` esta em `MachineX86`
- existem casts de ponteiro para `DWORD`
- existem trechos com `_asm`

Esses pontos quebram naturalmente num alvo ARM64.

## Principais bloqueios para OpenGL ES

### Bloqueio A: renderer legado

OpenGL ES nao aceita o pipeline fixo do jeito atual. O renderer precisa de uma camada nova.

Nao recomendo reescrever tudo de uma vez. O caminho mais seguro eh:

1. criar uma API interna de render
2. manter um backend `OpenGL Win32` temporario
3. adicionar um backend `OpenGL ES`
4. migrar modulo por modulo

### Bloqueio B: texto e input de Windows

O sistema de texto atual depende de `GDI` e `IME` do Windows. Em mobile isso precisa virar:

- atlas de fonte
- shaping/layout simplificado ou biblioteca especifica
- teclado virtual do sistema
- composicao de texto controlada pela camada mobile

### Bloqueio C: audio

`DirectSound` e `wzAudio` devem ser trocados por um backend abstrato.

### Bloqueio D: loop principal acoplado ao WndProc

Hoje o cliente mistura:

- eventos de janela
- eventos de rede
- render
- timers do sistema

Isso precisa ser reorganizado para um loop de app independente de Win32.

### Bloqueio E: x86 e 32 bits

Antes de mirar Android/iOS, precisamos remover:

- casts `pointer -> DWORD`
- inline assembly
- dependencias de `MachineX86`

## Estrategia recomendada

### Fase 1. Separar a camada de plataforma sem mudar o jogo

Criar interfaces para:

- `PlatformApp`
- `PlatformWindow`
- `PlatformGLContext`
- `PlatformInput`
- `PlatformAudio`
- `PlatformTextInput`
- `PlatformNetwork`
- `PlatformStorage`

Objetivo:

- o jogo continua rodando no Windows
- mas para de chamar Win32 diretamente em todo lugar

### Fase 2. Criar um renderer compativel com OpenGL ES

Recomendacao:

- usar OpenGL ES 3.x como alvo principal
- introduzir shaders, buffers e estados explicitamente
- manter a mesma logica visual e os mesmos assets

Primeiro migrar:

- bitmap/UI quads
- texto
- sprites
- estados basicos de blend/depth

Depois migrar:

- BMD/modelos
- efeitos
- terrenos
- agua/fog/post-process

### Fase 3. Tradutor de toque para comportamento de PC

Para deixar "igual ao PC", o ideal nao eh redesenhar tudo no inicio. O ideal eh:

- manter a UI em coordenadas logicas de PC
- criar uma resolucao virtual
- traduzir toque para `mouse move`, `left click`, `right click`, `scroll`, `drag`
- mapear botoes virtuais para atalhos importantes

Assim o jogo continua com a mesma regra de clique e a mesma logica de tela.

### Fase 4. Audio, rede e teclado virtual

Trocar os backends sem mexer na logica do jogo:

- efeitos sonoros
- musica
- sockets
- text input / teclado virtual

### Fase 5. Android primeiro, iOS depois

Android eh o alvo natural para validar:

- desempenho
- input por toque
- compatibilidade de assets
- backend OpenGL ES

Depois disso fica muito mais seguro abrir o backend iOS.

## O que eu faria primeiro neste repositorio

Ordem sugerida:

1. Introduzir uma `platform layer` no proprio cliente Windows.
2. Remover dependencias diretas de `HWND/HDC/HGLRC` fora do backend.
3. Encapsular input global de mouse/teclado.
4. Encapsular audio.
5. Encapsular rede.
6. Substituir o sistema de texto baseado em `GDI`.
7. So depois abrir o backend OpenGL ES.

## Avaliacao final

Começar por partes:
- Select Server
- Login
- Select Personagem
- Gameplay

Se o objetivo for:

- mesmo gameplay
- mesmas telas
- mesma logica
- mesma aparencia geral do cliente PC

entao eu recomendo um port por compatibilidade, nao um remake.

Em outras palavras:

- reaproveitar o core do cliente
- trocar a base Win32
- trocar o backend grafico
- trocar input/audio/texto/rede por adaptadores

## Proximo passo pratico

O melhor proximo passo tecnico eh comecar a Fase 1:

- criar a estrutura da `platform layer`
- fazer o cliente Windows passar por ela
- sem quebrar o jogo atual

Depois disso, abrir o backend mobile com OpenGL ES deixa de ser um salto no escuro.

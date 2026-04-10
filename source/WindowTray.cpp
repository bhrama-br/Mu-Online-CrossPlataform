#include "stdafx.h"
#include "WindowTray.h"
#include "resource.h"
//#include <Shellapi.h>

//#pragma comment (lib, "Shell32.lib")

TrayMode gTrayMode;

void TrayMode::Load()
{
	this->TempIconProc = NULL;
	this->InTray = false;
	this->LastPress = GetTickCount();
}

void TrayMode::SwitchState()
{
	if (IsWindowVisible(g_hWnd))
	{
		if (GetForegroundWindow() == g_hWnd)
		{
			ShowWindow(g_hWnd, SW_HIDE);
			UpdateWindow(g_hWnd);
			if (!this->InTray) {
				this->ShowNotify(true);
			}
			this->ShowMessage(NIIF_INFO, gProtect->m_MainInfo.WindowName, "");
		}
	}
	else
	{
		ShowWindow(g_hWnd, SW_SHOW);
		UpdateWindow(g_hWnd);
		this->ShowNotify(false);
	}
}

void TrayMode::ShowNotify(bool Mode)
{
	this->InTray = Mode;
	NOTIFYICONDATA Icon = { 0 };

	Icon.cbSize = sizeof(NOTIFYICONDATA);
	Icon.uID = TRAYMODE_ICON_ID;
	Icon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	Icon.hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	this->TempIconProc = Icon.hIcon;
	Icon.hWnd = g_hWnd;
	Icon.uCallbackMessage = TRAYMODE_ICON_MESSAGE;

	strcpy_s(Icon.szTip, sizeof(Icon.szTip), gProtect->m_MainInfo.WindowName);
	strcat_s(Icon.szTip, "");

	if (Mode)
	{
		Shell_NotifyIcon(NIM_ADD, &Icon);
	}
	else
	{
		Shell_NotifyIcon(NIM_DELETE, &Icon);
	}

	DeleteObject(Icon.hIcon);
}

void TrayMode::ShowMessage(DWORD Type, char * Title, char * Message)
{
	NOTIFYICONDATA Icon = { 0 };

	Icon.cbSize = sizeof(NOTIFYICONDATA);
	Icon.uID = TRAYMODE_ICON_ID;
	Icon.hWnd = g_hWnd;
	Icon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_INFO;
	Icon.hIcon = this->TempIconProc;
	Icon.uCallbackMessage = TRAYMODE_ICON_MESSAGE;
	Icon.dwInfoFlags = Type;
	Icon.uTimeout = 5000;

	strcpy_s(Icon.szInfo, Message);
	strcpy_s(Icon.szInfoTitle, Title);

	Shell_NotifyIcon(NIM_MODIFY, &Icon);
}

void TrayMode::ShowMessageLua(DWORD Type, char * Title, char * Message)
{
	if (!this->InTray) {
		this->ShowNotify(true);
	}
	this->ShowMessage(Type, Title, Message);
}


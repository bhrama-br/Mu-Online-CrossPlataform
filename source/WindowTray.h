#pragma once

#define TRAYMODE_ICON_ID		0x444
#define TRAYMODE_ICON_MESSAGE	0x445

class TrayMode
{
public:
	void	Load();
	void	SwitchState();
	void	ShowNotify(bool Mode);
	void	ShowMessage(DWORD Type, char * Title, char * Message);
	void	ShowMessageLua(DWORD Type, char * Title, char * Message);

	HICON	TempIconProc;
	bool	InTray;

	DWORD	LastPress;
};

extern TrayMode gTrayMode;
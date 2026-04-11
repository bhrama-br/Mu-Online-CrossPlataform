//*****************************************************************************
// File: LoginWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "LoginWin.h"
#include "Input.h"
#include "UIMng.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "UIControls.h"
#include "ZzzScene.h"
#include "wsclientinline.h"
#include "DSPlaySound.h"
#include "./Utilities/Log/muConsoleDebug.h"
#include "ProtocolSend.h"
#include "ServerListManager.h"
#include "Widescreen.h"

#define	LIW_ACCOUNT		0
#define	LIW_PASSWORD	1

#define LIW_OK			0
#define LIW_CANCEL		1

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
extern int g_iChatInputType;

CLoginWin::CLoginWin()
{
	m_pIDInputBox = NULL;
	m_pPassInputBox = NULL;
}

CLoginWin::~CLoginWin()
{
	SAFE_DELETE(m_pIDInputBox);
	SAFE_DELETE(m_pPassInputBox);
}

void CLoginWin::Create()
{
	CWin::Create(329, 245, -2);

	m_asprInputBox[LIW_ACCOUNT].Create(156, 23, BITMAP_LOG_IN + 8);
	m_asprInputBox[LIW_PASSWORD].Create(156, 23, BITMAP_LOG_IN + 8);

	for (int i = 0; i < 2; ++i)
	{
		m_aBtn[i].Create(54, 30, BITMAP_BUTTON + i, 3, 2, 1);
		CWin::RegisterButton(&m_aBtn[i]);
	}

	char Acc[32], Pass[32];
	GetPrivateProfileString("AutoLogin", "User", "", Acc, 32, ".\\config.ini");
	GetPrivateProfileString("AutoLogin", "Password", "", Pass, 32, ".\\config.ini");

	SAFE_DELETE(m_pIDInputBox);

	m_pIDInputBox = new CUITextInputBox;
	m_pIDInputBox->Init(g_hWnd, 140, 14, MAX_ID_SIZE);
	m_pIDInputBox->SetBackColor(0, 0, 0, 255);
	m_pIDInputBox->SetTextColor(255, 255, 230, 210);
	m_pIDInputBox->SetFont(g_hFixFont);
	m_pIDInputBox->SetState(UISTATE_NORMAL);
	m_pIDInputBox->SetText(Acc);

	SAFE_DELETE(m_pPassInputBox);

	m_pPassInputBox = new CUITextInputBox;
	m_pPassInputBox->Init(g_hWnd, 140, 14, MAX_PASSWORD_SIZE, TRUE);
	m_pPassInputBox->SetBackColor(0, 0, 0, 25);
	m_pPassInputBox->SetTextColor(255, 255, 230, 210);
	m_pPassInputBox->SetFont(g_hFixFont);
	m_pPassInputBox->SetState(UISTATE_NORMAL);
	m_pPassInputBox->SetText(Pass);

	m_pIDInputBox->SetTabTarget(m_pPassInputBox);
	m_pPassInputBox->SetTabTarget(m_pIDInputBox);
	
	this->FirstLoad = 1;
}

void CLoginWin::PreRelease()
{
	for (int i = 0; i < 2; ++i)
		m_asprInputBox[i].Release();
}

void CLoginWin::SetPosition(int nXCoord, int nYCoord)
{
	int cx = nXCoord - 61; int cy = 0;

	CWin::SetPosition(nXCoord, nYCoord);

	m_asprInputBox[LIW_ACCOUNT].SetPosition(nXCoord + 109, nYCoord + 106);
	m_asprInputBox[LIW_PASSWORD].SetPosition(nXCoord + 109, nYCoord + 131);

	if (g_iChatInputType == 1)
	{
		m_pIDInputBox->SetPosition(int((m_asprInputBox[LIW_ACCOUNT].GetXPos() + 6) / g_fScreenRate_x), int((m_asprInputBox[LIW_ACCOUNT].GetYPos() + 6) / g_fScreenRate_y));

		m_pPassInputBox->SetPosition(int((m_asprInputBox[LIW_PASSWORD].GetXPos() + 6) / g_fScreenRate_x), int((m_asprInputBox[LIW_PASSWORD].GetYPos() + 24) / g_fScreenRate_y));
	}

	cx = (int)(WindowWidth / g_fScreenRate_x) / 2 - 95;

	cy = (int)(WindowHeight / g_fScreenRate_y) / 2 - 125;
}

void CLoginWin::Show(bool bShow)
{
	CWin::Show(bShow);

	for (int i = 0; i < 2; ++i)
	{
		m_asprInputBox[i].Show(bShow);
	}
}

bool CLoginWin::CursorInWin(int nArea)
{
	if (!CWin::m_bShow)
		return false;

	switch (nArea)
	{
	case WA_MOVE:
		return false;
	}

	return CWin::CursorInWin(nArea);
}

void CLoginWin::UpdateWhileActive(double dDeltaTick)
{
	CInput& rInput = CInput::Instance();

	if (CInput::Instance().IsKeyDown(VK_RETURN))
	{
		::PlayBuffer(SOUND_CLICK01);
		RequestLogin();
	}
	else if (CInput::Instance().IsKeyDown(VK_ESCAPE))
	{
		::PlayBuffer(SOUND_CLICK01);
		CancelLogin();
		CUIMng::Instance().SetSysMenuWinShow(false);
	}
}

void CLoginWin::UpdateWhileShow(double dDeltaTick)
{
	m_pIDInputBox->DoAction();
	m_pPassInputBox->DoAction();
}

void CLoginWin::RenderControls()
{
	if (this->FirstLoad == 1)
	{
		if (strlen(m_ID) > 0)
			CUIMng::Instance().m_LoginWin.GetPassInputBox()->GiveFocus();
		else
			CUIMng::Instance().m_LoginWin.GetIDInputBox()->GiveFocus();
		this->FirstLoad = 0;
	}

	RenderBitmap(BITMAP_LOG_IN+7, (GetWindowsX / 2) - 100, int(CWin::GetYPos() / g_fScreenRate_y) + 30, 200, 137, 0, 0, 0.6492025, 0.894812, 1, 1, 0.0);

	GWidescreen.RenderImageF(931316, (GetWindowsX / 2) - 40, int((CWin::GetYPos() + 101) / g_fScreenRate_y), 114.f, 21.f, 0.0, 0.0, 256.f, 46.5f);

	GWidescreen.RenderImageF(931316, (GetWindowsX / 2) - 40, int((CWin::GetYPos() + 142) / g_fScreenRate_y), 114.f, 21.f, 0.0, 0.0, 256.f, 46.5f);

	m_pIDInputBox->Render();
	m_pPassInputBox->Render();

	int buttonPosY = int((CWin::GetYPos() + 186) / g_fScreenRate_y);

	if (CheckMouseIn((GetWindowsX / 2) - 70, buttonPosY, 60, 18))
	{
		if (MouseLButtonPush)
		{
			MouseLButtonPush = false;

			MouseUpdateTime = 0;

			MouseUpdateTimeMax = 6;

			RequestLogin();
		}
		::RenderBitmap(BITMAP_LOG_IN + 8, (GetWindowsX / 2) - 70, buttonPosY, 60, 18, 0.002, 0.213, 0.95, 0.20, 1, 1, 1.0);
	}
	else 
	{
		::RenderBitmap(BITMAP_LOG_IN + 8, (GetWindowsX / 2) - 70, buttonPosY, 60, 18, 0.002, 0.002, 0.95, 0.20, 1, 1, 1.0);
	}

	if (CheckMouseIn((GetWindowsX / 2) + 12, buttonPosY, 60, 18))
	{
		if (MouseLButtonPush)
		{
			MouseLButtonPush = false;

			MouseUpdateTime = 0;

			MouseUpdateTimeMax = 6;

			CancelLogin();
		}
		::RenderBitmap(BITMAP_LOG_IN + 8, (GetWindowsX / 2) + 12, buttonPosY, 60, 18, 0.002, 0.213, 0.95, 0.20, 1, 1, 1.0);
	}
	else {
		::RenderBitmap(BITMAP_LOG_IN + 8, (GetWindowsX / 2) + 12, buttonPosY, 60, 18, 0.002, 0.002, 0.95, 0.20, 1, 1, 1.0);
	}

	g_pRenderText->SetFont(g_hFixFont);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(237, 214, 161, 255);
	g_pRenderText->RenderText((GetWindowsX / 2) - 70, buttonPosY + ((18 / 2) - 5), GlobalText[452], 60, 0, 3);
	g_pRenderText->RenderText((GetWindowsX / 2) + 12, buttonPosY + ((18 / 2) - 5), GlobalText[453], 60, 0, 3);

	m_pIDInputBox->Render();
	m_pPassInputBox->Render();

	g_pRenderText->SetFont(g_hFixFont);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(CLRDW_WHITE);
	g_pRenderText->RenderText(int((CWin::GetXPos() + 45) / g_fScreenRate_x), int((CWin::GetYPos() + 110) / g_fScreenRate_y), GlobalText[450]);
	g_pRenderText->RenderText(int((CWin::GetXPos() + 45) / g_fScreenRate_x), int((CWin::GetYPos() + 149) / g_fScreenRate_y), GlobalText[451]);

	unicode::t_char szServerName[MAX_TEXT_LENGTH];

	const char* apszGlobalText[4] = { GlobalText[461], GlobalText[460], GlobalText[3130], GlobalText[3131] };
	sprintf(szServerName, "%s", g_ServerListManager->GetSelectServerName());

	g_pRenderText->RenderText((GetWindowsX / 2) - 100, int((CWin::GetYPos() + 60) / g_fScreenRate_y), szServerName, 200, 0, 3);
	g_pRenderText->SetTextColor(CLRDW_WHITE);
}

void CLoginWin::RequestLogin()
{
	if (CurrentProtocolState == REQUEST_JOIN_SERVER)
		return;

	CUIMng::Instance().HideWin(this);

	char szID[MAX_ID_SIZE+1] = { 0, };
	char szPass[MAX_PASSWORD_SIZE+1] = {0, };
	m_pIDInputBox->GetText(szID, MAX_ID_SIZE+1);
	m_pPassInputBox->GetText(szPass, MAX_PASSWORD_SIZE+1);

	if (unicode::_strlen(szID) <= 0)
		CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_ID);
	else if (unicode::_strlen(szPass) <= 0)
		CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_PASSWORD);
	else
	{
		if (CurrentProtocolState == RECEIVE_JOIN_SERVER_SUCCESS)
		{
			g_ConsoleDebug->Write( MCD_NORMAL, "Login with the following account: %s", szID);

			g_ErrorReport.Write("> Login Request.\r\n");
			g_ErrorReport.Write("> Try to Login \"%s\"\r\n", szID);
			SendRequestLogIn(szID, szPass);
		}
	}
}

void CLoginWin::CancelLogin()
{
	ConnectConnectionServer();
	CUIMng::Instance().HideWin(this);
}

void CLoginWin::ConnectConnectionServer()
{
	LogIn = 0;
	CurrentProtocolState = REQUEST_JOIN_SERVER;
    CreateSocket(szServerIpAddress, g_ServerPort);
}

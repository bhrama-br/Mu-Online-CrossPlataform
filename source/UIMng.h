//*****************************************************************************
// File: UIMng.h
//*****************************************************************************
#pragma once

#include "PList.h"
#if !defined(__ANDROID__)
#include "MsgWin.h"
#include "SysMenuWin.h"
#include "OptionWin.h"
#endif
#include "LoginMainWin.h"
#include "ServerSelWin.h"
#include "LoginWin.h"
#if !defined(__ANDROID__)
#include "CreditWin.h"
#endif
#if !defined(__ANDROID__)
#include "CharSelMainWin.h"
#include "CharMakeWin.h"
#include "CharInfoBalloonMng.h"
#include "ServerMsgWin.h"
#endif

#define	UIM_SCENE_NONE			0
#define	UIM_SCENE_TITLE			1
#define UIM_SCENE_LOGIN			2
#define	UIM_SCENE_LOADING		3
#define	UIM_SCENE_CHARACTER		4
#define	UIM_SCENE_MAIN			5

class CSprite;
class CGaugeBar;
class CWin;
class CLoadingScene;

class CUIMng  
{
public:
#if !defined(__ANDROID__)
	CMsgWin			m_MsgWin;
	CSysMenuWin		m_SysMenuWin;
	COptionWin		m_OptionWin;
#endif
	CLoginMainWin	m_LoginMainWin;
	CServerSelWin	m_ServerSelWin;
	CLoginWin		m_LoginWin;
#if !defined(__ANDROID__)
	CCreditWin		m_CreditWin;
#endif
#if !defined(__ANDROID__)
	CCharSelMainWin	m_CharSelMainWin;
	CCharMakeWin	m_CharMakeWin;
	CCharInfoBalloonMng	m_CharInfoBalloonMng;
	CServerMsgWin	m_ServerMsgWin;
#endif
	CLoadingScene*	m_pLoadingScene;

protected:
	CSprite*		m_asprTitle;
	CGaugeBar*		m_pgbLoding;
	CPList			m_WinList;
	bool			m_bCursorOnUI;
	bool			m_bBlockCharMove;
	int				m_nScene;
	bool			m_bWinActive;
	bool			m_bSysMenuWinShow;
#ifdef MOVIE_DIRECTSHOW
	bool			m_bMoving;
#endif // MOVIE_DIRECTSHOW

public:
	virtual ~CUIMng();

	static CUIMng& Instance();

	void CreateTitleSceneUI();
	void ReleaseTitleSceneUI();
	void RenderTitleSceneUI(HDC hDC, DWORD dwNow, DWORD dwTotal);
	void Create();
	void Release();
	void CreateLoginScene();
	void CreateCharacterScene();
	void CreateMainScene();
	void Update(double dDeltaTick);
	void Render();

	void ShowWin(CWin* pWin);
	void HideWin(CWin* pWin);

	bool IsCursorOnUI() { return m_bCursorOnUI; }
	void PopUpMsgWin(int nMsgCode, char* pszMsg = NULL);
	void AddServerMsg(char* pszMsg);
#if defined(__ANDROID__)
	void CloseMsgWin() {}
#else
	void CloseMsgWin() { HideWin(&m_MsgWin); }
#endif
	void SetSysMenuWinShow(bool bShow) { m_bSysMenuWinShow = bShow; }
	bool IsSysMenuWinShow() { return m_bSysMenuWinShow; };

#ifdef MOVIE_DIRECTSHOW
	void SetMoving(bool bMoving) { m_bMoving = bMoving; };
	bool IsMoving() { return m_bMoving; };
#endif // MOVIE_DIRECTSHOW

protected:
	CUIMng();

	void RemoveWinList();
	CWin* SetActiveWin(CWin* pWin);
	void CheckDockWin();
	bool SetDockWinPosition(CWin* pMoveWin, int nDockX, int nDockY);
};

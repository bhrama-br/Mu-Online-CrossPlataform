//*****************************************************************************
// File: Input.h
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_INPUT_H__56D42544_2BF6_4B84_B368_F36CACDFEF1E__INCLUDED_)
#define AFX_INPUT_H__56D42544_2BF6_4B84_B368_F36CACDFEF1E__INCLUDED_

#pragma once

class CInput
{
protected:
	HWND	m_hWnd;

	POINT	m_ptCursor;
	long	m_lDX;
	long	m_lDY;
//	long	m_lDZ;
	bool	m_bLBtnDn;
	bool	m_bLBtnHeldDn;
	bool	m_bLBtnUp;
	bool	m_bLBtnDbl;	
	bool	m_bRBtnDn;
	bool	m_bRBtnHeldDn;
	bool	m_bRBtnUp;
	bool	m_bRBtnDbl;
	bool	m_bMBtnDn;
	bool	m_bMBtnHeldDn;
	bool	m_bMBtnUp;
	bool	m_bMBtnDbl;

	long	m_lScreenWidth;	
	long	m_lScreenHeight;
	bool	m_bLeftHand;
	bool	m_bTextEditMode;

	POINT	m_ptFormerCursor;
	double	m_dDoubleClickTime;
	double	m_dBtn0LastClickTime;
	double	m_dBtn1LastClickTime;
	double	m_dBtn2LastClickTime;
	bool	m_bFormerBtn0Dn;
	bool	m_bFormerBtn1Dn;
	bool	m_bFormerBtn2Dn;

#if defined(__ANDROID__)
	bool	m_abKeyPressed[256];
	bool	m_abKeyHeld[256];
#endif
	
protected:
	CInput();

public:
	virtual ~CInput();

	static CInput& Instance();
	bool Create(HWND hWnd, long lScreenWidth, long lScreenHeight);
	void Update();

	bool IsKeyDown(int nVirtualKeyCode)
	{
		if (m_bTextEditMode)
			return false;
#if defined(__ANDROID__)
		if (nVirtualKeyCode < 0 || nVirtualKeyCode >= static_cast<int>(sizeof(m_abKeyPressed)))
			return false;
		return m_abKeyPressed[nVirtualKeyCode];
#else
		return SEASON3B::IsPress(nVirtualKeyCode);
#endif
	}
	bool IsKeyHeldDown(int nVirtualKeyCode)
	{
		if (m_bTextEditMode)
			return false;
#if defined(__ANDROID__)
		if (nVirtualKeyCode < 0 || nVirtualKeyCode >= static_cast<int>(sizeof(m_abKeyHeld)))
			return false;
		return m_abKeyHeld[nVirtualKeyCode];
#else
		return SEASON3B::IsRepeat(nVirtualKeyCode);
#endif
	}

	POINT GetCursorPos(){ return m_ptCursor; }
	long GetCursorX(){ return m_ptCursor.x; }
	long GetCursorY(){ return m_ptCursor.y; }
	long GetDX(){ return m_lDX; }
	long GetDY(){ return m_lDY; }
	bool IsLBtnDn(){ return m_bLBtnDn; }
	bool IsLBtnHeldDn(){ return m_bLBtnHeldDn; }
	bool IsLBtnUp(){ return m_bLBtnUp; }
	bool IsLBtnDbl(){ return m_bLBtnDbl; }
	bool IsRBtnDn(){ return m_bRBtnDn; }
	bool IsRBtnHeldDn(){ return m_bRBtnHeldDn; }
	bool IsRBtnUp(){ return m_bRBtnUp; }
	bool IsRBtnDbl(){ return m_bRBtnDbl; }
	bool IsMBtnDn(){ return m_bMBtnDn; }
	bool IsMBtnHeldDn(){ return m_bMBtnHeldDn; }
	bool IsMBtnUp(){ return m_bMBtnUp; }
	bool IsMBtnDbl(){ return m_bMBtnDbl; }

	long GetScreenWidth(){ return m_lScreenWidth; }
	long GetScreenHeight(){ return m_lScreenHeight; }
	void SetLeftHandMode(bool bLeftHand){ m_bLeftHand = bLeftHand; }
	bool IsLeftHandMode(){ return m_bLeftHand; }
	void SetTextEditMode(bool bTextEditMode){ m_bTextEditMode = bTextEditMode; }
	bool IsTextEditMode(){ return m_bTextEditMode; }

#if defined(__ANDROID__)
	void AndroidConfigure(HWND hWnd, long lScreenWidth, long lScreenHeight);
	void AndroidSyncMouseState(const POINT& cursor, long dx, long dy,
		bool lbtn_dn, bool lbtn_held, bool lbtn_up, bool lbtn_dbl,
		bool rbtn_dn, bool rbtn_held, bool rbtn_up, bool rbtn_dbl,
		bool mbtn_dn, bool mbtn_held, bool mbtn_up, bool mbtn_dbl);
	void AndroidSetKeyState(int nVirtualKeyCode, bool pressed, bool held_down);
	void AndroidClearKeyState(int nVirtualKeyCode);
#endif
};

#endif

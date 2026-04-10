//*****************************************************************************
// File: Input.cpp
//*****************************************************************************

#include "stdafx.h"
#include "Input.h"
#include "UsefulDef.h"
#include "./Time/Timer.h"
#if	defined WINDOWMODE && !defined(__ANDROID__)
#include "UIManager.h"
extern bool g_bWndActive;
#endif
#if !defined(__ANDROID__)
extern CTimer*	g_pTimer;
#endif

CInput::CInput()
{
	m_hWnd = NULL;
	m_ptCursor.x = m_ptCursor.y = 0;
	m_lDX = m_lDY = 0L;
	m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnUp = m_bLBtnDbl = false;
	m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnUp = m_bRBtnDbl = false;
	m_bMBtnDn = m_bMBtnHeldDn = m_bMBtnUp = m_bMBtnDbl = false;
	m_lScreenWidth = 0;
	m_lScreenHeight = 0;
	m_bLeftHand = false;
	m_bTextEditMode = false;
	m_ptFormerCursor.x = m_ptFormerCursor.y = 0;
	m_dDoubleClickTime = 0.0;
	m_dBtn0LastClickTime = m_dBtn1LastClickTime = m_dBtn2LastClickTime = 0.0;
	m_bFormerBtn0Dn = m_bFormerBtn1Dn = m_bFormerBtn2Dn = false;
#if defined(__ANDROID__)
	memset(m_abKeyPressed, 0, sizeof(m_abKeyPressed));
	memset(m_abKeyHeld, 0, sizeof(m_abKeyHeld));
#endif
}

CInput::~CInput()
{

}

CInput& CInput::Instance()
{
	static CInput s_Input;                  
    return s_Input;
}

bool CInput::Create(HWND hWnd, long lScreenWidth, long lScreenHeight)
{
	if (NULL == hWnd)
		return false;

	m_hWnd = hWnd;

	m_lScreenWidth = lScreenWidth;
	m_lScreenHeight = lScreenHeight;

	::GetCursorPos(&m_ptCursor);
	::ScreenToClient(m_hWnd, &m_ptCursor);
	m_ptFormerCursor = m_ptCursor;

	m_bLeftHand = false;

	m_dDoubleClickTime = (double)::GetDoubleClickTime();

	m_dBtn0LastClickTime = m_dBtn1LastClickTime = m_dBtn2LastClickTime = 0.0;

	m_bFormerBtn0Dn = m_bFormerBtn1Dn = m_bFormerBtn2Dn = false;

	m_bLBtnHeldDn = m_bRBtnHeldDn = m_bMBtnHeldDn = false;

	m_bTextEditMode	= false;

	return true;
}

#if defined(__ANDROID__)
void CInput::AndroidConfigure(HWND hWnd, long lScreenWidth, long lScreenHeight)
{
	if (hWnd != NULL)
	{
		m_hWnd = hWnd;
	}

	m_lScreenWidth = lScreenWidth;
	m_lScreenHeight = lScreenHeight;

	if (m_ptCursor.x >= m_lScreenWidth)
		m_ptCursor.x = std::max<long>(0, m_lScreenWidth - 1);
	if (m_ptCursor.y >= m_lScreenHeight)
		m_ptCursor.y = std::max<long>(0, m_lScreenHeight - 1);
}

void CInput::AndroidSyncMouseState(const POINT& cursor, long dx, long dy,
	bool lbtn_dn, bool lbtn_held, bool lbtn_up, bool lbtn_dbl,
	bool rbtn_dn, bool rbtn_held, bool rbtn_up, bool rbtn_dbl,
	bool mbtn_dn, bool mbtn_held, bool mbtn_up, bool mbtn_dbl)
{
	m_ptFormerCursor = m_ptCursor;
	m_ptCursor = cursor;
	m_lDX = dx;
	m_lDY = dy;
	m_bLBtnDn = lbtn_dn;
	m_bLBtnHeldDn = lbtn_held;
	m_bLBtnUp = lbtn_up;
	m_bLBtnDbl = lbtn_dbl;
	m_bRBtnDn = rbtn_dn;
	m_bRBtnHeldDn = rbtn_held;
	m_bRBtnUp = rbtn_up;
	m_bRBtnDbl = rbtn_dbl;
	m_bMBtnDn = mbtn_dn;
	m_bMBtnHeldDn = mbtn_held;
	m_bMBtnUp = mbtn_up;
	m_bMBtnDbl = mbtn_dbl;
}

void CInput::AndroidSetKeyState(int nVirtualKeyCode, bool pressed, bool held_down)
{
	if (nVirtualKeyCode < 0 || nVirtualKeyCode >= static_cast<int>(sizeof(m_abKeyPressed)))
		return;

	m_abKeyPressed[nVirtualKeyCode] = pressed;
	m_abKeyHeld[nVirtualKeyCode] = held_down;
}

void CInput::AndroidClearKeyState(int nVirtualKeyCode)
{
	if (nVirtualKeyCode < 0 || nVirtualKeyCode >= static_cast<int>(sizeof(m_abKeyPressed)))
		return;

	m_abKeyPressed[nVirtualKeyCode] = false;
	m_abKeyHeld[nVirtualKeyCode] = false;
}
#endif

void CInput::Update()
{
#if defined(__ANDROID__)
	return;
#else
	m_lDX = m_lDY = 0L;
	m_bLBtnUp = m_bRBtnUp = m_bMBtnUp
		= m_bLBtnDn = m_bRBtnDn = m_bMBtnDn
		= m_bLBtnDbl = m_bRBtnDbl = m_bMBtnDbl = false;

	::GetCursorPos(&m_ptCursor);
	::ScreenToClient(m_hWnd, &m_ptCursor);

	m_ptCursor.x = LIMIT(m_ptCursor.x, 0, m_lScreenWidth - 1);
	m_ptCursor.y = LIMIT(m_ptCursor.y, 0, m_lScreenHeight - 1);

	m_lDX = m_ptCursor.x - m_ptFormerCursor.x;
	m_lDY = m_ptCursor.y - m_ptFormerCursor.y;

	if (m_lDX || m_lDY)
		m_bFormerBtn0Dn = m_bFormerBtn1Dn = m_bFormerBtn2Dn = false;

	m_ptFormerCursor = m_ptCursor;

	double dAbsTime;

	if (SEASON3B::IsPress(VK_LBUTTON))
	{
		dAbsTime = g_pTimer->GetAbsTime();

		if (dAbsTime - m_dBtn0LastClickTime <= m_dDoubleClickTime
			&& m_bFormerBtn0Dn)
		{
			if (m_bLeftHand)
			{
				m_bRBtnDn =	m_bRBtnHeldDn = m_bRBtnDbl = true;
				m_bRBtnUp = false;
			}
			else
			{
				m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = true;
				m_bLBtnUp = false;
			}
			m_bFormerBtn0Dn = false;
		}
		else
		{
			if (m_bLeftHand)
			{
				m_bRBtnDn =	m_bRBtnHeldDn =	true;
				m_bRBtnUp = m_bRBtnDbl = false;
			}
			else
			{
				m_bLBtnDn = m_bLBtnHeldDn = true;
				m_bLBtnUp = m_bLBtnDbl = false;
			}
			m_bFormerBtn0Dn = true;
		}
		m_dBtn0LastClickTime = dAbsTime;
	}
	else if (SEASON3B::IsNone(VK_LBUTTON))
	{
		if (m_bLeftHand)
		{
			if (m_bRBtnHeldDn)
			{
				m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = false;
				m_bRBtnUp = true;
			}
		}
		else
		{
			if (m_bLBtnHeldDn)
			{
				m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = false;
				m_bLBtnUp = true;
			}
		}
	}

	if (SEASON3B::IsPress(VK_RBUTTON))
	{
		dAbsTime = g_pTimer->GetAbsTime();

		if (dAbsTime - m_dBtn1LastClickTime <= m_dDoubleClickTime
			&& m_bFormerBtn1Dn)
		{
			if (m_bLeftHand)
			{
				m_bLBtnDn =	m_bLBtnHeldDn = m_bLBtnDbl = true;
				m_bLBtnUp = false;
			}
			else
			{
				m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = true;
				m_bRBtnUp = false;
			}
			m_bFormerBtn1Dn = false;
		}
		else
		{
			if (m_bLeftHand)
			{
				m_bLBtnDn =	m_bLBtnHeldDn = true;
				m_bLBtnUp = m_bLBtnDbl = false;
			}
			else
			{
				m_bRBtnDn = m_bRBtnHeldDn = true;
				m_bRBtnUp = m_bRBtnDbl = false;
			}
			m_bFormerBtn1Dn = true;
		}
		m_dBtn1LastClickTime = dAbsTime;
	}
	else if (SEASON3B::IsNone(VK_RBUTTON))
	{
		if (m_bLeftHand)
		{
			if (m_bLBtnHeldDn)
			{
				m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = false;
				m_bLBtnUp = true;
			}
		}
		else
		{
			if (m_bRBtnHeldDn)
			{
				m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = false;
				m_bRBtnUp = true;
			}
		}
	}

	if (SEASON3B::IsPress(VK_MBUTTON))
	{
		dAbsTime = g_pTimer->GetAbsTime();

		if (dAbsTime - m_dBtn2LastClickTime <= m_dDoubleClickTime
			&& m_bFormerBtn2Dn)
		{
			m_bMBtnDbl = true;
			m_bFormerBtn2Dn = false;
		}
		else
		{
			m_bFormerBtn2Dn = true;
			m_bMBtnDbl = false;
		}
		m_bMBtnDn = m_bMBtnHeldDn = true;
		m_bMBtnUp = false;
		m_dBtn2LastClickTime = dAbsTime;
	}
	else if (SEASON3B::IsNone(VK_MBUTTON))
	{
		if (m_bMBtnHeldDn)
		{
			m_bMBtnDn = m_bMBtnHeldDn = m_bMBtnDbl = false;
			m_bMBtnUp = true;
		}
	}
#if	defined WINDOWMODE
	if(GetActiveWindow() == NULL)
	{
		m_lDX = m_lDY = 0L;

		m_bLBtnUp = m_bRBtnUp = m_bMBtnUp
			= m_bLBtnDn = m_bRBtnDn = m_bMBtnDn
			= m_bLBtnDbl = m_bRBtnDbl = m_bMBtnDbl = false;

		m_ptCursor.x = m_ptCursor.y = 0;
		return;
	}
#endif
#endif
}

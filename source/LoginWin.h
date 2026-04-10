//*****************************************************************************
// File: LoginWin.h
//*****************************************************************************
#pragma once

#include "Win.h"

#include "Button.h"

class CUITextInputBox;

class CLoginWin : public CWin  
{
protected:
	CSprite		m_asprInputBox[2];
	CButton		m_aBtn[2];
	CUITextInputBox*	m_pIDInputBox, * m_pPassInputBox;

public:
	CLoginWin();
	virtual ~CLoginWin();
	void Create();
	void SetPosition(int nXCoord, int nYCoord);
	void Show(bool bShow);
	bool CursorInWin(int nArea);

	void ConnectConnectionServer();

	CUITextInputBox* GetIDInputBox() const { return m_pIDInputBox; }
	CUITextInputBox* GetPassInputBox() const { return m_pPassInputBox; }
#if defined(__ANDROID__)
	const CSprite* GetInputBoxSprite(int index) const
	{
		return (index >= 0 && index < 2) ? &m_asprInputBox[index] : NULL;
	}
#endif

	void CancelLogin();
private:
	int FirstLoad = 0;

protected:
	void PreRelease();
	void UpdateWhileActive(double dDeltaTick);
	void UpdateWhileShow(double dDeltaTick);
	void RenderControls();
	void RequestLogin();
};

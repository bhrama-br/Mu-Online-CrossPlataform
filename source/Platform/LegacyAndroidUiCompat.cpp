#include "stdafx.h"

#if defined(__ANDROID__)

#include "DSPlaySound.h"
#include "GlobalBitmap.h"
#include "Platform/LegacyAndroidUiCompat.h"
#include "UIControls.h"
#include "ZzzOpenglUtil.h"

#include <algorithm>
#include <string>
#include <unordered_map>

CMultiLanguage* CMultiLanguage::ms_Singleton = NULL;

namespace
{
	size_t GetMeasuredLength(LPCSTR text, int count)
	{
		if (text == NULL)
		{
			return 0;
		}
		return (count >= 0) ? static_cast<size_t>(count) : strlen(text);
	}

	class AndroidStubUiServices
	{
	public:
		AndroidStubUiServices()
			: language("")
		{
		}

		CMultiLanguage language;
	};

	AndroidStubUiServices g_android_stub_ui_services;

	struct AndroidInputBoxState
	{
		std::string text;
		int text_limit;
		bool visible;
		bool focused;
	};

	std::unordered_map<const CUITextInputBox*, AndroidInputBoxState> g_android_input_box_states;
	CUITextInputBox* g_android_focused_input_box = NULL;

	AndroidInputBoxState& GetAndroidInputBoxState(const CUITextInputBox* input_box)
	{
		return g_android_input_box_states[input_box];
	}
}

extern DWORD g_dwKeyFocusUIID;

namespace platform
{
	void ClearLegacyAndroidTextInputFocus()
	{
		if (g_android_focused_input_box != NULL)
		{
			GetAndroidInputBoxState(g_android_focused_input_box).focused = false;
			g_android_focused_input_box = NULL;
		}
		g_dwKeyFocusUIID = 0;
	}

	bool IsLegacyAndroidTextInputFocused(const CUITextInputBox* input_box)
	{
		return input_box != NULL &&
			g_android_focused_input_box == input_box &&
			GetAndroidInputBoxState(input_box).focused;
	}

	bool AppendLegacyAndroidTextInputChar(CUITextInputBox* input_box, char value)
	{
		if (input_box == NULL || value < 32 || value > 126)
		{
			return false;
		}

		AndroidInputBoxState& state = GetAndroidInputBoxState(input_box);
		if (state.text_limit > 0 && static_cast<int>(state.text.size()) >= state.text_limit)
		{
			return false;
		}

		state.text.push_back(value);
		return true;
	}

	bool BackspaceLegacyAndroidTextInput(CUITextInputBox* input_box)
	{
		if (input_box == NULL)
		{
			return false;
		}

		AndroidInputBoxState& state = GetAndroidInputBoxState(input_box);
		if (state.text.empty())
		{
			return false;
		}

		state.text.erase(state.text.size() - 1);
		return true;
	}
}

DWORD CreateUIID()
{
	static DWORD s_next_ui_id = 1;
	return s_next_ui_id++;
}

DWORD g_dwActiveUIID = 0;
DWORD g_dwMouseUseUIID = 0;
DWORD g_dwCurrentPressedButtonID = 0;
DWORD g_dwKeyFocusUIID = 0;
bool MouseOnWindow = false;

BOOL CheckMouseIn(int iPos_x, int iPos_y, int iWidth, int iHeight, int CoordType)
{
	if (CoordType == COORDINATE_TYPE_LEFT_DOWN)
	{
		return (MouseX >= iPos_x && MouseX < iPos_x + iWidth &&
			MouseY >= iPos_y - iHeight && MouseY < iPos_y) ? TRUE : FALSE;
	}

	return (MouseX >= iPos_x && MouseX < iPos_x + iWidth &&
		MouseY >= iPos_y && MouseY < iPos_y + iHeight) ? TRUE : FALSE;
}

void CUIMessage::SendUIMessage(int iMessage, int iParam1, int iParam2)
{
	UI_MESSAGE message = {};
	message.m_iMessage = iMessage;
	message.m_iParam1 = iParam1;
	message.m_iParam2 = iParam2;
	m_MessageList.push_back(message);
}

void CUIMessage::GetUIMessage()
{
	if (m_MessageList.empty())
	{
		return;
	}

	m_WorkMessage = m_MessageList.front();
	m_MessageList.pop_front();
}

CUIControl::CUIControl()
{
	m_dwUIID = CreateUIID();
	m_dwParentUIID = 0;
	SetState(0);
	m_iOptions = 0;
	SetPosition(0, 0);
	SetSize(100, 100);
	SetArrangeType();
	SetResizeType();
	m_iCoordType = COORDINATE_TYPE_LEFT_TOP;
}

void CUIControl::SetState(int iState)
{
	m_iState = iState;
}

int CUIControl::GetState()
{
	return m_iState;
}

void CUIControl::SetPosition(int iPos_x, int iPos_y)
{
	m_iPos_x = iPos_x;
	m_iPos_y = iPos_y;
}

void CUIControl::SetSize(int iWidth, int iHeight)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;
}

void CUIControl::SetArrangeType(int iArrangeType, int iRelativePos_x, int iRelativePos_y)
{
	m_iArrangeType = iArrangeType;
	m_iRelativePos_x = iRelativePos_x;
	m_iRelativePos_y = iRelativePos_y;
}

void CUIControl::SetResizeType(int iResizeType, int iRelativeWidth, int iRelativeHeight)
{
	m_iResizeType = iResizeType;
	m_iRelativeWidth = iRelativeWidth;
	m_iRelativeHeight = iRelativeHeight;
}

void CUIControl::SendUIMessageDirect(int iMessage, int iParam1, int iParam2)
{
	SendUIMessage(iMessage, iParam1, iParam2);
	DoAction(TRUE);
}

BOOL CUIControl::DoAction(BOOL bMessageOnly)
{
	while (!m_MessageList.empty())
	{
		GetUIMessage();
		if (HandleMessage() == FALSE)
		{
			DefaultHandleMessage();
		}
	}

	DoActionSub(bMessageOnly);
	return FALSE;
}

void CUIControl::DefaultHandleMessage()
{
}

CUITextInputBox::CUITextInputBox()
{
	m_hParentWnd = NULL;
	m_hEditWnd = reinterpret_cast<HWND>(this);
	m_hOldProc = NULL;
	m_hMemDC = NULL;
	m_hBitmap = NULL;
	m_pFontBuffer = NULL;
	m_pTabTarget = NULL;
	m_dwTextColor = _ARGB(255, 255, 255, 255);
	m_dwBackColor = _ARGB(255, 0, 0, 0);
	m_dwSelectBackColor = _ARGB(255, 150, 150, 150);
	m_fCaretWidth = 0.0f;
	m_fCaretHeight = 0.0f;
	m_bPasswordInput = FALSE;
	m_bLock = FALSE;
	m_bIsReady = FALSE;
	m_iRealWindowPos_x = 0;
	m_iRealWindowPos_y = 0;
	m_bUseMultiLine = FALSE;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;
	m_iNumLines = 0;
	m_fScrollBarWidth = 0.0f;
	m_fScrollBarRange_top = 0.0f;
	m_fScrollBarRange_bottom = 0.0f;
	m_fScrollBarHeight = 0.0f;
	m_fScrollBarPos_y = 0.0f;
	m_fScrollBarClickPos_y = 0.0f;
	m_iCaretBlinkTemp = 0;
	SetPosition(193, 422);
	GetAndroidInputBoxState(this).text_limit = MAX_TEXT_LENGTH;
	GetAndroidInputBoxState(this).visible = false;
	GetAndroidInputBoxState(this).focused = false;
}

CUITextInputBox::~CUITextInputBox()
{
	g_android_input_box_states.erase(this);
	if (g_android_focused_input_box == this)
	{
		g_android_focused_input_box = NULL;
	}
}

void CUITextInputBox::SetSize(int iWidth, int iHeight)
{
	CUIControl::SetSize(iWidth, iHeight);
}

void CUITextInputBox::Init(HWND hWnd, int iWidth, int iHeight, int iMaxLength, BOOL bIsPassword)
{
	m_hParentWnd = hWnd;
	m_hEditWnd = reinterpret_cast<HWND>(this);
	m_bPasswordInput = bIsPassword;
	m_bIsReady = TRUE;
	SetSize(iWidth, iHeight);
	AndroidInputBoxState& state = GetAndroidInputBoxState(this);
	state.text_limit = iMaxLength > 0 ? iMaxLength : MAX_TEXT_LENGTH;
	state.visible = false;
	state.focused = false;
}

void CUITextInputBox::Render()
{
}

void CUITextInputBox::GiveFocus(BOOL SelectText)
{
	(void)SelectText;
	if (g_android_focused_input_box != NULL && g_android_focused_input_box != this)
	{
		GetAndroidInputBoxState(g_android_focused_input_box).focused = false;
	}
	g_android_focused_input_box = this;
	GetAndroidInputBoxState(this).focused = true;
	g_dwKeyFocusUIID = GetUIID();
}

void CUITextInputBox::SetState(int iState)
{
	CUIControl::SetState(iState);
	GetAndroidInputBoxState(this).visible = (iState != UISTATE_HIDE);
}

void CUITextInputBox::SetFont(HFONT hFont)
{
	(void)hFont;
}

void CUITextInputBox::SetTextLimit(int iLimit)
{
	GetAndroidInputBoxState(this).text_limit = iLimit > 0 ? iLimit : MAX_TEXT_LENGTH;
}

void CUITextInputBox::SetText(const char* pszText)
{
	AndroidInputBoxState& state = GetAndroidInputBoxState(this);
	state.text = pszText != NULL ? pszText : "";
	if (state.text_limit > 0 && static_cast<int>(state.text.size()) > state.text_limit)
	{
		state.text.resize(static_cast<size_t>(state.text_limit));
	}
}

void CUITextInputBox::GetText(char* pszText, int iGetLenght)
{
	if (pszText == NULL || iGetLenght <= 0)
	{
		return;
	}

	const std::string& text = GetAndroidInputBoxState(this).text;
	const size_t copy_size = std::min(static_cast<size_t>(iGetLenght - 1), text.size());
	memcpy(pszText, text.c_str(), copy_size);
	pszText[copy_size] = '\0';
}

void CUITextInputBox::GetText(wchar_t* pwszText, int iGetLenght)
{
	if (pwszText == NULL || iGetLenght <= 0)
	{
		return;
	}

	const std::string& text = GetAndroidInputBoxState(this).text;
	const size_t copy_size = std::min(static_cast<size_t>(iGetLenght - 1), text.size());
	for (size_t index = 0; index < copy_size; ++index)
	{
		pwszText[index] = static_cast<wchar_t>(static_cast<unsigned char>(text[index]));
	}
	pwszText[copy_size] = L'\0';
}

void CUITextInputBox::SetIMEPosition()
{
}

BOOL CUITextInputBox::DoMouseAction()
{
	return FALSE;
}

CMultiLanguage::CMultiLanguage(std::string strSelectedML)
	: byLanguage(0)
	, iCodePage(CP_UTF8)
	, iNumByteForOneCharUTF8(1)
{
	(void)strSelectedML;
	ms_Singleton = this;
}

BYTE CMultiLanguage::GetLanguage()
{
	return byLanguage;
}

int CMultiLanguage::GetCodePage()
{
	return iCodePage;
}

int CMultiLanguage::GetNumByteForOneCharUTF8()
{
	return iNumByteForOneCharUTF8;
}

BOOL CMultiLanguage::IsCharUTF8(const char* pszText)
{
	(void)pszText;
	return FALSE;
}

int CMultiLanguage::ConvertCharToWideStr(std::wstring& wstrDest, LPCSTR lpString)
{
	wstrDest.clear();
	if (lpString == NULL)
	{
		return 0;
	}

	while (*lpString != '\0')
	{
		wstrDest.push_back(static_cast<wchar_t>(static_cast<unsigned char>(*lpString)));
		++lpString;
	}
	return static_cast<int>(wstrDest.size());
}

int CMultiLanguage::ConvertWideCharToStr(std::string& strDest, LPCWSTR lpwString, int iConversionType)
{
	(void)iConversionType;
	strDest.clear();
	if (lpwString == NULL)
	{
		return 0;
	}

	while (*lpwString != L'\0')
	{
		strDest.push_back(static_cast<char>(*lpwString & 0xFF));
		++lpwString;
	}
	return static_cast<int>(strDest.size());
}

void CMultiLanguage::ConvertANSIToUTF8OrViceVersa(std::string& strDest, LPCSTR lpString)
{
	strDest = lpString != NULL ? lpString : "";
}

int CMultiLanguage::GetClosestBlankFromCenter(const std::wstring wstrTarget)
{
	if (wstrTarget.empty())
	{
		return -1;
	}

	int center = static_cast<int>(wstrTarget.size() / 2);
	for (int offset = 0; offset <= center; ++offset)
	{
		const int left = center - offset;
		const int right = center + offset;
		if (left >= 0 && wstrTarget[static_cast<size_t>(left)] == L' ')
		{
			return left;
		}
		if (right < static_cast<int>(wstrTarget.size()) && wstrTarget[static_cast<size_t>(right)] == L' ')
		{
			return right;
		}
	}
	return -1;
}

WPARAM CMultiLanguage::ConvertFulltoHalfWidthChar(DWORD wParam)
{
	return static_cast<WPARAM>(wParam);
}

BOOL CMultiLanguage::_GetTextExtentPoint32(HDC hdc, LPCWSTR lpString, int cbString, LPSIZE lpSize)
{
	(void)hdc;
	if (lpSize == NULL)
	{
		return FALSE;
	}

	const size_t length = (lpString != NULL)
		? static_cast<size_t>(cbString >= 0 ? cbString : wcslen(lpString))
		: 0;
	lpSize->cx = static_cast<LONG>(length * 8);
	lpSize->cy = 14;
	return TRUE;
}

BOOL CMultiLanguage::_GetTextExtentPoint32(HDC hdc, LPCSTR lpString, int cbString, LPSIZE lpSize)
{
	(void)hdc;
	if (lpSize == NULL)
	{
		return FALSE;
	}

	const size_t length = GetMeasuredLength(lpString, cbString);
	lpSize->cx = static_cast<LONG>(length * 8);
	lpSize->cy = 14;
	return TRUE;
}

BOOL CMultiLanguage::_TextOut(HDC hdc, int nXStart, int nYStart, LPCWSTR lpString, int cbString)
{
	(void)hdc;
	(void)nXStart;
	(void)nYStart;
	(void)lpString;
	(void)cbString;
	return TRUE;
}

BOOL CMultiLanguage::_TextOut(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cbString)
{
	(void)hdc;
	(void)nXStart;
	(void)nYStart;
	(void)lpString;
	(void)cbString;
	return TRUE;
}

CUIRenderText::CUIRenderText()
	: m_pRenderText(NULL)
	, m_iRenderTextType(0)
{
}

CUIRenderText::~CUIRenderText()
{
}

CUIRenderText* CUIRenderText::GetInstance()
{
	static CUIRenderText render_text;
	return &render_text;
}

bool CUIRenderText::Create(int iRenderTextType, HDC hDC)
{
	(void)hDC;
	m_iRenderTextType = iRenderTextType;
	return true;
}

void CUIRenderText::Release()
{
}

int CUIRenderText::GetRenderTextType() const
{
	return m_iRenderTextType;
}

HDC CUIRenderText::GetFontDC() const
{
	return reinterpret_cast<HDC>(1);
}

BYTE* CUIRenderText::GetFontBuffer() const
{
	return NULL;
}

DWORD CUIRenderText::GetTextColor() const
{
	return 0xFFFFFFFFu;
}

DWORD CUIRenderText::GetBgColor() const
{
	return 0x00000000u;
}

void CUIRenderText::SetTextColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha)
{
	(void)byRed;
	(void)byGreen;
	(void)byBlue;
	(void)byAlpha;
}

void CUIRenderText::SetTextColor(DWORD dwColor)
{
	(void)dwColor;
}

void CUIRenderText::SetBgColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha)
{
	(void)byRed;
	(void)byGreen;
	(void)byBlue;
	(void)byAlpha;
}

void CUIRenderText::SetBgColor(DWORD dwColor)
{
	(void)dwColor;
}

void CUIRenderText::SetFont(HFONT hFont)
{
	(void)hFont;
}

void CUIRenderText::RenderText(int iPos_x, int iPos_y, const char* pszText, int iBoxWidth, int iBoxHeight, int iSort, OUT SIZE* lpTextSize)
{
	(void)iPos_x;
	(void)iPos_y;
	(void)iBoxWidth;
	(void)iBoxHeight;
	(void)iSort;

	if (lpTextSize != NULL)
	{
		lpTextSize->cx = static_cast<LONG>(GetMeasuredLength(pszText, -1) * 8);
		lpTextSize->cy = 14;
	}
}

HRESULT PlayBuffer(int Buffer, OBJECT* Object, BOOL bLooped)
{
	(void)Buffer;
	(void)Object;
	(void)bLooped;
	return 0;
}

#if !defined(MU_ANDROID_HAS_ZZZOPENGLUTIL_RUNTIME)
void BindTexture(int tex)
{
	BITMAP_t* bitmap = Bitmaps.FindTexture(static_cast<GLuint>(tex));
	if (bitmap != NULL && bitmap->TextureNumber != 0)
	{
		glBindTexture(GL_TEXTURE_2D, bitmap->TextureNumber);
	}
}
#endif

CBitmapCache::CBitmapCache()
{
}

CBitmapCache::~CBitmapCache()
{
}

bool CBitmapCache::Create()
{
	return true;
}

void CBitmapCache::Release()
{
}

void CBitmapCache::Add(GLuint uiBitmapIndex, BITMAP_t* pBitmap)
{
	(void)uiBitmapIndex;
	(void)pBitmap;
}

void CBitmapCache::Remove(GLuint uiBitmapIndex)
{
	(void)uiBitmapIndex;
}

void CBitmapCache::RemoveAll()
{
}

size_t CBitmapCache::GetCacheSize()
{
	return 0;
}

void CBitmapCache::Update()
{
}

bool CBitmapCache::Find(GLuint uiBitmapIndex, BITMAP_t** ppBitmap)
{
	(void)uiBitmapIndex;
	if (ppBitmap != NULL)
	{
		*ppBitmap = NULL;
	}
	return false;
}

#endif

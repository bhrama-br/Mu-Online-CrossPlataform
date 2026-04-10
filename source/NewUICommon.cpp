// NewUICommon.cpp: implementation of the CNewUICommon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Console.h"
#include "NewUICommon.h"
#include "NewUIRenderNumber.h"
#include "NewUISystem.h"
#include "NewUICommonMessageBox.h"
#include "ZzzTexture.h"
#include "ZzzOpenglUtil.h"

extern int MouseX, MouseY;
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
extern bool g_bWndActive;
#endif	// ASG_FIX_ACTIVATE_APP_INPUT

bool SEASON3B::CreateOkMessageBox(const unicode::t_string& strMsg, DWORD dwColor, float fPriority)
{
	CNewUICommonMessageBox* pMsgBox = g_MessageBox->NewMessageBox(MSGBOX_CLASS(CNewUICommonMessageBox));
	if(pMsgBox)
	{
		return pMsgBox->Create(MSGBOX_COMMON_TYPE_OK, strMsg, dwColor);
	}
	return false;
}

int SEASON3B::IsPurchaseShop() 
{ 
	if( g_pMyShopInventory->IsVisible() ) 
	{
		return 1;
	}
	else if( g_pPurchaseShopInventory->IsVisible() ) 
	{
		return 2;
	}

	return -1;
}

bool SEASON3B::CheckMouseIn(int x, int y, int width, int height)
{
	if(MouseX >= x && MouseX < x + width && MouseY >= y && MouseY < y + height)
		return true;
	return false;
}

#ifdef NEW_MUHELPER_ON
void SEASON3B::RenderImage(GLuint uiImageType, float x, float y)
{
	BITMAP_t* pImage = &Bitmaps[uiImageType];
	float u = 0.5 / pImage->Width;
	float v = 0.5 / pImage->Height;

	float temp_Width = ((float)pImage->output_width - 0.5) / pImage->Width;
	float temp_Height = ((float)pImage->output_height - 0.5) / pImage->Height;

	float vHeight = temp_Height - v;
	float uWidth = temp_Width - u;

	float Height = (double)pImage->output_height;
	float Width = (double)pImage->output_width;

	RenderBitmap(uiImageType, x, y, Width, Height, u, v, uWidth, vHeight);
}
#endif

void SEASON3B::RenderImage(GLuint uiImageType, float x, float y, float width, float height)
{
	if (uiImageType == 31319)	//IMAGE_IGS_BACK
	{
		if (width == 222.f)
		{
			width = 230.f;
		}
		if (height == 429.f)
		{
			GWidescreen.RenderImages(931300, x, y, width, 254.f, 0.0, 0.0, 420.f, 566.f);
			GWidescreen.RenderImages(931301, x, y + 253.5f, width, 175.f, 0.0, 0.0, 420.f, 391.f);
		}
		else if (height > 254.f)
		{
			float vh = ((height - 254.f) / 175.f) * 391.f;

			GWidescreen.RenderImages(931300, x, y, width, 254.f, 0.0, 0.0, 420.f, 566.f);
			GWidescreen.RenderImages(931301, x, y + 253.5f, width, (height - 254.f), 0.0, 0.0, 420.f, vh);
		}
		else
		{
			float vh = (height / 254.f) * 566.f;
			GWidescreen.RenderImages(931300, x, y, width, height, 0.0, 0.0, 420.f, vh);
		}
	}
	else if
		(uiImageType == 31316 //-- newui_msgbox_top					IMAGE_MSGBOX_TOP
			|| uiImageType == 31317 //-- newui_msgbox_middle			IMAGE_MSGBOX_MIDDLE
			|| uiImageType == 31318 //-- newui_msgbox_bottom			IMAGE_MSGBOX_BOTTOM
			|| uiImageType == 31328 //-- newui_Message_03				IMAGE_MSGBOX_TOP_TITLEBAR
			|| uiImageType == 31347 //-- newui_item_back01			IMAGE_INVENTORY_BACK_TOP
			|| uiImageType == 31348 //-- newui_item_back04			IMAGE_INVENTORY_BACK_TOP2
			|| uiImageType == 31349 //-- newui_item_back02-L			IMAGE_INVENTORY_BACK_LEFT
			|| uiImageType == 31350 //-- newui_item_back02-R			IMAGE_INVENTORY_BACK_RIGHT
			|| uiImageType == 31351 //-- newui_item_back03			IMAGE_INVENTORY_BACK_BOTTOM
			|| uiImageType == 31317 //-- newui_msgbox_middle			IMAGE_MSGBOX_MIDDLE
			|| uiImageType == 31316 //-- newui_msgbox_top				IMAGE_MSGBOX_TOP
			|| uiImageType == 31582 //-- newui_option_top				IMAGE_OPTION_FRAME_UP
			|| uiImageType == 31583 //-- newui_option_back06(L)		IMAGE_OPTION_FRAME_LEFT
			|| uiImageType == 31584 //-- newui_option_back06(R)		IMAGE_OPTION_FRAME_RIGHT
			)
	{
		if (uiImageType == 31351 && y < 384.f)		// IMAGE_INVENTORY_BACK_BOTTOM
		{
			float vh = (height / 175.f) * 391.f;

			GWidescreen.RenderImages(931302, x, y, width, height, 0.0, 391.f - vh, 420.f, vh);
		}
		if (uiImageType == 31318)	//IMAGE_MSGBOX_BOTTOM
		{
			float vh = (height / 175.f) * 391.f;

			GWidescreen.RenderImages(931302, x, y, width, height, 0.0, 391.f - vh, 420.f, vh);
		}
	}
	else if (uiImageType == 31336)	// IMAGE_ITEM_SQUARE
	{
		GWidescreen.RenderImages(931315, x, y, width, height, 0.0, 0.0, 44.f, 44.f);
	}
	else if (uiImageType == 31363 || uiImageType == 31368 || uiImageType == 31387) //-- bar zen		//	IMAGE_INVENTORY_MONEY , IMAGE_MYSHOPINVENTORY_EDIT , IMAGE_NPCQUEST_ZEN
	{
		GWidescreen.RenderImages(931316, x, y, 114.f, 21.f, 0.0, 0.0, 256.f, 46.5f);
	}
	else if (uiImageType == 31588 || uiImageType == 31589) //-- Effect		//	IMAGE_OPTION_EFFECT_BACK , IMAGE_OPTION_EFFECT_COLOR
	{
		float uw = (width / 141.f) * 256.f;

		GWidescreen.RenderImages(uiImageType, x, y, width, height, 0.0, 0.0, uw, 52.f);
	}
	else
	{
		BITMAP_t* pImage = &Bitmaps[uiImageType];

		float u, v, uw, vh;

		u = 0.5f / (float)pImage->Width;
		v = 0.5f / (float)pImage->Height;
		uw = (width - 0.5f) / (float)pImage->Width;
		vh = (height - 0.5f) / (float)pImage->Height;

		RenderBitmap(uiImageType, x, y, width, height, u, v, uw - u, vh - v);
	}
}

void SEASON3B::RenderImageA(GLuint uiImageType, float x, float y, float width, float height)
{
	if (uiImageType == SEASON3B::CNewUIMyInventory::IMAGE_INVENTORY_ITEM_BOOT
		|| uiImageType == SEASON3B::CNewUIMyInventory::IMAGE_INVENTORY_ITEM_HELM
		|| uiImageType == SEASON3B::CNewUIMyInventory::IMAGE_INVENTORY_ITEM_FAIRY
		|| uiImageType == SEASON3B::CNewUIMyInventory::IMAGE_INVENTORY_ITEM_GLOVES
		|| uiImageType == SEASON3B::CNewUIMyInventory::IMAGE_INVENTORY_ITEM_PANTS)
	{
		GWidescreen.RenderImages3F(uiImageType, x, y, width - 2, height - 3, 88, 89); //-- cuadrados
	}
	else if (uiImageType == SEASON3B::CNewUIMyInventory::IMAGE_INVENTORY_ITEM_RIGHT
		|| uiImageType == SEASON3B::CNewUIMyInventory::IMAGE_INVENTORY_ITEM_LEFT
		|| uiImageType == SEASON3B::CNewUIMyInventory::IMAGE_INVENTORY_ITEM_ARMOR)
	{
		GWidescreen.RenderImages3F(uiImageType, x, y, width - 2, height - 4, 88, 133); //-- centrales
	}
	else if (uiImageType == SEASON3B::CNewUIMyInventory::IMAGE_INVENTORY_ITEM_RING || uiImageType == SEASON3B::CNewUIMyInventory::IMAGE_INVENTORY_ITEM_NECKLACE) //-- ring pendals
	{
		GWidescreen.RenderImages3F(uiImageType, x, y, width - 4, height - 4, 46, 46);
	}
	else if (uiImageType == SEASON3B::CNewUIMyInventory::IMAGE_INVENTORY_ITEM_WING)
	{
		GWidescreen.RenderImages3F(uiImageType, x, y, width - 2, height - 3, 133, 88); //--  wings
	}
	else if (uiImageType == 65535 || uiImageType == 65536 || uiImageType == 65537)
	{
		GWidescreen.RenderImages3F(uiImageType, x, y, width - 4, height - 4, 46, 46);
	}
	else
	{
		BITMAP_t* pImage = &Bitmaps[uiImageType];

		float u, v, uw, vh;

		u = 0.5f / (float)pImage->Width;
		v = 0.5f / (float)pImage->Height;
		uw = (width - 0.5f) / (float)pImage->Width;
		vh = (height - 0.5f) / (float)pImage->Height;

		RenderBitmap(uiImageType, x, y, width, height, u, v, uw - u, vh - v);
	}
}

void SEASON3B::RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv,float uw, float vh, DWORD color)
{
	RenderColorBitmap(uiImageType, x, y, width, height, su, sv, uw, vh, color);
}

void SEASON3B::RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv)
{
	BITMAP_t* pImage = &Bitmaps[uiImageType];

	float u, v, uw, vh;
	u = ((su + 0.5f) / (float)pImage->Width);
	v = ((sv + 0.5f) / (float)pImage->Height);
	uw = (width - 0.5f) / (float)pImage->Width - (0.5f / (float)pImage->Width);
	vh = (height - 0.5f) / (float)pImage->Height - (0.5f / (float)pImage->Height);

	RenderBitmap(uiImageType, x, y, width, height, u, v, uw, vh);
}

void SEASON3B::RenderImageScale(GLuint uiImageType, float x, float y, float width, float height, float su, float sv, float uw, float vh, float alpha)
{
	BITMAP_t* pImage = &Bitmaps[uiImageType];
	RenderBitmap(uiImageType, x, y, width, height, su / (float)pImage->Width, sv / (float)pImage->Height, uw / (float)pImage->Width, vh / (float)pImage->Height, 1, 1, alpha);
}

void SEASON3B::RenderImageScaleAuto(GLuint uiImageType, float x, float y, float width, float height)
{
	BITMAP_t* pImage = &Bitmaps[uiImageType];

	float u, v, uw, vh;
	u = 0.5f / (float)pImage->Width;
	v = 0.5f / (float)pImage->Height;
	uw = ((float)width - 0.5f) / (float)pImage->Width;
	vh = ((float)height - 0.5f) / (float)pImage->Height;

	RenderBitmap(uiImageType, x, y, width, height, u, v, uw, vh, 1, 1, 0.0f);
}

int ConvertTexture(int image)
{
	switch (image)
	{
	case 31364: //-- exit
		image = 931306;
		break;
	case 31558: //-- exit
		image = 931350;
		break;
	case 31365: //-- repair
		image = 931307;
		break;
	case 31369:
		image = 931308;
		break;
	case 31370:
		image = 931314;
		break;
	case 31413: //-- mix
		image = 931309;
		break;
	case 31407: //-- setzen
		image = 931310;
		break;
	case 31408: //-- getzen
		image = 931311;
		break;
	case 31409: //-- lock
		image = 931312;
		break;
	case 31410: //-- lock
		image = 931313;
		break;
	}
	return image;
}

void SEASON3B::RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv, DWORD color)
{
	if (uiImageType != ConvertTexture(uiImageType))
	{
		if (sv == 24.0)
		{
			sv = 108.f;
		}
		if (sv == 35.0)
		{
			sv = 108.f;
		}
		GWidescreen.RenderImages(ConvertTexture(uiImageType), x, y, width, height, 0.0, sv, 53.5f, 53.5f);
	}
	else if ((uiImageType >= 31302 && uiImageType <= 31306) || uiImageType ==  931317) //-- button main interface
	{
		if (sv == 22.f || sv == 24.0) //-- fix button cashshop
		{
			sv = 54.f;
		}
		if (sv == 66.f || sv == 44.0) //-- fix button cashshop
		{
			sv = 110.f;
		}

		GWidescreen.RenderImages(uiImageType, x, y, width, height, su, sv, 52.f, 52.f);
	}
	else if (uiImageType == 31323)
	{
		if (sv == 29.0)
		{
			sv = 54.f;
		}
		if (sv == 58.0)
		{
			sv = 108.f;
		}
		GWidescreen.RenderImages(931324, x, y, width, height, 0.0, sv, 294.5f, 53.5f);
	}
	else if (uiImageType == 931326)
	{
		if (sv == 29.0)
		{
			sv = 54.f;
		}
		if (sv == 58.0)
		{
			sv = 108.f;
		}

		GWidescreen.RenderImages(931326, x, y, 54, 23, 0.0, sv, 122.f, 53.5f);
	}
	else if (uiImageType == 941000 || uiImageType == 941001 || uiImageType == 941002 || uiImageType == 941003 || uiImageType == 941004 || uiImageType == 941005 || uiImageType == 941006)
	{
		if (sv == 29.0)
		{
			sv = 54.f;
		}
		if (sv == 58.0)
		{
			sv = 108.f;
		}

		GWidescreen.RenderImages(uiImageType, x, y, 54, 23, 0.0, sv, 122.f, 53.5f);
	}
	else if (uiImageType == 931327)
	{
		if (sv == 29.0)
		{
			sv = 54.f;
		}
		if (sv == 58.0)
		{
			sv = 108.f;
		}
			
		GWidescreen.RenderImages(931327, x, y, 54, 23, 0.0, sv, 122.f, 53.5f);
	}
	else if (uiImageType == 31322) //-- boton cerrar
	{
		if (sv == 23.0)
		{
			sv = 54.f;
		}
		if (sv == 60.0)
		{
			sv = 108.f;
		}
		GWidescreen.RenderImages(931325, x, y, width, height, 0.0, sv, 122.f, 53.5f);
	}
	else
	{
		BITMAP_t* pImage = &Bitmaps[uiImageType];

		float u, v, uw, vh;
		u = ((su + 0.5f) / (float)pImage->Width);
		v = ((sv + 0.5f) / (float)pImage->Height);
		uw = (width - 0.5f) / (float)pImage->Width - (0.5f / (float)pImage->Width);
		vh = (height - 0.5f) / (float)pImage->Height - (0.5f / (float)pImage->Height);

		RenderColorBitmap(uiImageType, x, y, width, height, u, v, uw, vh, color);
	}
}

float SEASON3B::RenderNumber(float x, float y, int iNum, float fScale)
{
	return g_RenderNumber->RenderNumber(x, y, iNum, fScale);
}

bool SEASON3B::IsNone(int iVirtKey)
{
	if (GetForegroundWindow() != g_hWnd)
	{
		return 0;
	}
	return g_pNewKeyInput->IsNone(iVirtKey); 
}

bool SEASON3B::IsRelease(int iVirtKey)
{
	if (GetForegroundWindow() != g_hWnd)
	{
		return 0;
	}
	return g_pNewKeyInput->IsRelease(iVirtKey); 
}

bool SEASON3B::IsPress(int iVirtKey)
{
	if (GetForegroundWindow() != g_hWnd)
	{
		return 0;
	}
	return g_pNewKeyInput->IsPress(iVirtKey); 
}

bool SEASON3B::IsRepeat(int iVirtKey)
{
	if (GetForegroundWindow() != g_hWnd)
	{
		return 0;
	}
	return g_pNewKeyInput->IsRepeat(iVirtKey); 
}

SEASON3B::CNewKeyInput::CNewKeyInput()
{
	Init();
}

SEASON3B::CNewKeyInput::~CNewKeyInput()
{

}

void SEASON3B::CNewKeyInput::Init()
{
	memset(&m_pInputInfo, 0, sizeof(INPUTSTATEINFO)*256);
}

SEASON3B::CNewKeyInput* SEASON3B::CNewKeyInput::GetInstance()
{
	static SEASON3B::CNewKeyInput s_Instance;
	return &s_Instance;
}

void SEASON3B::CNewKeyInput::ScanAsyncKeyState()
{
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
	if (!g_bWndActive)
		return;
#endif	// ASG_FIX_ACTIVATE_APP_INPUT

	for(int key=0; key < 256; key++)
	{
		if(HIBYTE(GetAsyncKeyState(key)) & 0x80)
		{
			if(m_pInputInfo[key].byKeyState == KEY_NONE || m_pInputInfo[key].byKeyState == KEY_RELEASE)
			{
				// press event (key was up before but down now)
				m_pInputInfo[key].byKeyState = KEY_PRESS;
			}
			else if(m_pInputInfo[key].byKeyState == KEY_PRESS)
			{
				// drag event (key is still down)
				m_pInputInfo[key].byKeyState = KEY_REPEAT;
			}
		}
		else // Key is not currently pressed
		{
			if(m_pInputInfo[key].byKeyState == KEY_REPEAT || m_pInputInfo[key].byKeyState == KEY_PRESS)
			{
				// release event (key was down before but up now)
				m_pInputInfo[key].byKeyState = KEY_RELEASE;
			}
			else if(m_pInputInfo[key].byKeyState == KEY_RELEASE)
			{
				m_pInputInfo[key].byKeyState = KEY_NONE;
			}
		}
	}

	if( IsPress(VK_RETURN) && IsEnterPressed() == false ) {
		m_pInputInfo[VK_RETURN].byKeyState = KEY_NONE;
	}
	SetEnterPressed( false );
}

bool SEASON3B::CNewKeyInput::IsNone(int iVirtKey)
{
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
	if (!g_bWndActive)
		return false;
#endif	// ASG_FIX_ACTIVATE_APP_INPUT
	return (m_pInputInfo[iVirtKey].byKeyState == KEY_NONE) ? true : false;
}

bool SEASON3B::CNewKeyInput::IsRelease(int iVirtKey)
{
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
	if (!g_bWndActive)
		return false;
#endif	// ASG_FIX_ACTIVATE_APP_INPUT
	return (m_pInputInfo[iVirtKey].byKeyState == KEY_RELEASE) ? true : false;
}

bool SEASON3B::CNewKeyInput::IsPress(int iVirtKey)
{
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
	if (!g_bWndActive)
		return false;
#endif	// ASG_FIX_ACTIVATE_APP_INPUT
	return (m_pInputInfo[iVirtKey].byKeyState == KEY_PRESS) ? true : false;	
}

bool SEASON3B::CNewKeyInput::IsRepeat(int iVirtKey)
{
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
	if (!g_bWndActive)
		return false;
#endif	// ASG_FIX_ACTIVATE_APP_INPUT
	return (m_pInputInfo[iVirtKey].byKeyState == KEY_REPEAT) ? true : false;
}

void SEASON3B::CNewKeyInput::SetKeyState(int iVirtKey, KEY_STATE KeyState)
{
	m_pInputInfo[iVirtKey].byKeyState = KeyState;
}


#ifdef NEW_MUHELPER_ON
void SEASON3B::CCheckBox::Initialize()
{
	m_hTextFont = g_hFont;
	m_ImgIndex = 0;
	output_width = 0;
	output_height = 0;
	m_bEnable = 0;
}

void SEASON3B::CCheckBox::Destroy()
{
}

void SEASON3B::CCheckBox::RegisterImgIndex(int imgindex)
{
	m_ImgIndex = imgindex;

	if (m_ImgIndex != -1)
	{
		BITMAP_t* pImage = &Bitmaps[m_ImgIndex];

		output_width = pImage->Width;
		output_height = pImage->Height;
	}
}

void SEASON3B::CCheckBox::ChangeCheckBoxInfo(int x, int y, int sx, int sy)
{
	SetPos(x, y);
	SetSize(sx, sy);
}

void SEASON3B::CCheckBox::ChangeCheckBoxText(unicode::t_string btname)
{
	m_Name = btname;
}

bool SEASON3B::CCheckBox::Create(int imgindex, int x, int y, int sx, int sy, unicode::t_string btname)
{
	RegisterImgIndex(imgindex);
	ChangeCheckBoxInfo(x, y, sx, sy);
	ChangeCheckBoxText(btname);
	return true;
}

bool SEASON3B::CCheckBox::Render()
{
	float sv = (double)(m_Size.cy * (m_bEnable == false));
	RenderImage(m_ImgIndex, (float)m_Pos.x, (float)m_Pos.y, (float)m_Size.cx, (float)m_Size.cy, 0.0, sv);
	g_pRenderText->RenderText(m_Pos.x + m_Size.cx, m_Pos.y + 4, m_Name.c_str(), 0, 0, RT3_SORT_LEFT, 0);

	return true;
}

bool SEASON3B::CCheckBox::UpdateMouseEvent()
{
	bool success = false;

	if (IsRelease(VK_LBUTTON) && CheckMouseIn(m_Pos.x, m_Pos.y, m_Size.cx, m_Size.cy))
	{
		success = true;
		m_bEnable = m_bEnable == false;
	}

	return success;
}

void SEASON3B::CCheckBox::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

void SEASON3B::CCheckBox::SetSize(int width, int height)
{
	m_Size.cx = width;
	m_Size.cy = height;
}

SEASON3B::stMacroUIImage::stMacroUIImage()
{
	m_ImgIndex = -1;
	m_Pos.x = m_Pos.y = 0;
	output_width = 0.0;
	output_height = 0.0;
}

SEASON3B::stMacroUIImage::~stMacroUIImage()
{
}

void SEASON3B::stMacroUIImage::Register(int imgindex, int x, int y, int width, int height)
{
	m_ImgIndex = imgindex;
	m_Pos.x = x;
	m_Pos.y = y;
	output_width = width;
	output_height = height;
}

SEASON3B::stMacroUIText::stMacroUIText()
{
	m_Pos.x = m_Pos.y = 0;
	m_Name = "";
}

SEASON3B::stMacroUIText::~stMacroUIText()
{
}

void SEASON3B::stMacroUIText::Register(int x, int y, const char* pzText)
{
	m_Pos.x = x;
	m_Pos.y = y;
	m_Name = pzText;
}

void SEASON3B::stMacroUIText::Render()
{
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y, m_Name.c_str(), 0, 0, RT3_SORT_LEFT, 0);
}

SEASON3B::CSlideBar::CSlideBar()
{
	m_Imgindex = 0;
	m_ImgBack = 0;
	m_Pos.x = 0;
	m_Pos.y = 0;
	m_Width = 0;
	m_Height = 0;
	m_MaxLength = 0;
	m_iValue = 0;
	m_MinLength = 0;
}

SEASON3B::CSlideBar::~CSlideBar()
{
}

void SEASON3B::CSlideBar::Create(int imgback, int imgindex, int x, int y, __int16 sx, __int16 sy, __int16 iMaxLength, __int16 start)
{
	m_Imgindex = imgindex;
	m_ImgBack = imgback;
	m_Pos.x = x;
	m_Pos.y = y;
	m_Width = sx;
	m_Height = sy;
	m_MaxLength = iMaxLength;
	m_MinLength = start;
}

bool SEASON3B::CSlideBar::MouseUpdate()
{
	int min = m_Width / m_MaxLength;
	if (CheckMouseIn(m_Pos.x - min, m_Pos.y, min + m_Width, m_Height) && IsRepeat(VK_LBUTTON))
	{
		int current = MouseX - m_Pos.x;
		if (current >= m_MinLength)
		{
			m_iValue = (current * m_MaxLength / m_Width) + 1;
		}
		else
		{
			m_iValue = m_MinLength;
		}
	}
	return true;
}

void SEASON3B::CSlideBar::Render()
{
	RenderImage(m_ImgBack, m_Pos.x, m_Pos.y, m_Width, m_Height);
	if (m_iValue > 0)
	{
		float width = (double)m_Width * 0.1 * m_iValue;

		RenderImage(m_Imgindex, m_Pos.x, m_Pos.y, width, m_Height);
	}
}

int SEASON3B::CSlideBar::GetSlideLevel()
{
	return m_iValue;
}

void SEASON3B::CSlideBar::SetSlideLevel(__int16 Value)
{
	m_iValue = Value;
}

SEASON3B::COptionButtonGroup::COptionButtonGroup()
{
	m_x = 0;
	m_y = 0;
	m_w = 0;
	m_h = 0;
	m_index = 0;
	m_imgindex = 0;
}

SEASON3B::COptionButtonGroup::~COptionButtonGroup()
{
	std::vector<CCheckBox*>::iterator iter = this->m_box.begin();
	while (iter != this->m_box.end())
	{
		delete* iter;
		iter++;
	}
	this->m_box.shrink_to_fit();
	this->m_box.clear();
}

void SEASON3B::COptionButtonGroup::Create(int imgindex, int x, int y, int w, int h, unsigned char count)
{
	this->m_x = x;
	this->m_y = y;
	this->m_w = w;
	this->m_h = h;
	for (int i = 0; i < count; i++)
	{
		CCheckBox* box = new CCheckBox;
		box->RegisterImgIndex(imgindex);
		box->ChangeCheckBoxInfo(0, 0, w, h);
		this->m_box.push_back(box);
	}
}

void SEASON3B::COptionButtonGroup::SetOptionText(unsigned char index, int offset_x, int offset_y, const char* text)
{
	unicode::t_string strText = text;
	this->m_box[index]->SetPos(this->m_x + offset_x, this->m_y + offset_y);
	this->m_box[index]->ChangeCheckBoxText(strText);
}

bool SEASON3B::COptionButtonGroup::UpdateMouseEvent()
{
	for (size_t i = 0; i < this->m_box.size(); i++)
	{
		if (true == this->m_box[i]->UpdateMouseEvent())
		{
			if (true == this->m_box[i]->IsSelected())
			{
				SetIndex(i);
				return true;
			}
		}
	}
	return false;
}

bool SEASON3B::COptionButtonGroup::Render()
{
	for (size_t i = 0; i < this->m_box.size(); i++)
	{
		this->m_box[i]->Render();
	}
	return true;
}

void SEASON3B::COptionButtonGroup::SetIndex(int index)
{
	if ((unsigned int)index < this->m_box.size())
		m_index = index;
	else
		m_index = 0;

	for (size_t i = 0; i < this->m_box.size(); i++)
	{
		if (i == this->m_index)
			this->m_box[i]->SetSelected(true);
		else
			this->m_box[i]->SetSelected(false);
	}
}

int SEASON3B::COptionButtonGroup::GetIndex()
{
	return m_index;
}
#endif


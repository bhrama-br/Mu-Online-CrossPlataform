// NewUIMixInventory.cpp: implementation of the CStackInterface class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StackInterface.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "MixMgr.h"
#include "ZzzBMD.h"
#include "ZzzEffect.h"
#include "ZzzObject.h"
#include "ZzzInventory.h"
#include "ZzzInterface.h"
#include "ZzzInfomation.h"
#include "ZzzCharacter.h"
#include "wsclientinline.h"
#include "DSPlaySound.h"
#include "Platform/RenderColorCompat.h"
#include "SocketSystem.h"
#include "LuaStack.hpp"
#include "NProtocol.h"
#include "Protocol.h"
#include "CustomJewelStack.h"

using namespace SEASON3B;

CStackInterface::CStackInterface()
{
	m_pNewUIMng = NULL;

	m_Pos.x = m_Pos.y = 0;
}

CStackInterface::~CStackInterface() { Release(); }

bool CStackInterface::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if (NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;

	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_UNMIXSTACK, this);

	SetPos(x, y);

	LoadImages();

	Show(false);

	return true;
}

void CStackInterface::Release()
{
	UnloadImages();

	if (m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

void CStackInterface::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool CStackInterface::UpdateMouseEvent()
{
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_UNMIXSTACK))
	{
		if (IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_UNMIXSTACK);
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}

	if (CheckMouseIn(m_Pos.x, m_Pos.y, 230.0, 180))
	{
		if (gJewelStack.m_VisibleSplitStack)
		{
			if (CheckMouseIn(m_Pos.x + (((230 / 2) - 54) / 2), m_Pos.y + 180 - 50, 54, 30))
			{
				if (SEASON3B::IsPress(VK_LBUTTON))
				{
					gJewelStack.ProcessUnSplit(gJewelStack.m_ItemSlot, gJewelStack.m_ValueGetStack);

					gJewelStack.m_VisibleSplitStack = 0;
					gJewelStack.m_EventKey = 2;
					//
					g_pNewUISystem->Hide(SEASON3B::INTERFACE_UNMIXSTACK);
					PlayBuffer(SOUND_CLICK01);
					//
					ResetMouseLButton();
				}
				else
				{
					gJewelStack.m_EventKey = 1;
				}
			}
			else
			{
				gJewelStack.m_EventKey = 0;
			}

			if (CheckMouseIn(m_Pos.x + (230 / 2) + (((230 / 2) - 54) / 2), m_Pos.y + 180 - 50, 54, 30))
			{
				if (SEASON3B::IsPress(VK_LBUTTON))
				{
					gJewelStack.m_VisibleSplitStack = 0;
					gJewelStack.m_EventKey2 = 2;
					//
					g_pNewUISystem->Hide(SEASON3B::INTERFACE_UNMIXSTACK);
					PlayBuffer(SOUND_CLICK01);
					//
					ResetMouseLButton();
				}
				else
				{
					gJewelStack.m_EventKey2 = 1;
				}
			}
			else
			{
				gJewelStack.m_EventKey2 = 0;
			}

			if (CheckMouseIn(m_Pos.x + (230 / 2) - (50 / 2) - 18, m_Pos.y + (180 / 2) - 18, 12, 12))
			{
				if (SEASON3B::IsPress(VK_LBUTTON))
				{
					if (gJewelStack.m_ValueGetStack > 1)
					{
						gJewelStack.m_ValueGetStack--;
					}
				}
			}

			if (CheckMouseIn(m_Pos.x + (230 / 2) + 30, m_Pos.y + (180 / 2) - 18, 12, 12))
			{
				if (SEASON3B::IsPress(VK_LBUTTON))
				{
					if (gJewelStack.m_ValueGetStack < gJewelStack.m_Durability)
					{
						gJewelStack.m_ValueGetStack++;
					}
				}
			}
		}


		return false;
	}

	return true;
}

bool CStackInterface::UpdateKeyEvent()
{
	return true;
}

bool CStackInterface::Update()
{
	return true;
}

bool CStackInterface::Render()
{
	EnableAlphaTest();

	glColor4f(1.f, 1.f, 1.f, 1.f);

	RenderFrame();

	RenderText();

	DisableAlphaBlend();

	return true;
}

void CStackInterface::RenderFrame()
{
	EnableAlphaTest();

	float x = m_Pos.x;
	float y = m_Pos.y;

	SEASON3B::RenderImage(31319, x, y + 2.0, 222.0, 180);

	SEASON3B::RenderImage(31328, x, y, 230.0, 45.f);

	y += 45.f;

	for (int i = 0; i < 6; ++i)
	{
		SEASON3B::RenderImage(31317, x, y, 230.0, 15.f);
		y += 15.f;
	}

	SEASON3B::RenderImage(31317, x, y, 230.0, 5.f);

	y += 5.f;

	SEASON3B::RenderImage(31318, x, y, 230.0, 45.f);

	y = 98;

	auto StructImage = &Bitmaps[31320];
	auto StructImage2 = &Bitmaps[31321];

	int m_fButtonWidth = 54;
	int m_fButtonHeight = 30;

	//if (StructImage != 0)
	//{
		GWidescreen.RenderImages(931327, x + (((230 / 2) - 54) / 2), y + 180 - 50, 54, 23, 0.0, gJewelStack.m_EventKey * 54.f, 122.f, 53.5f);
	//}

	//if (StructImage2 != 0) 
	//{
		GWidescreen.RenderImages(931325, x + (230 / 2) + (((230 / 2) - 54) / 2), y + 180 - 50, 54, 23, 0.0, gJewelStack.m_EventKey2 * 54.f, 122.f, 53.5f);
	//}

	if (CheckMouseIn(m_Pos.x + (230 / 2) - (50 / 2) - 18, m_Pos.y + (180 / 2) - 18, 12, 12)) 
	{
		glColor3f(0.6f, 0.6f, 0.6f);
	}
	//-
	if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x + (230 / 2) - (50 / 2) - 18, m_Pos.y + (180 / 2) - 18, 12, 12)) 
	{
		RenderBitmap(71002, x + (230 / 2) - (50 / 2) - 18, y + (180 / 2) - 18, 12, 12, 0, 0, 1.0, 0.919724525, 1, 1, 1);
	}
	else 
	{
		RenderBitmap(71001, x + (230 / 2) - (50 / 2) - 18, y + (180 / 2) - 18, 12, 12, 0, 0, 1.0, 0.919724525, 1, 1, 1);
	}

	glColor3f(1.0f, 1.0f, 1.0f);

	if (CheckMouseIn(m_Pos.x + (230 / 2) + 30, m_Pos.y + (180 / 2) - 18, 12, 12)) 
	{
		glColor3f(0.6f, 0.6f, 0.6f);
	}

	//+
	if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x + (230 / 2) + 30, m_Pos.y + (180 / 2) - 18, 12, 12)) 
	{
		RenderBitmap(71004, x + (230 / 2) + 30, y + (180 / 2) - 18, 12, 12, 0, 0, 1.0, 0.919724525, 1, 1, 1);
	}
	else {
		RenderBitmap(71003, x + (230 / 2) + 30, y + (180 / 2) - 18, 12, 12, 0, 0, 1.0, 0.919724525, 1, 1, 1);
	}

	glColor3f(1.0f, 1.0f, 1.0f);

	EnableAlphaTest();

	glColor4f(0.f, 0.f, 0.f, 1.f);

	RenderColor(x + (230 / 2) - (50 / 2), y + (180 / 2) - 20, 50, 15, 0.0, 0);

	EndRenderColor();

	DisableAlphaBlend();
}

void CStackInterface::RenderText()
{
	float x = m_Pos.x;
	float y = m_Pos.y;

	g_pRenderText->SetBgColor(0);

	g_pRenderText->SetTextColor(255, 189, 25, 255);

	g_pRenderText->SetFont(g_hFontBold);

	char buffer[15];
	sprintf(buffer, "%d", gJewelStack.m_ValueGetStack);

	g_pRenderText->RenderText(x + (230 / 2) - (50 / 2), y + (180 / 2) - 15, buffer, 50, 0, 3, 0);

	auto itemAttribute = &ItemAttribute[gJewelStack.m_ItemIndex];

	if (itemAttribute)
	{
		g_pRenderText->RenderText(x, y + 10, itemAttribute->Name, 230, 0, 3, 0);
	}

}

float CStackInterface::GetLayerDepth()
{
	return 3.4f;
}

void CStackInterface::LoadImages()
{
	LoadBitmap("Custom\\Interface\\Button_Minus.jpg", 71001, GL_LINEAR, GL_CLAMP);

	LoadBitmap("Custom\\Interface\\Button_Minus_Clicked.jpg", 71002, GL_LINEAR, GL_CLAMP);

	LoadBitmap("Custom\\Interface\\Button_Plus.jpg", 71003, GL_LINEAR, GL_CLAMP);

	LoadBitmap("Custom\\Interface\\Button_Plus_Clicked.jpg", 71004, GL_LINEAR, GL_CLAMP);
}

void CStackInterface::UnloadImages()
{
	DeleteBitmap(71001);
	DeleteBitmap(71002);
	DeleteBitmap(71003);
	DeleteBitmap(71004);
}

void CStackInterface::ResetMouseLButton()
{
	MouseLButton = false;
	MouseLButtonPop = false;
	MouseLButtonPush = false;
}

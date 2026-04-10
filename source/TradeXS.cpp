#include "stdafx.h"
#include "NewUISystem.h"
#include "TradeXS.h"
#include "Platform/RenderColorCompat.h"

SEASON3B::CTradeX::CTradeX()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
}

SEASON3B::CTradeX::~CTradeX()
{
	Release();
}

bool SEASON3B::CTradeX::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if (NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MUCOLLECTOR, this);

	SetPos(x, y);

	LoadImages();

	Show(false);

	return true;
}

void SEASON3B::CTradeX::Release()
{
	UnloadImages();

	if (m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

void SEASON3B::CTradeX::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool SEASON3B::CTradeX::Update()
{
	if (IsVisible())
	{

	}
	return true;
}

bool SEASON3B::CTradeX::UpdateMouseEvent()
{
	POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };
	POINT ptExitBtn2 = { m_Pos.x + 13, m_Pos.y + 391 };

	if (SEASON3B::IsRelease(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_MUCOLLECTOR);
	}

	if (CheckMouseIn(m_Pos.x, m_Pos.y, WINDOW_WIDTH, WINDOW_HEIGHT))
	{
		return false;
	}
	return true;
}

bool SEASON3B::CTradeX::UpdateKeyEvent()
{
	if (IsVisible())
	{
		if (SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_MUCOLLECTOR);

			return false;
		}
	}
	return true;
}

float SEASON3B::CTradeX::GetLayerDepth()
{
	return 3.4;
}

float SEASON3B::CTradeX::GetKeyEventOrder()
{
	return 3.4;
}

bool SEASON3B::CTradeX::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);

	DWORD TextColor = g_pRenderText->GetTextColor();

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(0xFFFFFFFF);

	g_pRenderText->SetBgColor(0);

	RenderImage(IMAGE_BASE_WINDOW_BACK, m_Pos.x, m_Pos.y, float(WINDOW_WIDTH), float(WINDOW_HEIGHT));
	RenderImage(IMAGE_BASE_WINDOW_TOP, m_Pos.x, m_Pos.y, float(WINDOW_WIDTH), 64.f);
	RenderImage(IMAGE_BASE_WINDOW_LEFT, m_Pos.x, m_Pos.y + 64.f, 21.f, float(WINDOW_HEIGHT) - 64.f - 45.f);
	RenderImage(IMAGE_BASE_WINDOW_RIGHT, m_Pos.x + float(WINDOW_WIDTH) - 21.f, m_Pos.y + 64.f, 21.f, float(WINDOW_HEIGHT) - 64.f - 45.f);
	RenderImage(IMAGE_BASE_WINDOW_BOTTOM, m_Pos.x, m_Pos.y + float(WINDOW_HEIGHT) - 45.f, float(WINDOW_WIDTH), 45.f);


	g_pRenderText->SetFont(g_hFontBold);

	g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 13, "Testinho", 190, 0, RT3_SORT_CENTER);

	DisableAlphaBlend();

	return true;
}

void SEASON3B::CTradeX::LoadImages()
{

}

void SEASON3B::CTradeX::UnloadImages()
{

}

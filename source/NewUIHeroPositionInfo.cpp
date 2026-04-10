// NewUIHeroPositionInfo.cpp: implementation of the CNewUIHeroPositionInfo class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIHeroPositionInfo.h"
#include "NewUISystem.h"
#include "wsclientinline.h"
#include "MapManager.h"
#include "iexplorer.h"
#include "Platform/RenderColorCompat.h"

using namespace SEASON3B;

CNewUIHeroPositionInfo::CNewUIHeroPositionInfo()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
	m_CurHeroPosition.x = m_CurHeroPosition.y = 0;
}

CNewUIHeroPositionInfo::~CNewUIHeroPositionInfo()
{
	Release();
}

//---------------------------------------------------------------------------------------------
// Create
bool CNewUIHeroPositionInfo::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_HERO_POSITION_INFO, this );

	WidenX = HERO_POSITION_INFO_BASEB_WINDOW_WIDTH;
	if(WindowWidth == 800)
	{
		WidenX = (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH + (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH * 0.4f));
	}
	else
	if(WindowWidth == 1024)
	{
		WidenX = (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH + (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH * 0.2f));
	}

	SetPos(x, y);
	LoadImages();
	Show( true );
	return true;
}

void CNewUIHeroPositionInfo::Release()
{
	UnloadImages();
	
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

void CNewUIHeroPositionInfo::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool CNewUIHeroPositionInfo::BtnProcess()
{
	
	return false;
}

bool CNewUIHeroPositionInfo::UpdateMouseEvent()
{
	if( true == BtnProcess() )
		return false;

	int Width = HERO_POSITION_INFO_BASEA_WINDOW_WIDTH + WidenX + HERO_POSITION_INFO_BASEC_WINDOW_WIDTH;
	if( CheckMouseIn(m_Pos.x, m_Pos.y, Width, HERO_POSITION_INFO_BASE_WINDOW_HEIGHT) )
		return false;
	
	return true;
}

bool CNewUIHeroPositionInfo::UpdateKeyEvent()
{
	if (SEASON3B::CheckMouseIn(0, 0, 24, 24) == true)
	{
		if (IsPress(1))
		{
			if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_OPTION) == true)
			{
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_OPTION);
			}
			else
			{
				g_pNewUISystem->Show(SEASON3B::INTERFACE_OPTION);
			}

			PlayBuffer(SOUND_CLICK01);
		}
		return false;
	}

#ifdef NEW_MUHELPER_ON
	if (SEASON3B::CheckMouseIn(48, 0, 24, 24) == true)
	{
		if (IsPress(1))
		{
			if (pAIController->IsRunning() != true)
			{
				if (pAIController->CanUseAIController())
				{
					SendRequestStartHelper(0);
				}
				return false;
			}
			else
			{
				SendRequestStartHelper(TRUE);
				return false;
			}
			PlayBuffer(SOUND_CLICK01);
		}
		return false;
	}
#endif

	if (SEASON3B::CheckMouseIn(48, 0, 24, 24) == true)
	{
		if (IsPress(1))
		{
			leaf::OpenExplorer("https://www.discord.com/");
			PlayBuffer(SOUND_CLICK01);
		}
		return false;
	}

#ifdef NEW_MUHELPER_ON
	if (SEASON3B::CheckMouseIn(24, 0, 24, 24) == true)
	{
		if (IsPress(1))
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MACRO_MAIN);
			PlayBuffer(SOUND_CLICK01);
		}
		return false;
	}
#endif
	return true;
}

bool CNewUIHeroPositionInfo::Update()
{
	if( (IsVisible() == true) && (Hero != NULL) )
	{
		m_CurHeroPosition.x = ( Hero->PositionX );
		m_CurHeroPosition.y = ( Hero->PositionY );
	}

	return true;
}

bool CNewUIHeroPositionInfo::Render()
{
	float x = 0.0;

	GWidescreen.CreateNButton(IMAGE_HERO_POSITION_INFO_BASE_WINDOW, x, 00.0f, 24, 24, INTERFACE_OPTION); //-- Menu

	x += 24.f;

	GWidescreen.CreateNButton(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 1, x, 00.0f, 24, 24, INTERFACE_COMMAND); //-- Settings

	x += 24.f;

#ifdef NEW_MUHELPER_ON
	if (pAIController->IsRunning() != true)
	{
		GWidescreen.CreateNButton(140000, x, 00.0f, 24, 24, INTERFACE_COMMAND); //-- Helper Off
	}
	else
	{
		GWidescreen.CreateNButton(140001, x, 00.0f, 24, 24, INTERFACE_COMMAND); //-- Helper On
	}

	x += 24.f;
#endif

	GWidescreen.CreateNButton(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 2, x, 00.0f, 24, 24, INTERFACE_COMMAND); //-- Discord

	glColor4f(1.f, 1.f, 1.f, 1.f);

	if (SEASON3B::CheckMouseIn(0, 0, 24, 24) == true)
	{
		RenderTipText(2, 25, "Menu");
		glColor4f(1.f, 1.f, 1.f, 1.f);
	}

	if (SEASON3B::CheckMouseIn(24, 0, 24, 24) == true)
	{
		RenderTipText(26, 25, GlobalText[3561]);
		glColor4f(1.f, 1.f, 1.f, 1.f);
	}

#ifdef NEW_MUHELPER_ON
	if (SEASON3B::CheckMouseIn(48, 0, 24, 24) == true)
	{
		RenderTipText(26, 25, (pAIController->IsRunning() != true ? GlobalText[3562] : GlobalText[3563]));
		glColor4f(1.f, 1.f, 1.f, 1.f);
	}
#endif

	if (SEASON3B::CheckMouseIn(48, 0, 24, 24) == true)
	{
		RenderTipText(74, 25, "Discord");
		glColor4f(1.f, 1.f, 1.f, 1.f);
	}

	return true;
}

float CNewUIHeroPositionInfo::GetLayerDepth()
{
	return 8.2f;
}

void CNewUIHeroPositionInfo::OpenningProcess()
{
	
}

void CNewUIHeroPositionInfo::ClosingProcess()
{
	
}

void CNewUIHeroPositionInfo::SetCurHeroPosition( int x, int y )
{
	m_CurHeroPosition.x = x;
	m_CurHeroPosition.y = y;
}

void CNewUIHeroPositionInfo::LoadImages()
{
	LoadBitmap("Custom\\NewInterface\\btn_menu.jpg", IMAGE_HERO_POSITION_INFO_BASE_WINDOW, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\btn_settings_v2.jpg", IMAGE_HERO_POSITION_INFO_BASE_WINDOW+1, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\btn_discord.jpg", IMAGE_HERO_POSITION_INFO_BASE_WINDOW+2, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\btn_playt.jpg", 140000, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\btn_pause.jpg", 140001, GL_LINEAR);
}

void CNewUIHeroPositionInfo::UnloadImages()
{
	DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW);
	DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW+1);
	DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW+2);
	DeleteBitmap(140000);
	DeleteBitmap(140001);
}


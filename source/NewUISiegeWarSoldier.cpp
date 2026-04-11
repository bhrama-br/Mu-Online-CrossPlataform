// NewUISiegeWarSoldier.cpp: implementation of the NewUISiegeWarSoldier class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUISiegeWarSoldier.h"
#include "UIControls.h"
#include "ZzzCharacter.h"
#include "wsclientinline.h"
#include "Platform/RenderColorCompat.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUISiegeWarSoldier::CNewUISiegeWarSoldier()
{

}

CNewUISiegeWarSoldier::~CNewUISiegeWarSoldier()
{

}

//---------------------------------------------------------------------------------------------
// OnCreate
bool SEASON3B::CNewUISiegeWarSoldier::OnCreate( int x, int y )
{
	
	return true;
}

//---------------------------------------------------------------------------------------------
// OnRelease
void SEASON3B::CNewUISiegeWarSoldier::OnRelease()
{
	
}

//---------------------------------------------------------------------------------------------
// OnUpdate
bool SEASON3B::CNewUISiegeWarSoldier::OnUpdate()
{
	return true;
}

//---------------------------------------------------------------------------------------------
// OnRender
bool SEASON3B::CNewUISiegeWarSoldier::OnRender()
{
	EnableAlphaTest();
	glColor4f( 1.f, 1.f, 1.f, m_fMiniMapAlpha );
	
	g_pRenderText->SetFont( g_hFontBold );
	g_pRenderText->SetTextColor( 255, 255, 255, 255 );
	g_pRenderText->SetBgColor( 0, 0, 0, 0 );

	// ¸ðµç Ä³¸¯ÅÍÀÇ À§Ä¡
	RenderCharPosInMiniMap();

	DisableAlphaBlend();

	EnableAlphaTest ();
	glColor4f( 1.f, 1.f, 1.f, m_fMiniMapAlpha );

	// Áöµµ»óÀÇ ¸í·É Icon
	RenderCmdIconInMiniMap();

	DisableAlphaBlend();
	
	return true;
}

//---------------------------------------------------------------------------------------------
// OnCreate
void SEASON3B::CNewUISiegeWarSoldier::OnSetPos( int x, int y )
{

}

//---------------------------------------------------------------------------------------------
// RenderCharPosInMiniMap
// ¹Ì´Ï¸Ê¿¡ ¸ðµç Ä³¸¯ÅÍ¸¦ ·»´õ
void SEASON3B::CNewUISiegeWarSoldier::RenderCharPosInMiniMap()
{
	float fPosX, fPosY;
		
	// ¹Ì´Ï¸ã¿¡ ÇÃ·¹ÀÌ¾î ·»´õ
	for ( int i=0; i<MAX_CHARACTERS_CLIENT; ++i )
    {
        CHARACTER* c = &CharactersClient[i];
        if ( c!=NULL && c->Object.Live && c!=Hero && ( c->Object.Kind==KIND_PLAYER || c->Object.Kind==KIND_MONSTER || c->Object.Kind==KIND_NPC ) )
        {
            OBJECT* o = &c->Object;

			if ( g_isCharacterBuff( o, static_cast<eBuffState>(m_dwBuffState) ) )
			{
				glColor4f( 0.f, 1.f, 0.f, m_fMiniMapAlpha );
			}
			else
			{
				glColor4f( 0.8f, 0.f, 0.f, m_fMiniMapAlpha );
			}
			if ( o->Kind==KIND_NPC )
			{
				glColor4f( 1.f, 0.f, 1.f, m_fMiniMapAlpha );
			}
			else if ( o->Kind==KIND_MONSTER && o->Type==MODEL_MONSTER01+86 )
			{
				glColor4f( 1.f, 0.f, 1.f, m_fMiniMapAlpha );
			}
			
			fPosX = ( ( c->PositionX ) )/m_iMiniMapScale - m_MiniMapScaleOffset.x + m_MiniMapPos.x;
			fPosY = ( 256 - ( c->PositionY ) )/m_iMiniMapScale - m_MiniMapScaleOffset.y + m_MiniMapPos.y;
			RenderColor( fPosX, fPosY, 3, 3 );
		}
	}
}

//---------------------------------------------------------------------------------------------
// OnUpdateMouseEvent
bool SEASON3B::CNewUISiegeWarSoldier::OnUpdateMouseEvent()
{
	if( OnBtnProcess() )
		return false;	

	return true;
}

//---------------------------------------------------------------------------------------------
// OnUpdateKeyEvent
bool SEASON3B::CNewUISiegeWarSoldier::OnUpdateKeyEvent()
{
	return true;
}
//---------------------------------------------------------------------------------------------
// OnBtnProcess
bool SEASON3B::CNewUISiegeWarSoldier::OnBtnProcess()
{
	return false;
}

//---------------------------------------------------------------------------------------------
// OnLoadImages
void SEASON3B::CNewUISiegeWarSoldier::OnLoadImages()
{

}

//---------------------------------------------------------------------------------------------
// OnUnloadImages
void SEASON3B::CNewUISiegeWarSoldier::OnUnloadImages()
{

}

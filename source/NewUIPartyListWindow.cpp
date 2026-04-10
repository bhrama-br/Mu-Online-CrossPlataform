// NewUIPartyListWindow.cpp: implementation of the CNewUIPartyInfo class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIPartyListWindow.h"
#include "NewUISystem.h"
#include "wsclientinline.h"
#include "ZzzInventory.h"
#include "CharacterManager.h"
#include "SkillManager.h"
#include "Platform/RenderColorCompat.h"

using namespace SEASON3B;

int biVal_list[6] = { 0,0,0,0,0,0 };

CNewUIPartyListWindow::CNewUIPartyListWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
	m_bActive = false;
	m_iVal = 34;
	m_iLimitUserIDHeight[0] = 48;
	m_iLimitUserIDHeight[1] = 58;
	m_iSelectedCharacter = -1;
	m_minimizeState = false;
	
	for( int i=0 ; i<MAX_PARTYS ; i++ )
	{
		m_iPartyListBGColor[ i ]  = PARTY_LIST_BGCOLOR_DEFAULT;
		m_bPartyMemberoutofSight[ i ] = false;
	}
}

CNewUIPartyListWindow::~CNewUIPartyListWindow()
{
	Release();
}

bool CNewUIPartyListWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_PARTY_INFO_WINDOW, this );

	m_minimizeState = false;

	SetPos(x, y);
	
	LoadImages();
	
	// Exit Party Button Initialize
	for( int i=0 ; i<MAX_PARTYS ; i++ )
	{	
		int iVal = i*m_iVal;
		m_BtnPartyExit[i].ChangeButtonImgState( true, IMAGE_PARTY_LIST_EXIT );
		m_BtnPartyExit[i].ChangeButtonInfo(m_Pos.x + 2 + PARTY_LIST_WINDOW_WIDTH - 3 - 9, m_Pos.y + 20 + iVal + 3, 8, 8);
	}	

	Show( true );
	
	return true;
}

void CNewUIPartyListWindow::Release()
{
	UnloadImages();
	
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

void CNewUIPartyListWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
	
	for( int i=0 ; i<MAX_PARTYS ; i++ )
	{	
		int iVal = i*m_iVal;
		m_BtnPartyExit[i].ChangeButtonInfo(m_Pos.x + 2 + PARTY_LIST_WINDOW_WIDTH - 3 - 9, m_Pos.y + 20 + iVal + 3, 8, 8);
	}
}

void CNewUIPartyListWindow::SetPos(int x)
{
	m_Pos.y = g_pItemEnduranceInfo->GetUIEndPos();
	SetPos((GetScreenWidth() - PARTY_LIST_WINDOW_WIDTH - 5), m_Pos.y);
}

#ifdef NEW_MUHELPER_ON
int SEASON3B::CNewUIPartyListWindow::GetRightBottomPos(POINT* pos)
{
	(*pos).x = m_Pos.x + 0 + (PARTY_LIST_WINDOW_WIDTH + 2);
	(*pos).y = m_Pos.y + 0 + PartyNumber * m_iVal + 2;
	return 0;
}
#endif

int CNewUIPartyListWindow::GetSelectedCharacter()
{
	if( m_iSelectedCharacter == -1 )
		return -1;
	
	return Party[m_iSelectedCharacter].index;
}

void CNewUIPartyListWindow::SetListBGColor( )
{
	for( int i=0 ; i<PartyNumber ; i++)
	{	
		m_iPartyListBGColor[ i ] = PARTY_LIST_BGCOLOR_DEFAULT;
		
		if( Party[i].index == -1 )
		{
			m_iPartyListBGColor[ i ] = PARTY_LIST_BGCOLOR_RED;
		}
		
		if( Party[i].index>-1 )
		{
			m_iPartyListBGColor[ i ]  = PARTY_LIST_BGCOLOR_GREEN;
		}
	}
}

bool CNewUIPartyListWindow::BtnProcess()
{
	m_iSelectedCharacter = -1;

	for( int i=0 ; i<PartyNumber ; i++ )
	{	
		int iVal = i*m_iVal;	
		
		if( !strcmp( Party[0].Name, Hero->ID ) || !strcmp( Party[i].Name, Hero->ID ) )
		{
			if( m_BtnPartyExit[i].UpdateMouseEvent() )
			{
				g_pPartyInfoWindow->LeaveParty( i );
				return true;
			}
		}
		
		if( CheckMouseIn( m_Pos.x, m_Pos.y+iVal, PARTY_LIST_WINDOW_WIDTH, PARTY_LIST_WINDOW_HEIGHT) )
		{
			m_iSelectedCharacter = i;

			if( SelectedCharacter==-1) {
				CHARACTER *c = &CharactersClient[Party[i].index];
				if( c && c != Hero ) {
					CreateChat ( c->ID, "", c );
				}
			}

			if( SelectCharacterInPartyList(	&Party[i] ) )	
			{
				return true;
			}
		}
	}
	
	return false;
}

bool CNewUIPartyListWindow::UpdateMouseEvent()
{
	if( !m_bActive )
		return true;
	
	if( true == BtnProcess() )
		return false;

	int iNextPosY = m_Pos.y;
	if (CheckMouseIn(m_Pos.x, iNextPosY, 100, 20) && SEASON3B::IsPress(VK_LBUTTON)) 
	{
		if (CheckMouseIn(m_Pos.x + 80, iNextPosY + 2, 13, 13) && SEASON3B::IsPress(VK_LBUTTON)) 
		{
			m_minimizeState = !m_minimizeState;

			PlayBuffer(SOUND_CLICK01);
		}

		return false;
	}

	if (PartyNumber > 0)
	{
		int iHeight = (PARTY_LIST_WINDOW_HEIGHT * PartyNumber) + (4 * (PartyNumber - 1));
		if (CheckMouseIn(m_Pos.x - 5, iNextPosY, 100.f, iHeight + 30))
		{
			return false;
		}
	}
	
	return true;
}

bool CNewUIPartyListWindow::UpdateKeyEvent()
{
	return true;
}

bool CNewUIPartyListWindow::Update()
{
	if( PartyNumber <= 0 )
	{
		m_bActive = false;
		return true;
	}
	
	m_bActive = true;
	
	for( int i=0 ; i<PartyNumber ; i++)
	{	
		Party[ i ].index = -2;
	}
	
	return true;
}

bool CNewUIPartyListWindow::Render()
{
	if( !m_bActive )
		return true;

	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	int iNextPosY = m_Pos.y;

	RenderImageScale(951000, m_Pos.x - 5, iNextPosY, 100.f, 18.f, 0.f, 0.f, 120.f, 20.f);
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->RenderText(m_Pos.x - 5 + (95 / 2), iNextPosY + 5.5f, GlobalText[190], 0, 0, RT3_WRITE_CENTER);

	if (m_minimizeState) 
	{
		RenderBitmap(951001, m_Pos.x + 80, iNextPosY + 2, 13, 13, 0.008, 0.263782, 0.51488634, 0.263782, 1, 1, 1.0);
		return true;
	}
	RenderBitmap(951001, m_Pos.x + 80, iNextPosY + 2, 13, 13, 0.008, 0.52488634, 0.51488634, 0.263782, 1, 1, 1.0);

	iNextPosY += 20;

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

	int bIval = 0;

	for (int i = 0; i < PartyNumber; i++)
	{
		int iVal = i * m_iVal;

		iVal += biVal_list[i];

		glColor4f(0.f, 0.f, 0.f, 0.9f);
		RenderColor(float(m_Pos.x + 2), float(iNextPosY + 2 + iVal), PARTY_LIST_WINDOW_WIDTH - 3, 10);
		EnableAlphaTest();

		int addY = 0;

		if (Party[i].index == -1)
		{
			//auto buffs = gPartyBuffs.GetInfoByName(Party[i].Name);
			//if (buffs != nullptr) 
			//{
			//	if (buffs->count > 0) 
			//	{
			//		glColor4f(0.f, 0.f, 0.f, 0.6f);
			//		RenderColor(float(m_Pos.x + 2), float(iNextPosY + 2 + iVal + 10), PARTY_LIST_WINDOW_WIDTH - 3, PARTY_LIST_WINDOW_HEIGHT);
			//		glColor4f(0.3f, 0.f, 0.f, 0.5f);
			//		RenderColor(m_Pos.x + 2, iNextPosY + 2 + iVal + 10, PARTY_LIST_WINDOW_WIDTH - 3, PARTY_LIST_WINDOW_HEIGHT);
			//		addY += 10;
			//		EnableAlphaTest();
			//	}
			//	else 
			//	{
			//		glColor4f(0.f, 0.f, 0.f, 0.6f);
			//		RenderColor(float(m_Pos.x + 2), float(iNextPosY + 2 + iVal + 10), PARTY_LIST_WINDOW_WIDTH - 3, PARTY_LIST_WINDOW_HEIGHT - 10);
			//		glColor4f(0.3f, 0.f, 0.f, 0.5f);
			//		RenderColor(m_Pos.x + 2, iNextPosY + 2 + iVal + 10, PARTY_LIST_WINDOW_WIDTH - 3, PARTY_LIST_WINDOW_HEIGHT - 10);
			//		EnableAlphaTest();
			//	}
			//}
		}
		else
		{
			if (Party[i].index >= 0 && Party[i].index < MAX_CHARACTERS_CLIENT)
			{
				CHARACTER* pChar = &CharactersClient[Party[i].index];
				OBJECT* pObj = &pChar->Object;

				if (g_isCharacterBuff(pObj, eBuff_Defense) == true)
				{
					glColor4f(0.f, 0.f, 0.f, 0.6f);
					RenderColor(float(m_Pos.x + 2), float(iNextPosY + 2 + iVal + 10), PARTY_LIST_WINDOW_WIDTH - 3, PARTY_LIST_WINDOW_HEIGHT);
					glColor4f(0.2f, 1.f, 0.2f, 0.2f);
					RenderColor(m_Pos.x + 2, iNextPosY + 2 + iVal + 10, PARTY_LIST_WINDOW_WIDTH - 3, PARTY_LIST_WINDOW_HEIGHT);
					addY += 10;
					EnableAlphaTest();
				}
				else 
				{
					//auto buffs = gPartyBuffs.GetInfoByName(Party[i].Name);
					//if (buffs != nullptr) 
					//{
					//	if (buffs->count > 0) 
					//	{
					//		glColor4f(0.f, 0.f, 0.f, 0.6f);
					//		RenderColor(float(m_Pos.x + 2), float(iNextPosY + 2 + iVal + 10), PARTY_LIST_WINDOW_WIDTH - 3, PARTY_LIST_WINDOW_HEIGHT);
					//		addY += 10;
					//	}
					//	else 
					//	{
							glColor4f(0.f, 0.f, 0.f, 0.6f);
							RenderColor(float(m_Pos.x + 2), float(iNextPosY + 2 + iVal + 10), PARTY_LIST_WINDOW_WIDTH - 3, PARTY_LIST_WINDOW_HEIGHT - 10);
					//	}
					//}

					EnableAlphaTest();
				}
			}

			if (m_iSelectedCharacter != -1 && m_iSelectedCharacter == i)
			{
				//auto buffs = gPartyBuffs.GetInfoByName(Party[i].Name);
				//if (buffs != nullptr) 
				//{
				//	if (buffs->count > 0) 
				//	{
				//		glColor4f(0.f, 0.f, 0.f, 0.6f);
				//		RenderColor(float(m_Pos.x + 2), float(iNextPosY + 2 + iVal + 10), PARTY_LIST_WINDOW_WIDTH - 3, PARTY_LIST_WINDOW_HEIGHT);
				//		glColor4f(0.4f, 0.4f, 0.4f, 0.7f);
				//		RenderColor(m_Pos.x + 2, iNextPosY + 2 + iVal + 10, PARTY_LIST_WINDOW_WIDTH - 3, PARTY_LIST_WINDOW_HEIGHT);
				//	}
				//	else 
				//	{
				//		glColor4f(0.f, 0.f, 0.f, 0.6f);
				//		RenderColor(float(m_Pos.x + 2), float(iNextPosY + 2 + iVal + 10), PARTY_LIST_WINDOW_WIDTH - 3, PARTY_LIST_WINDOW_HEIGHT - 10);
				//		glColor4f(0.4f, 0.4f, 0.4f, 0.7f);
				//		RenderColor(m_Pos.x + 2, iNextPosY + 2 + iVal + 10, PARTY_LIST_WINDOW_WIDTH - 3, PARTY_LIST_WINDOW_HEIGHT - 10);
				//	}
				//}

				EnableAlphaTest();
			}
		}

		EndRenderColor();

		g_pRenderText->SetFont(g_hFont);
		if (i == 0)
		{
			if (Party[i].index == -1)
			{
				g_pRenderText->SetTextColor(RGBA(128, 75, 11, 255));
			}
			else
			{
				g_pRenderText->SetTextColor(RGBA(255, 148, 22, 255));
			}

			RenderImage(IMAGE_PARTY_LIST_FLAG, m_Pos.x + 4, iNextPosY + 2, 9, 10);
			g_pRenderText->RenderText(m_Pos.x + 2 + ((PARTY_LIST_WINDOW_WIDTH - 3) / 2), iNextPosY + 3.5f + iVal, Party[i].Name, PARTY_LIST_WINDOW_WIDTH - 3, 0, RT3_WRITE_CENTER);

		}
		else
		{
			if (Party[i].index == -1)
			{
				g_pRenderText->SetTextColor(RGBA(128, 128, 128, 255));
			}
			else
			{
				g_pRenderText->SetTextColor(RGBA(255, 255, 255, 255));
			}
			g_pRenderText->RenderText(m_Pos.x + 2 + ((PARTY_LIST_WINDOW_WIDTH - 3) / 2), iNextPosY + 3.5f + iVal, Party[i].Name, PARTY_LIST_WINDOW_WIDTH - 3, 0, RT3_WRITE_CENTER);
		}

		float fLife = ((float)Party[i].currHP / (float)Party[i].maxHP) * (float)PARTY_LIST_HP_BAR_WIDTH;

		RenderImageScale(951002, m_Pos.x + 5, iNextPosY + iVal + 15, PARTY_LIST_HP_BAR_WIDTH, PARTY_LIST_HP_BAR_HEIGHT, 0.f, 0.f, 333.f, 14.f);
		RenderImageScale(951003, m_Pos.x + 5, iNextPosY + iVal + 15, fLife, PARTY_LIST_HP_BAR_HEIGHT, 0.f, 0.f, 333.f, 14.f);

		unicode::t_char szText[256] = { 0, };
		g_pRenderText->SetTextColor(RGBA(237, 214, 161, 255));
		unicode::_sprintf(szText, GlobalText[358], Party[i].currHP, Party[i].maxHP);
		g_pRenderText->RenderText(m_Pos.x + PARTY_LIST_WINDOW_WIDTH - 5, iNextPosY + iVal + 21, szText, 90, 0, RT3_WRITE_RIGHT_TO_LEFT);

		float x = 0.0f, y = 0.0f;
		x = m_Pos.x + 5;
		y = iNextPosY + iVal + 30;
		//auto buffs = gPartyBuffs.GetInfoByName(Party[i].Name);
		//if (buffs != nullptr) 
		//{
		//	if (buffs->count > 0) 
		//	{
		//		gPartyBuffs.DrawPartyBuffs(x, y, Party[i].Name);
		//	}
		//}

		biVal_list[i + 1] = addY;

		if (!strcmp(Party[0].Name, Hero->ID) || !strcmp(Party[i].Name, Hero->ID))
		{
			m_BtnPartyExit[i].ChangeButtonInfo(m_Pos.x + 2 + PARTY_LIST_WINDOW_WIDTH - 3 - 9, m_Pos.y + 20 + iVal + 3, 8, 8);

			m_BtnPartyExit[i].Render();
		}
	}
	
	DisableAlphaBlend();
	
	return true;
}

void SEASON3B::CNewUIPartyListWindow::RenderPartyHPOnHead( )
{
    if ( PartyNumber<=0 ) 
		return;

    float   Width = 38.f;
    char    Text[100];

    for ( int j=0; j<PartyNumber; ++j )
    {
		PARTY_t *p = &Party[j];

        if (p->index<=-1) continue;

        CHARACTER*  c = &CharactersClient[p->index];
	    OBJECT*     o = &c->Object;
	    vec3_t      Position;
	    int         ScreenX, ScreenY;

	    Vector ( o->Position[0], o->Position[1], o->Position[2]+o->BoundingBoxMax[2]+100.f, Position );
		
		BeginOpengl();
		Projection ( Position, &ScreenX, &ScreenY );
		EndOpengl();

        ScreenX -= (int)(Width/2);

	    if((MouseX>=ScreenX && MouseX<ScreenX+Width && MouseY>=ScreenY-2 && MouseY<ScreenY+6))
	    {
		    sprintf ( Text,"HP : %d0%%", p->stepHP );
			g_pRenderText->SetTextColor(255, 230, 210, 255);
		    g_pRenderText->RenderText(ScreenX, ScreenY-6, Text);
	    }

	    EnableAlphaTest ();
   	    glColor4f ( 0.f, 0.f, 0.f, 0.5f );
	    RenderColor ( (float)(ScreenX+1), (float)(ScreenY+1), Width+4.f, 5.f );

	    EnableAlphaBlend ();
	    glColor3f ( 0.2f, 0.0f, 0.0f );
	    RenderColor ( (float)ScreenX, (float)ScreenY, Width+4.f, 5.f );

        glColor3f ( 50.f/255.f, 10/255.f, 0.f );
	    RenderColor ( (float)(ScreenX+2), (float)(ScreenY+2), Width, 1.f );

        int stepHP = min ( 10, p->stepHP );

        glColor3f ( 250.f/255.f, 10/255.f, 0.f );
        for ( int k=0; k<stepHP; ++k )
        {
	        RenderColor ( (float)(ScreenX+2+(k*4)), (float)(ScreenY+2), 3.f, 2.f );
        }
	    DisableAlphaBlend();
    }
	DisableAlphaBlend();
	glColor3f(1.f,1.f,1.f);
}

float CNewUIPartyListWindow::GetLayerDepth()
{
	return 5.4f;
}

void CNewUIPartyListWindow::OpenningProcess()
{
	
}

void CNewUIPartyListWindow::ClosingProcess()
{
	
}

bool CNewUIPartyListWindow::SelectCharacterInPartyList( PARTY_t* pMember )
{
    int HeroClass = gCharacterManager.GetBaseClass( Hero->Class );
	
	if ( HeroClass==CLASS_ELF 
		|| HeroClass==CLASS_WIZARD 
		|| HeroClass == CLASS_SUMMONER
		)
	{
		int Skill = CharacterAttribute->Skill[Hero->CurrentSkill];
		
		if ( Skill==AT_SKILL_HEALING 
			|| Skill==AT_SKILL_DEFENSE 
			|| (AT_SKILL_DEF_POWER_UP <= Skill && Skill <= AT_SKILL_DEF_POWER_UP+4)
			|| (AT_SKILL_ATT_POWER_UP <= Skill && Skill <= AT_SKILL_ATT_POWER_UP+4)
			|| Skill==AT_SKILL_ATTACK 
			|| Skill==AT_SKILL_TELEPORT_B 
			|| Skill==AT_SKILL_WIZARDDEFENSE 
			|| (AT_SKILL_HEAL_UP <= Skill && Skill <= AT_SKILL_HEAL_UP+4)
			|| (AT_SKILL_SOUL_UP <= Skill && Skill <= AT_SKILL_SOUL_UP+4)
			|| Skill == AT_SKILL_ALICE_THORNS
			|| Skill == AT_SKILL_RECOVER
			)
		{
			SelectedCharacter = pMember->index;
			return true;	
		}
	}

	return false;
}

void CNewUIPartyListWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_party_flag.tga", IMAGE_PARTY_LIST_FLAG, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\btn_close_v2.jpg", IMAGE_PARTY_LIST_EXIT, GL_LINEAR);
	LoadBitmap("Custom\\minimize_bar.jpg", 951000, GL_LINEAR);
	LoadBitmap("Custom\\chat_buttons_size.jpg", 951001, GL_LINEAR);
	LoadBitmap("Interface\\newui_statusicon.jpg", 951004, GL_LINEAR);
	LoadBitmap("Interface\\newui_statusicon2.jpg", 951005, GL_LINEAR);
	LoadBitmap("Custom\\party_info_life_back.jpg", 951002, GL_LINEAR);
	LoadBitmap("Custom\\party_info_life_top.jpg", 951003, GL_LINEAR);
}

void CNewUIPartyListWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_PARTY_LIST_FLAG);
	DeleteBitmap(IMAGE_PARTY_LIST_EXIT);
	DeleteBitmap(951000);
	DeleteBitmap(951001);
	DeleteBitmap(951002);
	DeleteBitmap(951003);
	DeleteBitmap(951004);
	DeleteBitmap(951005);
}

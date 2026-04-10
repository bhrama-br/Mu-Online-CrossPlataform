

#include "stdafx.h"
#include "NewUIChatInputBox.h"
#include "NewUIChatLogWindow.h"
#include "UIControls.h"
#include "NewUISystem.h"
#include "wsclientinline.h"
#include "CSChaosCastle.h"
#include "ZzzOpenData.h"
#include "MapManager.h"
#include "Platform/RenderColorCompat.h"
using namespace SEASON3B;

SEASON3B::CNewUIChatInputBox::CNewUIChatInputBox() : MAX_CHAT_SIZE_UTF16((int)(MAX_CHAT_SIZE/(g_pMultiLanguage->GetNumByteForOneCharUTF8())))

{ 
	Init(); 
}

SEASON3B::CNewUIChatInputBox::~CNewUIChatInputBox() 
{ 
	Release(); 
}

void SEASON3B::CNewUIChatInputBox::Init()
{
	m_pNewUIMng = NULL;
	m_pNewUIChatLogWnd = NULL;
	m_WndPos.x = m_WndPos.y = 0;
	m_WndSize.cx = m_WndSize.cy = 0;
	m_pChatInputBox = m_pWhsprIDInputBox = NULL;
	m_iCurChatHistory = m_iCurWhisperIDHistory = 0;

	m_iTooltipType = INPUT_TOOLTIP_NOTHING;
	m_iInputMsgType = INPUT_CHAT_MESSAGE;
	m_bBlockWhisper = false;
	m_bOnlySystemMessage = false;
	m_bShowChatLog = true;
	m_bWhisperSend = true;
	m_ShowMenuChat = false;
	m_posy = 180.f;

	m_bShowMessageElseNormal = false;
}

void SEASON3B::CNewUIChatInputBox::LoadImages()
{
	LoadBitmap("Custom\\NewInterface\\main_frame_chat_button.tga", IMAGE_INPUTBOX_BACK, GL_LINEAR);
}

void SEASON3B::CNewUIChatInputBox::UnloadImages()
{
	DeleteBitmap(IMAGE_INPUTBOX_BACK);
}

bool SEASON3B::CNewUIChatInputBox::Create(CNewUIManager* pNewUIMng, CNewUIChatLogWindow* pNewUIChatLogWnd, int x, int y)
{
	Release();
	
	if(NULL == pNewUIChatLogWnd || NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_CHATINPUTBOX, this);
	
	m_pNewUIChatLogWnd = pNewUIChatLogWnd;
	SetWndPos(x, y);

	m_pChatInputBox = new CUITextInputBox;
	m_pChatInputBox->Init(g_hWnd, 176, 14, MAX_CHAT_SIZE_UTF16-1);
	m_pChatInputBox->SetPosition(GetWindowsCX(252.f), m_WndPos.y + 2);
	m_pChatInputBox->SetTextColor(255, 255, 230, 210);
	m_pChatInputBox->SetBackColor(0, 0, 0, 25);
	m_pChatInputBox->SetFont(g_hFont);
	m_pChatInputBox->SetState(UISTATE_HIDE);

	m_pWhsprIDInputBox = new CUITextInputBox;
	m_pWhsprIDInputBox->Init(g_hWnd, 50, 14, 10);
	m_pWhsprIDInputBox->SetPosition(GetCenterX(640) + 382, m_WndPos.y + 2);
	m_pWhsprIDInputBox->SetSize(50, 14);
	m_pWhsprIDInputBox->SetTextColor(255, 200, 200, 200);
	m_pWhsprIDInputBox->SetBackColor(0, 0, 0, 25);
	m_pWhsprIDInputBox->SetFont(g_hFont);
	m_pWhsprIDInputBox->SetState(UISTATE_HIDE);

	m_pChatInputBox->SetTabTarget(m_pWhsprIDInputBox);
	m_pWhsprIDInputBox->SetTabTarget(m_pChatInputBox);

	LoadImages();

	SetInputMsgType(INPUT_CHAT_MESSAGE);

	Show(false);
	
	return true;
}

void SEASON3B::CNewUIChatInputBox::Release()
{
	UnloadImages();
	
	RemoveAllChatHIstory();
	RemoveAllWhsprIDHIstory();

	SAFE_DELETE(m_pChatInputBox);
	SAFE_DELETE(m_pWhsprIDInputBox);

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
	
	Init();
}

void SEASON3B::CNewUIChatInputBox::SetWndPos(int x, int y)
{
	m_WndPos.x = x; m_WndPos.y = y;
	m_WndSize.cx = CHATBOX_WIDTH; m_WndSize.cy = CHATBOX_HEIGHT;

	if(m_pChatInputBox && m_pWhsprIDInputBox)
	{
		m_pChatInputBox->SetPosition(m_WndPos.x+72, m_WndPos.y+32);
		m_pWhsprIDInputBox->SetPosition(m_WndPos.x+5, m_WndPos.y+32);
	}
}

void SEASON3B::CNewUIChatInputBox::SetInputMsgType(int iInputMsgType)
{
	m_iInputMsgType = iInputMsgType;
}

int SEASON3B::CNewUIChatInputBox::GetInputMsgType() const 
{ 
	return m_iInputMsgType; 
}

void SEASON3B::CNewUIChatInputBox::SetFont(HFONT hFont)
{
	m_pChatInputBox->SetFont(hFont);
	m_pWhsprIDInputBox->SetFont(hFont);
}

bool SEASON3B::CNewUIChatInputBox::HaveFocus() 
{ 
	return (m_pChatInputBox->HaveFocus() || m_pWhsprIDInputBox->HaveFocus()); 
}

void SEASON3B::CNewUIChatInputBox::AddChatHistory(const type_string& strText)
{
	type_vec_history::iterator vi = std::find(m_vecChatHistory.begin(), m_vecChatHistory.end(), strText);
	if(vi != m_vecChatHistory.end())
		m_vecChatHistory.erase(vi);
	else if(m_vecChatHistory.size() > 12)
		m_vecChatHistory.erase(m_vecChatHistory.begin());
	m_vecChatHistory.push_back(strText);
}

void SEASON3B::CNewUIChatInputBox::RemoveChatHistory(int index)
{
	if(index >= 0 && index < (int)m_vecChatHistory.size())
		m_vecChatHistory.erase(m_vecChatHistory.begin() + index);
}

void SEASON3B::CNewUIChatInputBox::RemoveAllChatHIstory()
{
	m_vecChatHistory.clear();
}

void SEASON3B::CNewUIChatInputBox::AddWhsprIDHistory(const type_string& strWhsprID)
{
	type_vec_history::iterator vi = std::find(m_vecWhsprIDHistory.begin(), m_vecWhsprIDHistory.end(), strWhsprID);
	if(vi != m_vecWhsprIDHistory.end())
		m_vecWhsprIDHistory.erase(vi);
	else if(m_vecWhsprIDHistory.size() > 5)
		m_vecWhsprIDHistory.erase(m_vecWhsprIDHistory.begin());
	m_vecWhsprIDHistory.push_back(strWhsprID);
}

void SEASON3B::CNewUIChatInputBox::RemoveWhsprIDHistory(int index)
{
	if(index >= 0 && index < (int)m_vecWhsprIDHistory.size())
	{
		m_vecWhsprIDHistory.erase(m_vecWhsprIDHistory.begin() + index);
	}
}

void SEASON3B::CNewUIChatInputBox::RemoveAllWhsprIDHIstory()
{
	m_vecWhsprIDHistory.clear();
}

bool SEASON3B::CNewUIChatInputBox::IsBlockWhisper()
{
	return m_bBlockWhisper;
}

void SEASON3B::CNewUIChatInputBox::SetBlockWhisper(bool bBlockWhisper)
{
	m_bBlockWhisper = bBlockWhisper;
}
bool SEASON3B::CNewUIChatInputBox::UpdateMouseEvent()
{
	float x = m_WndPos.x;

	x = GetWindowsCX(254.f);

	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX) == false)
	{
		return true;
	}

	auto const releaseMouse = SEASON3B::IsRelease(VK_LBUTTON);

	if (SelectedCharacter >= 0 && (IsVisible() && releaseMouse))
	{
		auto const character = &CharactersClient[SelectedCharacter];

		if (character->Object.Kind == KIND_PLAYER
			&& !gMapManager.InChaosCastle()
			&& !(::IsStrifeMap(gMapManager.WorldActive)
				&& Hero->m_byGensInfluence != character->m_byGensInfluence))
		{
			SetWhsprID(character->ID);
		}
	}

	m_iTooltipType = INPUT_TOOLTIP_NOTHING;

	if (IsRelease(VK_LBUTTON))
	{
		if (m_ShowMenuChat)
		{
			int i = 0;

			for (i = 0; i < 7; i++)
			{
				if (i >= 0 && i <= 1)
				{
					if (SEASON3B::CheckMouseIn(x + 256, m_WndPos.y - (i * 18) - 16, 40, 16))
					{
						if (i == 1)
						{
							m_bOnlySystemMessage = !m_bOnlySystemMessage;

							if (m_bOnlySystemMessage)
							{
								m_pNewUIChatLogWnd->ChangeMessage(TYPE_SYSTEM_MESSAGE);
							}
							else
							{
								m_pNewUIChatLogWnd->ChangeMessage(TYPE_ALL_MESSAGE);
							}
						}
						if (i == 0)
						{
							if (m_pNewUIChatLogWnd->m_ShowSystemMessage)
							{
								//m_pNewUIChatLogWnd->ChangeMessage(TYPE_SYSTEM_MESSAGE);
								m_pNewUIChatLogWnd->m_ShowSystemMessage = 0;
							}
							else
							{
								//m_pNewUIChatLogWnd->ChangeMessage(TYPE_ALL_MESSAGE);
								m_pNewUIChatLogWnd->m_ShowSystemMessage = 1;
							}
						}


						PlayBuffer(25, 0, 0);
						return true;
					}
				}
				else if (i >= 4 && i <= 6)
				{
					if (SEASON3B::CheckMouseIn(x + 256, m_WndPos.y - ((i - 2) * 18) - 16, 40, 16))
					{
						if (i == 6)
						{
							if (m_pNewUIChatLogWnd->m_ShowGuildMessage == 0)
							{
								m_pNewUIChatLogWnd->m_ShowPartyMessage = 0;
								m_pNewUIChatLogWnd->m_ShowGuildMessage = 1;
								SetInputMsgType(INPUT_GUILD_MESSAGE);
							}
							else
							{
								m_pNewUIChatLogWnd->m_ShowGuildMessage = 0;
								SetInputMsgType(INPUT_CHAT_MESSAGE);
							}
						}
						else if (i == 5)
						{
							if (m_pNewUIChatLogWnd->m_ShowPartyMessage == 0)
							{
								m_pNewUIChatLogWnd->m_ShowPartyMessage = 1;
								m_pNewUIChatLogWnd->m_ShowGuildMessage = 0;
								SetInputMsgType(INPUT_PARTY_MESSAGE);
							}
							else
							{
								m_pNewUIChatLogWnd->m_ShowPartyMessage = 0;
								SetInputMsgType(INPUT_CHAT_MESSAGE);
							}
						}
						else
						{
							if (i == 4)
							{
								if (m_pNewUIChatLogWnd->m_ShowWhisperMessage == 0)
								{
									m_pNewUIChatLogWnd->m_ShowWhisperMessage = 1;
									m_pNewUIChatLogWnd->ChangeMessage(TYPE_WHISPER_MESSAGE);
								}
								else
								{
									m_pNewUIChatLogWnd->m_ShowWhisperMessage = 0;
									m_pNewUIChatLogWnd->ChangeMessage(TYPE_ALL_MESSAGE);
								}
							}
						}

						PlayBuffer(25, 0, 0);
						return true;
					}
				}
			}

			if (SEASON3B::CheckMouseIn(x + 256, m_WndPos.y - (i * 18) + 24, 12, 12)) //-- size
			{
				if (m_pNewUIChatLogWnd->IsShowFrame())
				{
					m_pNewUIChatLogWnd->SetSizeAuto();
					m_pNewUIChatLogWnd->UpdateWndSize();
					m_pNewUIChatLogWnd->UpdateScrollPos();
				}

				PlayBuffer(25, 0, 0);
				return false;
			}
			if (SEASON3B::CheckMouseIn(x + 270, m_WndPos.y - (i * 18) + 24, 12, 12)) //-- alpha
			{
				if (m_pNewUIChatLogWnd->IsShowFrame())
				{
					m_pNewUIChatLogWnd->SetBackAlphaAuto();
				}
				PlayBuffer(25, 0, 0);
				return false;
			}
			if (SEASON3B::CheckMouseIn(x + 284, m_WndPos.y - (i * 18) + 24, 12, 12)) //-- active
			{
				if (m_pNewUIChatLogWnd->IsShowFrame())
				{
					m_pNewUIChatLogWnd->HideFrame();
				}
				else
				{
					m_pNewUIChatLogWnd->ShowFrame();
				}

				PlayBuffer(25, 0, 0);
				return false;
			}
		}

		if (SEASON3B::CheckMouseIn(x + 238, m_WndPos.y, 16, 16))
		{
			m_ShowMenuChat = !m_ShowMenuChat;
			return false;
		}
	}

	if (m_ShowMenuChat && SEASON3B::CheckMouseIn(x + 256, m_WndPos.y - 160, 40, 160))
	{
		return false;
	}

	return !CheckMouseIn(x, m_WndPos.y, m_WndSize.cx, m_WndSize.cy);
}

bool SEASON3B::CNewUIChatInputBox::UpdateKeyEvent()
{
	if(SEASON3B::IsPress(VK_F2))
	{
		m_bShowMessageElseNormal = !m_bShowMessageElseNormal;
		
		if(m_bShowMessageElseNormal)
		{
			m_pNewUIChatLogWnd->ChangeMessage( SEASON3B::TYPE_WHISPER_MESSAGE );
		}
		else
		{
			m_pNewUIChatLogWnd->ChangeMessage( SEASON3B::TYPE_ALL_MESSAGE );
		}

		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	
	if(SEASON3B::IsPress(VK_F3))
	{
		if(m_bWhisperSend == false)
		{
			m_bWhisperSend = true;
			m_pWhsprIDInputBox->SetState(UISTATE_NORMAL);
		}
		else
		{
			m_bWhisperSend = false;
			m_pWhsprIDInputBox->SetState(UISTATE_HIDE);
			if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX))
			{
				m_pChatInputBox->GiveFocus();
			}
		}

		return false;
	}

	if(m_pNewUIChatLogWnd->IsShowFrame())
	{
		if(SEASON3B::IsPress(VK_F4))
		{	
			m_pNewUIChatLogWnd->SetSizeAuto();
			m_pNewUIChatLogWnd->UpdateWndSize();
			m_pNewUIChatLogWnd->UpdateScrollPos();
			PlayBuffer(SOUND_CLICK01);	
			return false;
		}
	}

	if(SEASON3B::IsPress(VK_F5))
	{
		if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX))
		{
			if(m_pNewUIChatLogWnd->IsShowFrame())
			{
				m_pNewUIChatLogWnd->HideFrame();
			}
			else
			{
				m_pNewUIChatLogWnd->ShowFrame();
			}
		}
		
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	
	if(m_pNewUIChatLogWnd->IsShowFrame())
	{
		if(IsPress(VK_PRIOR))
		{
			m_pNewUIChatLogWnd->Scrolling(m_pNewUIChatLogWnd->GetCurrentRenderEndLine() - m_pNewUIChatLogWnd->GetNumberOfShowingLines());
			return false;
		}
		if(IsPress(VK_NEXT))
		{
			m_pNewUIChatLogWnd->Scrolling(m_pNewUIChatLogWnd->GetCurrentRenderEndLine() + m_pNewUIChatLogWnd->GetNumberOfShowingLines());
			return false;
		}	
	}

	if( false == IsVisible() && SEASON3B::IsPress(VK_RETURN) )
	{
		if(gMapManager.InChaosCastle() == true && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHAOSCASTLE_TIME) == false)
		{
			g_pNewUISystem->Show(SEASON3B::INTERFACE_CHATINPUTBOX);
		}
		else if(gMapManager.InChaosCastle() == false)
		{
			g_pNewUISystem->Show(SEASON3B::INTERFACE_CHATINPUTBOX);
		}

		RestoreIMEStatus();
		return false;
	}

	if(IsVisible() && HaveFocus() && SEASON3B::IsPress(VK_RETURN))
	{
		char	szChatText[MAX_CHAT_SIZE+1]	= {'\0'};
		char	szWhisperID[MAX_ID_SIZE+1]	= {'\0'};		
		wchar_t *szReceivedChat = new wchar_t[MAX_CHAT_SIZE_UTF16];
		
		m_pChatInputBox->GetText(szReceivedChat, MAX_CHAT_SIZE_UTF16);
		m_pWhsprIDInputBox->GetText(szWhisperID, MAX_ID_SIZE+1);

		for (int i = 0; i < MAX_CHAT_SIZE_UTF16; i++)
			szReceivedChat[i] = g_pMultiLanguage->ConvertFulltoHalfWidthChar(szReceivedChat[i]);

		std::wstring wstrText = L"";
		
		if (szReceivedChat[0] != 0x002F)
		{
			switch(m_iInputMsgType) {
			case INPUT_PARTY_MESSAGE:
				wstrText = L"~";
				break;
			case INPUT_GUILD_MESSAGE:
				wstrText = L"@";
				break;
			case INPUT_GENS_MESSAGE:
				wstrText = L"$";
				break;
			default:
				break;
			}
		}
		wstrText.append(szReceivedChat);

		delete [] szReceivedChat;

		std::string strText;

		g_pMultiLanguage->ConvertWideCharToStr(strText, wstrText.c_str(), g_pMultiLanguage->GetCodePage());
		strncpy(szChatText, strText.c_str(), sizeof szChatText);
		
		if(unicode::_strlen(szChatText) != 0)
		{	
			if(!CheckCommand(szChatText))
			{
				{
					if(CheckAbuseFilter(szChatText))
					{
						g_pMultiLanguage->ConvertCharToWideStr(wstrText, GlobalText[570]);
					}
					
					if(m_pWhsprIDInputBox->GetState() == UISTATE_NORMAL && unicode::_strlen(szChatText) && strlen(szWhisperID) > 0 )
					{
						g_pMultiLanguage->ConvertWideCharToStr(strText, wstrText.c_str(), CP_UTF8);
						strncpy(szChatText, strText.c_str(), sizeof szChatText);
						SendChatWhisper(szWhisperID, szChatText);
						g_pChatListBox->AddText(Hero->ID, szChatText, SEASON3B::TYPE_WHISPER_MESSAGE);
						AddWhsprIDHistory(szWhisperID);
					}
					else if (strncmp(szChatText, GlobalText[260], strlen(GlobalText[260])) == 0)
					{
						char* pszMapName = szChatText + strlen(GlobalText[260])+1;
						int iMapIndex = g_pMoveCommandWindow->GetMapIndexFromMovereq(pszMapName);

						if (g_pMoveCommandWindow->IsTheMapInDifferentServer(gMapManager.WorldActive, iMapIndex))
						{
							SaveOptions();
						}
						
						SendRequestMoveMap(g_pMoveCommandWindow->GetMoveCommandKey(), iMapIndex);
					}
					else
					{	
						if ( Hero->SafeZone || (Hero->Helper.Type != MODEL_HELPER+2 && Hero->Helper.Type != MODEL_HELPER+3 && Hero->Helper.Type != MODEL_HELPER+4 && Hero->Helper.Type != MODEL_HELPER+37) ) 
						{
							CheckChatText(szChatText);
						}
						
						g_pMultiLanguage->ConvertWideCharToStr(strText, wstrText.c_str(), CP_UTF8);
						strncpy(szChatText, strText.c_str(), sizeof szChatText);
						SendChat(szChatText);
						AddChatHistory(szChatText);
					}
				}
			}
		}
		m_pChatInputBox->SetText("");
		m_iCurChatHistory = m_iCurWhisperIDHistory = 0;

		SaveIMEStatus();

		g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHATINPUTBOX);
		return false;
	}
	if(IsVisible() && m_pChatInputBox->HaveFocus())
	{
		if(SEASON3B::IsPress(VK_UP) && false == m_vecChatHistory.empty())
		{
			m_iCurChatHistory--;
			if(m_iCurChatHistory < 0)
				m_iCurChatHistory = m_vecChatHistory.size()-1;
			m_pChatInputBox->SetText(m_vecChatHistory[m_iCurChatHistory].c_str());

			return false;
		}
		else if(SEASON3B::IsPress(VK_DOWN) && false == m_vecChatHistory.empty())
		{
			m_iCurChatHistory++;

			if(m_iCurChatHistory >= (int)m_vecChatHistory.size())
				m_iCurChatHistory = 0;

			m_pChatInputBox->SetText(m_vecChatHistory[m_iCurChatHistory].c_str());

			return false;
		}
	}
	
	if(IsVisible() && m_pWhsprIDInputBox->HaveFocus() 
		&& m_pWhsprIDInputBox->GetState() == UISTATE_NORMAL)
	{
		if(SEASON3B::IsPress(VK_UP) && false == m_vecWhsprIDHistory.empty())
		{
			m_iCurWhisperIDHistory--;
			if(m_iCurWhisperIDHistory < 0)
				m_iCurWhisperIDHistory = m_vecWhsprIDHistory.size()-1;
			m_pWhsprIDInputBox->SetText(m_vecWhsprIDHistory[m_iCurWhisperIDHistory].c_str());

			return false;
		}
		else if(SEASON3B::IsPress(VK_DOWN) && false == m_vecWhsprIDHistory.empty())
		{
			m_iCurWhisperIDHistory++;

			if(m_iCurWhisperIDHistory >= (int)m_vecWhsprIDHistory.size())
				m_iCurWhisperIDHistory = 0;

			m_pWhsprIDInputBox->SetText(m_vecWhsprIDHistory[m_iCurWhisperIDHistory].c_str());

			return false;
		}
	}
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHATINPUTBOX);

			PlayBuffer(SOUND_CLICK01);

			return false;
		}
	}

	return true;
}

bool SEASON3B::CNewUIChatInputBox::Update()
{
	if(m_pChatInputBox->HaveFocus() && GetRelatedWnd() != m_pChatInputBox->GetHandle())
	{
		SetRelatedWnd(m_pChatInputBox->GetHandle());
	}
	if(m_pWhsprIDInputBox->HaveFocus() && GetRelatedWnd() != m_pWhsprIDInputBox->GetHandle())
	{
		SetRelatedWnd(m_pWhsprIDInputBox->GetHandle());
	}
	if(!HaveFocus() && GetRelatedWnd() != g_hWnd)
	{
		SetRelatedWnd(g_hWnd);
	}

	if(IsVisible() == false)
	{
		return true;
	}
	
	m_pChatInputBox->DoAction();
	
	if(m_pWhsprIDInputBox->GetState() == UISTATE_NORMAL)
	{
		m_pWhsprIDInputBox->DoAction();
	}
	
	m_pChatInputBox->DoAction();
	
	if(m_pWhsprIDInputBox->GetState() == UISTATE_NORMAL)
	{
		m_pWhsprIDInputBox->DoAction();
	}

	return true;
}

bool SEASON3B::CNewUIChatInputBox::Render()
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	RenderFrame();
	m_pChatInputBox->Render();
	m_pWhsprIDInputBox->Render();

	if(m_bWhisperSend == false)
	{
		char szWhisperID[32];
		m_pWhsprIDInputBox->GetText(szWhisperID, 32);
		g_pRenderText->SetTextColor(255, 255, 255, 100);
		g_pRenderText->RenderText(m_pWhsprIDInputBox->GetPosition_x(), m_pWhsprIDInputBox->GetPosition_y(), szWhisperID);

		EnableAlphaTest();
		glColor4f(0.5f, 0.2f, 0.2f, 0.2f);
		RenderColor(m_WndPos.x+2, m_WndPos.y+28, 61, 17);
		EndRenderColor();
	}
			
	DisableAlphaBlend();
	
	return true;
}

void SEASON3B::CNewUIChatInputBox::RenderButtonProcess(float x, float y, float w, float h, float su)
{
	float sv = 78.f;
	if (SEASON3B::CheckMouseIn((int)x, (int)y, (int)w, (int)h))
	{
		sv -= 38.f;

		if (MouseLButton)
		{
			sv -= 38.f;
		}
	}
	GWidescreen.RenderImageF(IMAGE_INPUTBOX_BACK, x, y, w, h, su, sv, 34.f, 34.f);
}

void SEASON3B::CNewUIChatInputBox::RenderButtonProcess(float x, float y, float w, float h, int Msgs, bool active)
{
	float sv = 38.f;
	if (SEASON3B::CheckMouseIn((int)x, (int)y, (int)w, (int)h))
	{
		if (MouseLButton)
		{
			sv -= 38.f;
		}
	}
	if (active)
	{
		sv = 76.f;
	}
	GWidescreen.RenderImageF(IMAGE_INPUTBOX_BACK, x, y, w, h, 2.f + (Msgs * 92.f), sv, 88.5f, 34.f);
}

bool SEASON3B::CNewUIChatInputBox::RenderFrame()
{
	float x = m_WndPos.x;
	float y = m_WndPos.y;
	float width = CHATBOX_WIDTH;
	float height = CHATBOX_HEIGHT;

	x = GetWindowsCX(254.f);

	glColor4f(0.f, 0.f, 0.f, 0.8f);

	RenderColor(x, y, 184.f, 16.f, 0.0, 0);

	RenderColor(x + 188.f, y, 46.f, 16.f, 0.0, 0);

	EndRenderColor();

	glColor4f(1.0, 1.0, 1.0, 1.0);

	EnableAlphaTest(true);

	RenderButtonProcess(x + 238, y, 16.f, 16.f, 646.f);

	if (m_ShowMenuChat)
	{
		int i;

		for (i = 0; i < 7; i++)
		{
			if (i >= 0 && i <= 1 || i > 3)
			{
				if (i >= 4 && i <= 6)
				{
					RenderButtonProcess(x + 256.f, y - ((i - 2) * 18.f) - 16.f, 40.f, 16.f, i, 0);
				}
				else
				{
					RenderButtonProcess(x + 256.f, y - (i * 18.f) - 16.f, 40.f, 16.f, i, 0);
				}
			}

			if (i == 6)
			{
				RenderButtonProcess(x + 256.f, y - ((i - 2) * 18.f) - 16.f, 40.f, 16.f, i, m_pNewUIChatLogWnd->m_ShowGuildMessage == 1);
			}
			else if (i == 5)
			{
				RenderButtonProcess(x + 256.f, y - ((i - 2) * 18.f) - 16.f, 40.f, 16.f, i, m_pNewUIChatLogWnd->m_ShowPartyMessage == 1);
			}
			else
			{
				if (i == 4)
				{
					RenderButtonProcess(x + 256.f, y - ((i - 2) * 18.f) - 16.f, 40.f, 16.f, i, m_pNewUIChatLogWnd->m_ShowWhisperMessage == 1);
				}
				//else if (i == 3)
				//{
				//	RenderButtonProcess(x + 256.f, y - (i * 18.f) - 16.f, 40.f, 16.f, i, m_pNewUIChatLogWnd->GetCurrentMsgType() == TYPE_GM_MESSAGE);
				//}
				else if (i == 1)
				{
					RenderButtonProcess(x + 256.f, y - (i * 18.f) - 16.f, 40.f, 16.f, i, m_pNewUIChatLogWnd->GetCurrentMsgType() == TYPE_SYSTEM_MESSAGE);
				}
				else if (i == 0)
				{
					RenderButtonProcess(x + 256.f, y - (i * 18.f) - 16.f, 40.f, 16.f, i, m_pNewUIChatLogWnd->m_ShowSystemMessage == 1);
				}
				else
				{
					//RenderButtonProcess(x + 256.f, y - (i * 18.f) - 16.f, 40.f, 16.f, i, false);
				}
			}
		}

		RenderButtonProcess(x + 256.f, y - (i * 18.f) + 24.f, 12.f, 12.f, 684.f);
		RenderButtonProcess(x + 270.f, y - (i * 18.f) + 24.f, 12.f, 12.f, 722.f);
		RenderButtonProcess(x + 284.f, y - (i * 18.f) + 24.f, 12.f, 12.f, 812.f);

	}
	return true;
}

float SEASON3B::CNewUIChatInputBox::GetLayerDepth() 
{
	return 6.2f; 
}

float SEASON3B::CNewUIChatInputBox::GetKeyEventOrder() 
{ 
	return 9.0f; 
}

void SEASON3B::CNewUIChatInputBox::OpenningProcess()
{
	m_pChatInputBox->GiveFocus();
	m_pChatInputBox->SetState(UISTATE_NORMAL);
	m_pChatInputBox->SetText("");

	if(m_bWhisperSend == true)
	{
		m_pWhsprIDInputBox->SetState(UISTATE_NORMAL);
	}
	else
	{
		m_pWhsprIDInputBox->SetState(UISTATE_HIDE);
	}
}

void SEASON3B::CNewUIChatInputBox::ClosingProcess()
{
	m_pNewUIChatLogWnd->HideFrame();

	m_pChatInputBox->SetState(UISTATE_HIDE);
	m_pWhsprIDInputBox->SetState(UISTATE_HIDE);

	SetFocus(g_hWnd);
}

void SEASON3B::CNewUIChatInputBox::GetChatText(type_string& strText)
{
	unicode::t_char szChatText[256];
	m_pChatInputBox->GetText(szChatText, 256);
	strText = szChatText;
}
void SEASON3B::CNewUIChatInputBox::GetWhsprID(type_string& strWhsprID)
{
	char szWhisperID[32];
	m_pWhsprIDInputBox->GetText(szWhisperID, 32);
	strWhsprID = szWhisperID;
}

void SEASON3B::CNewUIChatInputBox::SetWhsprID(const char* strWhsprID)
{
	m_pWhsprIDInputBox->SetText(strWhsprID);
}

void SEASON3B::CNewUIChatInputBox::SetTextPosition(int x, int y) 
{ 
	m_pChatInputBox->SetPosition(x, y); 
}

void SEASON3B::CNewUIChatInputBox::SetBuddyPosition(int x, int y) 
{ 
	m_pWhsprIDInputBox->SetPosition(x, y); 
}

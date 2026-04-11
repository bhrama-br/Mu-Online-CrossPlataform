#include "stdafx.h"
#include "TradeX.h"
#include "ItemMove.h"
#include "NewUIInventoryCtrl.h"
#include "ZzzInventory.h"
#include "UIControls.h"
#include "ZzzCharacter.h"
#include "NewUISystem.h"
#include "NewUIChatLogWindow.h"
#include "Platform/RenderColorCompat.h"

CTradeX gTradeX;

CTradeX::CTradeX()
{
	this->m_PosX = 260;

	this->m_PosY = 0;

	this->m_isVisible = false;

	this->m_TradeItemSource.clear();

	this->m_TradeItemTarget.clear();

	m_iMouseYSource = 0;

	m_iMouseYTarget = 0;

	TradeXHidden = 0;

	TradeXTransation = 0;

	TimeTradeX = 10;

	TickBlockOk = 0;

	this->m_nBackupMouseX = 0;

	this->m_nBackupMouseY = 0;

	this->m_bItemAutoMove = false;

	memcpy(NameTarget, "OgocX", sizeof(NameTarget));
}

CTradeX::~CTradeX()
{
	std::vector<TradeItem>::iterator target = this->m_TradeItemTarget.begin();

	for (; target != this->m_TradeItemTarget.end(); target++) 
	{
		target->Clear();
	}

	this->m_TradeItemTarget.clear();

	std::vector<TradeItem>::iterator source = this->m_TradeItemSource.begin();

	for (; source != this->m_TradeItemSource.end(); source++) 
	{
		source->Clear();
	}

	this->m_TradeItemSource.clear();
}

bool CTradeX::isVisible()
{
	return this->m_isVisible;
}

void CTradeX::SetItemAutoMove(bool ItemAutoMove)
{
	m_bItemAutoMove = ItemAutoMove;

	if (ItemAutoMove)
	{
		m_nBackupMouseX = MouseX;
		m_nBackupMouseY = MouseY;
	}
	else
	{
		m_nBackupMouseX = m_nBackupMouseY = 0;
	}

}

bool CTradeX::CheckMouse()
{
	if (CheckMouseIn(this->m_PosX, this->m_PosY, 190, 429)) {
		return true;
	}

	return false;
}

void CTradeX::Open()
{
	this->m_PosX = 260;
	this->m_PosY = 0;

	this->m_isVisible = 1;

	m_EventStateTarget = EVENT_NONE;
	m_EventStateSource = EVENT_NONE;

	ScrollBarPosXTarget = (float)(this->m_PosX + 160);

	ScrollBarPosXSource = (float)(this->m_PosX + 160);

	m_iMouseYSource = 0;
	m_iMouseYTarget = 0;
}

void CTradeX::Close()
{
}

void CTradeX::ClearTradeX()
{
	this->ClearTradeXSource();
	this->ClearTradeXTarget();

	this->m_isVisible = 0;

	memcpy(this->NameTarget, "OgocX", sizeof(this->NameTarget));

	this->TradeXTransation = 0;

	this->TradeXHidden = 0;

	this->m_SourceAcceptTradeX = 0;
	this->m_TargetAcceptTradeX = 0;

	this->TickBlockOk = 0;

	this->ClickEvent = GetTickCount();
}

void CTradeX::Render()
{
	if (!this->isVisible()) 
	{
		return;
	}

	EnableAlphaTest(1);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	this->RenderFrame();

	this->RenderText();

	this->RenderButtom();

	this->RenderScrollTarget((float)(this->m_PosX + 15), (float)(this->m_PosY + 105), 160, 90);

	this->RenderScrollSource((float)(this->m_PosX + 15), (float)(this->m_PosY + 275), 160, 90);

	DisableAlphaBlend();
}

void CTradeX::RenderFrame()
{
	SEASON3B::RenderImage(31319, (float)(this->m_PosX), (float)(this->m_PosY), 190.0, 429.0);
	SEASON3B::RenderImage(31347, (float)(this->m_PosX), (float)(this->m_PosY), 190.0, 64.0);
	SEASON3B::RenderImage(31349, (float)(this->m_PosX), (float)(this->m_PosY + 64), 21.0, 320.0);
	SEASON3B::RenderImage(31350, (float)(this->m_PosX + 169), (float)(this->m_PosY + 64), 21.0, 320.0);
	SEASON3B::RenderImage(31351, (float)(this->m_PosX), (float)(this->m_PosY + 384), 190.0, 45.0);

	if (this->m_TargetAcceptTradeX) 
	{
		RenderBitmap(31736, (float)(this->m_PosX + 14), (float)(this->m_PosY + 87), 13, 13, 0.0, 0.5, 1.0, 0.5, 1, 1, 1.0);
	}
	else 
	{
		RenderBitmap(31736, (float)(this->m_PosX + 14), (float)(this->m_PosY + 87), 13, 13, 0.0, 0.0, 1.0, 0.5, 1, 1, 1.0);
	}

	this->RenderBoxItem((float)(this->m_PosX + 15), (float)(this->m_PosY + 105), 160, 90);

	if (this->m_SourceAcceptTradeX) 
	{
		RenderBitmap(31736, this->m_PosX + 14, this->m_PosY + 258, 13, 13, 0.0, 0.5, 1.0, 0.5, 1, 1, 1.0);
	}
	else 
	{
		RenderBitmap(31736, this->m_PosX + 14, this->m_PosY + 258, 13, 13, 0.0, 0.0, 1.0, 0.5, 1, 1, 1.0);
	}

	this->RenderBoxItem(this->m_PosX + 15, this->m_PosY + 275, 160, 90);

	this->RenderItensSource();

	this->RenderItensTarget();
}

void CTradeX::RenderText()
{
	char buffer[256];

	g_pRenderText->SetFont(g_hFontBold);

	g_pRenderText->SetBgColor(0);

	g_pRenderText->SetTextColor(0xD8, 0xD8, 0xD8, 0xFF);

	g_pRenderText->RenderText(this->m_PosX, this->m_PosY + 11, "Extended exchange system", 190, 0, 3, 0);

	g_pRenderText->SetFont(g_hFont);

	g_pRenderText->SetTextColor(225, 225, 225, 0xFF);

	SIZE pBox;

	g_pRenderText->RenderText(this->m_PosX + 20, this->m_PosY + 45, "Transaction status:", 190, 0, 1, &pBox);

	g_pRenderText->SetTextColor(0xFF, 0xFF, 0xFF, 0xFF);

	if (gTradeX.TradeXTransation == 0) 
	{
		sprintf_s(buffer, "Pending");
	}
	else if (gTradeX.TradeXTransation == 1) 
	{
		g_pRenderText->SetTextColor(172, 255, 56, 255);

		sprintf_s(buffer, "Completed");
	}
	else if (gTradeX.TradeXTransation == 2) 
	{
		g_pRenderText->SetTextColor(225, 0, 0, 255);

		sprintf_s(buffer, "Canceled");
	}

	g_pRenderText->RenderText(this->m_PosX + 95 + pBox.cx, this->m_PosY + 45, buffer, 190, 0, 7, 0);

	this->RenderTextSource();

	this->RenderTextTarget();
}

void CTradeX::RenderButtom() 
{
	char buffer[150];

	if (CheckMouseIn(this->m_PosX + 23, this->m_PosY + 383, 60, 18))
	{
		RenderBitmap(150000, (float)(this->m_PosX + 23), (float)(this->m_PosY + 383), 60, 18, 0.002, 0.213, 0.95, 0.20, 1, 1, 1.0);
	}
	else 
	{
		RenderBitmap(150000, (float)(this->m_PosX + 23), (float)(this->m_PosY + 383), 60, 18, 0.002, 0.002, 0.95, 0.20, 1, 1, 1.0);
	}

	g_pRenderText->SetFont(g_hFontBold);

	g_pRenderText->SetTextColor(225, 225, 225, 255);

	g_pRenderText->RenderText((float)(this->m_PosX + 20), (float)(this->m_PosY + 389), "Cancel", 65, 0, 3, 0);

	DWORD tick = (DWORD)((GetTickCount() - this->TickBlockOk) / 1000);

	if (CheckMouseIn(this->m_PosX + 103, this->m_PosY + 383, 60, 18))
	{
		RenderBitmap(150000, (float)(this->m_PosX + 103), (float)(this->m_PosY + 383), 60, 18, 0.002, 0.213, 0.95, 0.20, 1, 1, 1.0);
	}
	else 
	{
		RenderBitmap(150000, (float)(this->m_PosX + 103), (float)(this->m_PosY + 383), 60, 18, 0.002, 0.002, 0.95, 0.20, 1, 1, 1.0);
	}

	if (tick < 10) 
	{
		g_pRenderText->SetTextColor(255, 15, 15, 255);
	}
	else 
	{
		g_pRenderText->SetTextColor(225, 225, 225, 255);
	}

	if (tick < 10) 
	{
		sprintf_s(buffer, "Accept (%d)", gTradeX.CountTimeTradeX(tick));
	}
	else 
	{
		sprintf_s(buffer, "Accept");
	}

	g_pRenderText->RenderText((float)(this->m_PosX + 100), (float)(this->m_PosY + 389), buffer, 65, 0, 3, 0);

}

void CTradeX::Update()
{
	if (!this->isVisible()) 
	{
		return;
	}

	this->ItemPointer = -1;

	this->ItemPointerTarget = -1;

	this->m_PosX = 260 + (2 * GetCenterX(640));

	ScrollBarPosXTarget = this->m_PosX + 160;

	ScrollBarPosXSource = this->m_PosX + 160;
}

void CTradeX::UpdateKeyEvent()
{
	this->TradeXHiddenWindow();
}

void CTradeX::UpdateMouseEvent()
{
	this->UpdateButtom();

	if (CheckMouseIn(ScrollBarPosXTarget, this->m_PosY + 125 + TargetScrollBarPos, 15, 30)) 
	{
		if (SEASON3B::IsPress(VK_LBUTTON))
		{
			m_EventStateTarget = EVENT_SCROLL_BTN_CLIKED;

			MouseLButton = false;

			MouseLButtonPop = false;

			MouseLButtonPush = false;
		}

		m_EventStateTarget = EVENT_SCROLL_BTN_DOWN;
	}
	else 
	{
		m_EventStateTarget = EVENT_NONE;
	}

	if (CheckMouseIn(ScrollBarPosXSource, this->m_PosY + 285 + SourceScrollBarPos, 15, 30)) 
	{
		m_EventStateSource = EVENT_SCROLL_BTN_DOWN;
	}
	else 
	{
		m_EventStateSource = EVENT_NONE;
	}

	if (CheckMouseIn(this->m_PosX + 120, this->m_PosY + 90, 170, 120)) 
	{
		if (SEASON3B::IsRepeat(VK_LBUTTON)) 
		{
			int iMoveValue = MouseY - m_iMouseYTarget;

			if (iMoveValue < 0) 
			{
				if (MouseY <= (this->m_PosY + 125 + TargetScrollBarPos) + (20 / 2)) 
				{
					this->ScrollLinesTarget(0);
				}
			}
			else if (iMoveValue > 0) 
			{
				if (MouseY >= (this->m_PosY + 125 + TargetScrollBarPos) + (20 / 2))
				{
					this->ScrollLinesTarget(1);
				}
			}

			m_iMouseYTarget = MouseY;
		}
	}

	if (CheckMouseIn(this->m_PosX + 120, this->m_PosY + 260, 170, 120)) 
	{
		if (SEASON3B::IsRepeat(VK_LBUTTON)) 
		{
			int iMoveValue = MouseY - m_iMouseYSource;

			if (iMoveValue < 0) 
			{
				if (MouseY <= (this->m_PosY + 285 + SourceScrollBarPos) + (20 / 2)) 
				{
					this->ScrollLinesSource(0);
				}
			}
			else if (iMoveValue > 0) 
			{
				if (MouseY >= (this->m_PosY + 285 + SourceScrollBarPos) + (20 / 2)) 
				{
					this->ScrollLinesSource(1);
				}
			}

			m_iMouseYSource = MouseY;
		}
	}
}

void CTradeX::UpdateButtom()
{
	if (SEASON3B::IsPress(VK_RBUTTON))
	{
		if (this->MoveItemToInventory()) 
		{
			MouseRButton = false;
			MouseRButtonPop = false;
			MouseRButtonPush = false;
			return;
		}

	}
	if (CheckMouseIn(this->m_PosX + 20, this->m_PosY + 380, 65, 30))
	{ //Cancel
		if (SEASON3B::IsPress(VK_LBUTTON) && MouseLButton)
		{
			MouseLButton = false;

			MouseLButtonPop = false;

			MouseLButtonPush = false;

			SendRequestTradeXResult(0, 1, 25);
		}
	}

	if (CheckMouseIn(this->m_PosX + 100, this->m_PosY + 380, 65, 30)) 
	{ //aceitar
		if (SEASON3B::IsPress(VK_LBUTTON) && MouseLButton)
		{
			MouseLButton = false;

			MouseLButtonPop = false;

			MouseLButtonPush = false;

			if ((DWORD)((GetTickCount() - this->TickBlockOk) / 1000) > 9) 
			{
				if (this->TradeXTransation == 0) 
				{
					this->m_SourceAcceptTradeX ^= 1;

					SendRequestTradeXResult(this->m_SourceAcceptTradeX, 0, 0);
				}
			}
		}
	}
}

void CTradeX::RenderButtomHidden()
{
	if (!this->isVisible()) 
	{
		return;
	}

	if (CheckMouseIn(g_pMyInventory->GetPos().x + 13, g_pMyInventory->GetPos().y + 390, 36, 29))
	{
		g_pRenderText->SetFont(g_hFontBold);

		g_pRenderText->SetTextColor(225, 225, 225, 255);

		g_pRenderText->SetBgColor(0, 0, 0, 255);

		g_pRenderText->RenderText(g_pMyInventory->GetPos().x, g_pMyInventory->GetPos().y + 379, "Hide extended trade", 100, 0, 3, 0);
	}

	if (CheckMouseIn(g_pMyInventory->GetPos().x + 13, g_pMyInventory->GetPos().y + 390, 36, 29) && MouseLButton)
	{
		RenderBitmap(31364, (float)(g_pMyInventory->GetPos().x + 13), (float)(g_pMyInventory->GetPos().y + 390), 36, 29, 0, 0.45, 0.56, 0.40, 1, 1, 1.0);

		gTradeX.TradeXHiddenWindow();

		MouseLButton = false;

		MouseLButtonPop = false;

		MouseLButtonPush = false;
	}
	else 
	{
		RenderBitmap(31364, (float)(g_pMyInventory->GetPos().x + 13), (float)(g_pMyInventory->GetPos().y + 390), 36, 29, 0, 0.0, 0.56, 0.40, 1, 1, 1.0);
	}
}

void CTradeX::RenderTextSource()
{
	char buffer[256];

	g_pRenderText->SetFont(g_hFont);

	g_pRenderText->SetTextColor(225, 225, 225, 255);

	if (this->m_SourceAcceptTradeX) 
	{
		sprintf_s(buffer, "You accepted the negotiation");
	}
	else 
	{
		sprintf_s(buffer, "You did not accept the negotiation");
	}

	g_pRenderText->RenderText(this->m_PosX + 31, this->m_PosY + 260, buffer, 190, 0, 1, 0);

	g_pRenderText->SetFont(g_hFontBold);

	g_pRenderText->SetTextColor(240, 240, 240, 255);

	g_pRenderText->RenderText(this->m_PosX + 20, this->m_PosY + 277, "Your Items", 190, 0, 1, 0);

	sprintf_s(buffer, "(%d items)", this->m_TradeItemSource.size());

	g_pRenderText->RenderText(this->m_PosX + 156, this->m_PosY + 277, buffer, 190, 0, 7, 0);
}

void CTradeX::RenderTextTarget()
{
	char buffer[256];

	g_pRenderText->SetFont(g_hFont);

	g_pRenderText->SetTextColor(225, 225, 225, 255);
	
	if (this->m_TargetAcceptTradeX) 
	{
		sprintf_s(buffer, "%s accepted the negotiation", NameTarget);
	}
	else 
	{
		sprintf_s(buffer, "%s did not accept the negotiation", NameTarget);
	}

	g_pRenderText->RenderText(this->m_PosX + 31, this->m_PosY + 89, buffer, 190, 0, 1, 0);

	g_pRenderText->SetFont(g_hFontBold);

	g_pRenderText->SetTextColor(240, 240, 240, 255);

	sprintf_s(buffer, "Items from %s", NameTarget);

	g_pRenderText->RenderText(this->m_PosX + 20, this->m_PosY + 107, buffer, 190, 0, 1, 0);

	sprintf_s(buffer, "(%d items)", this->m_TradeItemTarget.size());

	g_pRenderText->RenderText(this->m_PosX + 156, this->m_PosY + 107, buffer, 190, 0, 7, 0);
}

void CTradeX::RenderBoxItem(float x, float y, float width, float height)
{
	SEASON3B::RenderImageScale(941007, x, y, 145.f, 90.f, 0.f, 0.f, 330.f, 212.f);

	EnableAlphaTest(1);

	glColor4f(0.2f, 0.2f, 0.2f, 0.6f);

	RenderColor(x+1, y+2, width-17, 10.0, 0.0, 0);

	EndRenderColor();

	DisableAlphaBlend();

	EnableAlphaTest(1);
}

void CTradeX::RenderItensSource()
{
	float SizeY = 290;

	char buffer[256];

	g_pRenderText->SetFont(g_hFont);

	int LineStart = 0;
	int LineCount = 0;

	for (auto &itemSource : this->m_TradeItemSource) 
	{
		if (LineStart >= SourceScrollBarOffset && LineCount < 7) 
		{
			if (!itemSource.pItemObj) 
			{
				continue;
			}

			if (MouseX >= this->m_PosX + 20 && MouseX <= this->m_PosX + 130) 
			{
				if (MouseY >= this->m_PosY + SizeY && MouseY <= this->m_PosY + (SizeY + 8))
				{
					this->ItemPointer = itemSource.slot;
					RenderItemInformation(this->m_PosX - 100, this->m_PosY + 125, itemSource.pItemObj);
				}
			}

			auto itemAttr = &ItemAttribute[itemSource.pItemObj->Type];

			if (!itemAttr) 
			{
				continue;
			}

			g_pRenderText->SetBgColor(0);

			g_pRenderText->SetFont(g_hFont);

			g_pRenderText->SetTextColor(225, 225, 225, 255);

			sprintf_s(buffer, "[%d] - %s + %d", itemSource.slot + 1, itemAttr->Name, (itemSource.pItemObj->Level >> 3) & 0xF);

			g_pRenderText->RenderText(this->m_PosX + 20, this->m_PosY + SizeY, buffer, 190, 0, 1, 0);

			SizeY += 10;

			LineCount++;
		}

		LineStart++;
	}
}

void CTradeX::RenderItensTarget()
{
	float SizeY = 120;

	char buffer[256];

	g_pRenderText->SetFont(g_hFont);

	int LineStart = 0;
	int LineCount = 0;

	for (auto &itemTarget : this->m_TradeItemTarget) 
	{
		if (LineStart >= TargetScrollBarOffset && LineCount < 7) 
		{
			if (!itemTarget.pItemObj) 
			{
				continue;
			}

			if (MouseX >= this->m_PosX + 20 && MouseX <= this->m_PosX + 130) 
			{
				if (MouseY >= this->m_PosY + SizeY && MouseY <= this->m_PosY + (SizeY + 8))
				{
					this->ItemPointerTarget = itemTarget.slot;

					RenderItemInformation(this->m_PosX - 100, this->m_PosY + 125, itemTarget.pItemObj);
				}
			}

			auto itemAttr = &ItemAttribute[itemTarget.pItemObj->Type];

			if (!itemAttr) 
			{
				continue;
			}

			g_pRenderText->SetBgColor(0);

			g_pRenderText->SetTextColor(225, 225, 225, 255);

			g_pRenderText->SetFont(g_hFont);

			sprintf_s(buffer, "[%d] - %s + %d", itemTarget.slot + 1, itemAttr->Name, (itemTarget.pItemObj->Level >> 3) & 0xF);

			g_pRenderText->RenderText(this->m_PosX + 20, this->m_PosY + SizeY, buffer, 190, 0, 1, 0);

			SizeY += 10;

			LineCount++;
		}

		LineStart++;
	}
}

void CTradeX::RenderItemInformation(float x, float y, ITEM * pItemObj)
{
	EnableAlphaTest(true);

	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

	RenderColor(45, 40, 215.0f, 370.0f, 0.0, 0);

	EndRenderColor();

	DisableAlphaBlend();

	EnableAlphaTest(true);

	RenderItemInfo((int)155, (int)130, pItemObj, 0, 0, 0);

	EndRenderColor();

	DisableAlphaBlend();

	this->RenderItemImage(pItemObj);

	EnableAlphaTest(true);
}

void CTradeX::RenderItemImage(ITEM * pItemObj)
{
	BeginModelPreviewViewport(0, 0, WindowWidth, WindowHeight, 1.f, RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);

	auto lpAttr = &ItemAttribute[pItemObj->Type];

	float width = lpAttr->Width;
	float height = lpAttr->Height;

	RenderItem3D((float)155, (float)100, (float)width * 3, (float)height * 4, (int)pItemObj->Type, (int)pItemObj->Level * 8, (int)pItemObj->Option1, (int)pItemObj->ExtOption, 0);

	EndModelPreviewViewport();

	glColor3f(1, 1, 1);
	EnableAlphaTest(false);
}

void CTradeX::TargetInserItens(int slot, BYTE* ItemInfo)
{
	TradeItem info;

	info.pItemObj = g_pNewItemMng->CreateItem(ItemInfo);

	info.slot = slot;

	this->m_TradeItemTarget.push_back(info);

	this->CalcLinesRenderTarget();
}

void CTradeX::SourceInserItens(int slot, BYTE* ItemInfo)
{
	TradeItem info;

	info.pItemObj = g_pNewItemMng->CreateItem(ItemInfo);

	info.slot = slot;

	this->m_TradeItemSource.push_back(info);

	this->CalcLinesRenderSource();
}

void CTradeX::DelItemTarget(BYTE slot)
{
	std::vector<TradeItem>::iterator li = this->m_TradeItemTarget.begin();

	for (; li != this->m_TradeItemTarget.end(); li++) 
	{
		auto pItem = &(*li);

		if (pItem->slot == slot) 
		{
			pItem->Clear();
			this->m_TradeItemTarget.erase(li);
			break;
		}
	}
}

void CTradeX::DelItemSource(BYTE slot)
{
	std::vector<TradeItem>::iterator li = this->m_TradeItemSource.begin();

	for (; li != this->m_TradeItemSource.end(); li++) 
	{
		auto pItem = &(*li);

		if (pItem->slot == slot) 
		{
			pItem->Clear();
			this->m_TradeItemSource.erase(li);
			break;
		}
	}
}

void CTradeX::CalcLinesRenderTarget()
{
	TargetScrollBarOffset = 0;
	TargetScrollBarPos = 0;
	TargetScrollBarMaxOffset = this->m_TradeItemTarget.size() - 8;
	TargetScrollBarMultiplier = (float)((float)50 / (float)(this->m_TradeItemTarget.size() - 8));
}

void CTradeX::CalcLinesRenderSource()
{
	SourceScrollBarOffset = 0;
	SourceScrollBarPos = 0;
	SourceScrollBarMaxOffset = this->m_TradeItemSource.size() - 8;
	SourceScrollBarMultiplier = (float)((float)50 / (float)(this->m_TradeItemSource.size() - 8));
}

void CTradeX::RenderScrollTarget(float x, float y, float width, float height)
{
	//Bar
	EnableAlphaTest(1);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);


	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	SEASON3B::RenderImageScale(941008, x + width - 14.2, 113, 15, 80.5f, 0.f, 0.f, 30.f, 375.f);

	int Line = 0;

	if (m_EventStateTarget == EVENT_SCROLL_BTN_DOWN)
	{
		glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
	}
	else
	{
		glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
	}

	SEASON3B::RenderImageScale(941009, ScrollBarPosXTarget + 2, y + 8 + TargetScrollBarPos, 13, 30, 0.f, 0.f, 30.f, 60.f);

	DisableAlphaBlend();
}

void CTradeX::RenderScrollSource(float x, float y, float width, float height)
{
	//Bar
	EnableAlphaTest(1);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);


	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	SEASON3B::RenderImageScale(941008, x + width - 14.2, y + 8, 15, 80.5f, 0.f, 0.f, 30.f, 375.f);

	int Line = 0;

	if (m_EventStateTarget == EVENT_SCROLL_BTN_DOWN)
	{
		glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
	}
	else
	{
		glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
	}

	SEASON3B::RenderImageScale(941009, ScrollBarPosXSource + 2, y + 8 + SourceScrollBarPos, 13, 30, 0.f, 0.f, 30.f, 60.f);

	DisableAlphaBlend();
}

void CTradeX::ScrollLinesTarget(int mode)
{
	if (mode == 0) 
	{ //Up
		if (TargetScrollBarOffset > 0) 
		{
			TargetScrollBarOffset = TargetScrollBarOffset - 1;
			TargetScrollBarPos = TargetScrollBarPos - TargetScrollBarMultiplier;
		}
	}
	else if (mode == 1) 
	{ //Down
		if (TargetScrollBarOffset < TargetScrollBarMaxOffset) 
		{
			TargetScrollBarOffset = TargetScrollBarOffset + 1;
			TargetScrollBarPos = TargetScrollBarPos + TargetScrollBarMultiplier;
		}
	}
}

void CTradeX::ScrollLinesSource(int mode)
{
	if (mode == 0) 
	{ //Up
		if (SourceScrollBarOffset > 0) 
		{
			SourceScrollBarOffset = SourceScrollBarOffset - 1;
			SourceScrollBarPos = SourceScrollBarPos - SourceScrollBarMultiplier;
		}
	}
	else if (mode == 1) 
	{ //Down
		if (SourceScrollBarOffset < SourceScrollBarMaxOffset) 
		{
			SourceScrollBarOffset = SourceScrollBarOffset + 1;
			SourceScrollBarPos = SourceScrollBarPos + SourceScrollBarMultiplier;
		}
	}
}

void CTradeX::MouseWheel(MOUSEHOOKSTRUCTEX * lpMouse)
{
	//Target
	if (CheckMouseIn(this->m_PosX + 15, this->m_PosY + 105, 160, 90)) 
	{
		if (((int)lpMouse->mouseData) > 0) 
		{
			this->ScrollLinesTarget(0);
		}

		if (((int)lpMouse->mouseData) < 0) 
		{
			this->ScrollLinesTarget(1);
		}
		return;
	}

	//Source
	if (CheckMouseIn(this->m_PosX + 15, this->m_PosY + 275, 160, 90)) 
	{
		if (((int)lpMouse->mouseData) > 0) 
		{
			this->ScrollLinesSource(0);
		}

		if (((int)lpMouse->mouseData) < 0) 
		{
			this->ScrollLinesSource(1);
		}
		return;
	}
}

void CTradeX::ClearTradeXSource()
{
	std::vector<TradeItem>::iterator li = this->m_TradeItemSource.begin();

	for (; li != m_TradeItemSource.end(); li++) 
	{
		auto SourceItem = &(*li);

		if (SourceItem->pItemObj) 
		{
			g_pNewItemMng->DeleteItem(SourceItem->pItemObj);
			SourceItem->pItemObj = nullptr;
		}
	}

	m_TradeItemSource.clear();

	this->CalcLinesRenderSource();
}

void CTradeX::ClearTradeXTarget()
{
	std::vector<TradeItem>::iterator li = this->m_TradeItemTarget.begin();

	for (; li != m_TradeItemTarget.end(); li++) 
	{
		auto TargetItem = &(*li);

		if (TargetItem->pItemObj) 
		{
			g_pNewItemMng->DeleteItem(TargetItem->pItemObj);
			TargetItem->pItemObj = nullptr;
		}
	}

	m_TradeItemTarget.clear();

	this->CalcLinesRenderTarget();
}

int CTradeX::CountTimeTradeX(DWORD tick)
{
	return TimeTradeX - tick;
}

void CTradeX::TradeXHiddenWindow()
{
	PMSG_TRADEX_HIDDEN_WINDOW_RECV pMsg;

	pMsg.header.set(0xF3, 0xC1, sizeof(pMsg));

	gProtocol.DataSend((BYTE*)&pMsg, pMsg.header.size);
}

void CTradeX::RenderIconTradeExtended()
{
	if (this->TradeXHidden) 
	{
		RenderBitmap(TradeXIconID, 90, 30, 22.0, 20.f, 0.0, 0.0, 101.f / 128.f, 101.f / 128.f, 1, 1, 0.0);

		if (CheckMouseIn(90, 30, 22, 20)) 
		{
			EnableAlphaTest();
			g_pRenderText->SetFont(g_hFont);
			g_pRenderText->SetTextColor(225, 225, 225, 255);
			g_pRenderText->SetBgColor(0, 0, 0, 255);
			g_pRenderText->RenderText(75, 50, "Extended trade system", 100, 0, RT3_SORT_CENTER);
			DisableAlphaBlend();
		}
	}
}

void CTradeX::UpdateMouseEventTradeExtended()
{
	if (TradeXHidden) 
	{
		if (CheckMouseIn(90, 30, 22, 20) && MouseLButton)
		{
			MouseLButton = false;
			MouseLButtonPop = false;
			MouseLButtonPush = false;

			this->TradeXOpenWindow();

			this->ClickEvent = GetTickCount();
		}
	}
}

void CTradeX::TradeXOpenWindow()
{
	PMSG_TRADEX_OPEN_WINDOW_SEND pMsg;

	pMsg.header.set(0xF3, 0xC0, sizeof(pMsg));

	gProtocol.DataSend((BYTE*)&pMsg, pMsg.header.size);
}

void CTradeX::GCTradeResponseRecv(PMSG_TRADE_RESPONSE_RECV * lpMsg)
{
	if (lpMsg->TradeX) 
	{
		std::memcpy(this->NameTarget, lpMsg->name, sizeof(this->NameTarget));

		this->ClearTradeXSource();
		this->ClearTradeXTarget();

		this->Open();

		this->m_TargetAcceptTradeX = 0;
		this->m_SourceAcceptTradeX = 0;
	}
	else 
	{
		this->m_isVisible = 0;
	}
}

void CTradeX::TradeXItemAddRecv(PMSG_TRADEX_ITEM_ADD_RECV * lpMsg)
{
	this->TargetInserItens(lpMsg->slot, lpMsg->ItemInfo);
	this->CalcLinesRenderTarget();
}

void CTradeX::TradeXStateRecv(PMSG_TRADEX_STATE_RECV * lpMsg)
{
	if (lpMsg->state == 2) 
	{
		this->TradeXHidden = 0;
		this->m_isVisible = 0;
		this->TradeXTransation = lpMsg->Transation;

		std::memset(this->NameTarget, 0, sizeof(this->NameTarget));

		this->ClearTradeXSource();
		this->CalcLinesRenderSource();
		this->ClearTradeXTarget();
		this->CalcLinesRenderTarget();

		g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY);
	}
	else if (lpMsg->state == 1) 
	{
		this->TradeXHidden = 1;
		this->TradeXTransation = lpMsg->Transation;
		this->m_isVisible = 0;

		g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY);
	}
	else if (lpMsg->state == 3) 
	{
		this->ClearTradeXSource();
		this->CalcLinesRenderSource();
		this->ClearTradeXTarget();
		this->CalcLinesRenderTarget();

		TradeXTransation = 1;
	}
	else if (lpMsg->state == 4) 
	{
		this->ClearTradeXTarget();
		this->CalcLinesRenderTarget();

		TradeXTransation = 2;
	}
	else if (lpMsg->state == 5) 
	{
		this->TradeXTransation = 0;
		this->m_TargetAcceptTradeX = 0;
		this->TimeTradeX = 10;
	}
}

void CTradeX::TradeXListRecv(PMSG_TRADEX_ITEM_LIST_RECV * lpMsg)
{
	if (lpMsg->count <= 0) 
	{
		return;
	}

	for (int n = 0; n < lpMsg->count; n++) 
	{
		auto lpInfo = (PMSG_TRADEX_ITEM_RECV*)(((BYTE*)lpMsg) + sizeof(PMSG_TRADEX_ITEM_LIST_RECV) + (sizeof(PMSG_TRADEX_ITEM_RECV)*n));

		if (lpMsg->target == 1) 
		{
			this->TargetInserItens(lpInfo->slot, lpInfo->ItemInfo);
		}
		else 
		{
			this->SourceInserItens(lpInfo->slot, lpInfo->ItemInfo);
		}
	}

	if (lpMsg->target == 1) 
	{
		this->CalcLinesRenderTarget();
	}
	else 
	{
		this->CalcLinesRenderSource();
	}
}

void CTradeX::TradeXDelItemTarget(PMSG_TRADEX_ITEM_DEL_RECV * lpMsg)
{
	this->DelItemTarget(lpMsg->slot);

	this->CalcLinesRenderTarget();
}

void CTradeX::TradeXOkButtonRecv(PMSG_TRADE_OK_BUTTON_RECV * lpMsg)
{
	if (lpMsg->tradeX == 1) 
	{
		if (lpMsg->flag == 0) 
		{
			this->m_TargetAcceptTradeX = 0;

			this->TimeTradeX = 10;
			this->TickBlockOk = GetTickCount();
		}
		else if (lpMsg->flag == 1) 
		{
			this->m_TargetAcceptTradeX = 1;
		}
		else if (lpMsg->flag == 2) 
		{
			this->m_SourceAcceptTradeX = 0;

			this->TimeTradeX = 10;
			this->TickBlockOk = GetTickCount();
		}
		else if (lpMsg->flag == 3) 
		{
			this->m_SourceAcceptTradeX = 1;
		}
	}
}

void CTradeX::TradeXResultRecv(PMSG_TRADE_RESULT_RECV * lpMsg)
{
	if (lpMsg->tradeX == 1) 
	{
		if (lpMsg->result == 1) 
		{
			this->ClearTradeXSource();

			this->ClearTradeXTarget();

			this->TradeXTransation = 1;
		}
	}
}

bool CTradeX::CheckTradeXSourceEmptySlot()
{
	if (this->m_TradeItemSource.size() < 120) 
	{
		return 1;
	}

	return 0;
}

void CTradeX::ProcessToReceiveTradeXItemsSource(int nIndex, BYTE* pbyItemPacket)
{
	SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();

	if (nIndex >= 0 && nIndex < 120)
	{
		if (IsItemAutoMove())
		{
			SEASON3B::CNewUIInventoryCtrl* pMyInvenCtrl = g_pMyInventory->GetInventoryCtrl();

			ITEM* pItemObj = pMyInvenCtrl->FindItemAtPt(MouseX, MouseY);

			g_pMyInventory->GetInventoryCtrl()->RemoveItem(pItemObj);

			SetItemAutoMove(false);
		}

		this->SourceInserItens(nIndex, pbyItemPacket);

		this->CalcLinesRenderSource();
	}
}

void CTradeX::ProcessToRemoveTradeXItemsSource(int nIndex, BYTE* pbyItemPacket)
{
	g_pMyInventory->InsertItem(this->m_TargetSlotBackup, pbyItemPacket);

	this->DelItemSource(nIndex);

	this->CalcLinesRenderSource();

	SetItemAutoMove(false);
}

void CTradeX::MoveItemFromInventory()
{
	if (g_pPickedItem)
	{
		if (g_pPickedItem->GetItem())
		{
			return;
		}
	}

	if (IsItemAutoMove())
	{
		return;
	}

	if (this->CheckTradeXSourceEmptySlot() == 0)
	{
		return;
	}

	SEASON3B::CNewUIInventoryCtrl* pMyInvenCtrl = g_pMyInventory->GetInventoryCtrl();

	ITEM* pItemObj = pMyInvenCtrl->FindItemAtPt(MouseX, MouseY);

	if (pItemObj)
	{
		if (::IsTradeBan(pItemObj))
		{
			g_pChatListBox->AddText("", GlobalText[494], SEASON3B::TYPE_ERROR_MESSAGE);
			return;
		}

		SetItemAutoMove(true);

		int nSrcIndex = pItemObj->y * pMyInvenCtrl->GetNumberOfColumn() + pItemObj->x;

		SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX + nSrcIndex, pItemObj, REQUEST_EQUIPMENT_TRADEX, -1);

		::PlayBuffer(SOUND_GET_ITEM01);
	}
}

bool CTradeX::MoveItemToInventory()
{
	if (!this->CheckMouse()) 
	{
		return false;
	}

	float SizeY = 290;
	int LineStart = 0;
	int LineCount = 0;
	
	for (auto &itemSource : this->m_TradeItemSource) 
	{
		if (LineStart >= SourceScrollBarOffset && LineCount < 7) 
		{
			if (!itemSource.pItemObj) 
			{
				continue;
			}
	
			if (MouseX >= this->m_PosX + 20 && MouseX <= this->m_PosX + 130) 
			{
				if (MouseY >= this->m_PosY + SizeY && MouseY <= this->m_PosY + (SizeY + 8)) 
				{
					auto itemAttr = &ItemAttribute[itemSource.pItemObj->Type];
	
					if (itemAttr) 
					{
						int nDstIndex = g_pMyInventory->FindEmptySlot(itemSource.pItemObj);
	
						if (-1 != nDstIndex)
						{
							SetItemAutoMove(true);

							m_TargetSlotBackup = nDstIndex;

							SendRequestEquipmentItem(REQUEST_EQUIPMENT_TRADEX, itemSource.slot, itemSource.pItemObj, REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX + nDstIndex);

							::PlayBuffer(SOUND_GET_ITEM01);

							return true;
						}
					}
				}
			}
	
			SizeY += 10;
			LineCount++;
		}
	
		LineStart++;
	}

	return false;
}


void CTradeX::NewDeleteItemTradeX(ITEM* item) 
{
	g_pNewItemMng->DeleteItem(item);
}

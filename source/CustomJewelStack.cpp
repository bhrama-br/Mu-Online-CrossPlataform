#include "stdafx.h"
#include "CustomJewelStack.h"
#include "LuaStack.hpp"
#include "NewUISystem.h"
#include "Widescreen.h"
#include "NewUIInventoryCtrl.h"
#include "Protocol.h"

using namespace luaaa;

CJewelStack gJewelStack;

CJewelStack::CJewelStack() : m_Lua()
{
	this->m_VisibleSplitStack = 0;

	this->m_ValueGetStack = 0;

	this->m_EventKey = 0;

	this->m_EventKey2 = 0;

	this->m_JewelList.clear();
}

void SetJewelStack(int ItemIndex) 
{
	gJewelStack.m_JewelList.push_back(ItemIndex);
}

void CJewelStack::Init()
{
	LuaModule(this->m_Lua.GetState()).fun("SetJewelStack", &SetJewelStack);

	this->m_Lua.DoFile("Data\\Configs\\Lua\\Configs\\CustomJewelStack.lua");

	this->m_Lua.Generic_Call("StartLoadJewelStack", ">");
}

bool CJewelStack::CheckIsStack(int ItemIndex)
{
	for (auto& item : this->m_JewelList) 
	{
		if (item == ItemIndex) 
		{
			return true;
		}
	}

	return false;
}

bool CJewelStack::UpdateMouseEvent()
{
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY) == 0
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE) == 1
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY) == 1
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE) == 1
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) == 1
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY) == 1
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY) == 1)
	{
		return true;
	}

	if (SEASON3B::IsPress(VK_ESCAPE))
	{
		return true;
	}

	if (SEASON3B::CheckMouseIn(g_pMyInventory->m_Pos.x, g_pMyInventory->m_Pos.y, 190, 429))
	{
		if (SEASON3B::IsRepeat(VK_SHIFT) && SEASON3B::IsPress(VK_RBUTTON))
		{
			DWORD index = g_pMyInventory->GetPointedItemIndex();

			if (index != -1)
			{
				SEASON3B::CNewUIInventoryCtrl* pMyInvenCtrl = g_pMyInventory->GetInventoryCtrl();

				auto item = pMyInvenCtrl->FindItemAtPt(MouseX, MouseY);

				if (item)
				{
					if (this->CheckIsStack(item->Type) && item->Durability > 1)
					{
						this->Open(item->Type, (item->y * pMyInvenCtrl->GetNumberOfColumn() + item->x) + 12, item->Durability);
						return false;
					}
				}
			}
		}
	}

	return true;
}

void CJewelStack::Open(int ItemIndex, int ItemSlot, int Durability)
{
	this->m_VisibleSplitStack = 1;
	this->m_EventKey = 0;
	this->m_EventKey2 = 0;
	this->m_ValueGetStack = 1;

	this->m_ItemIndex = ItemIndex;
	this->m_ItemSlot = ItemSlot;
	this->m_Durability = Durability;

	g_pNewUISystem->Show(SEASON3B::INTERFACE_UNMIXSTACK);
}

void CJewelStack::ProcessUnSplit(int ItemSlot, int Stack)
{
	if (Stack <= 0) 
	{
		return;
	}

	CUSTOM_JEWEL_STACK_INFO_SEND pMsg;

	pMsg.header.set(0x4C, 0x81, sizeof(pMsg));

	pMsg.slot = ItemSlot;

	pMsg.stack = Stack;

	gProtocol.DataSend((BYTE*)&pMsg, pMsg.header.size);
}
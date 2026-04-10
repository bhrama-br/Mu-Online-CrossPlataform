#include "stdafx.h"
#include "CounterItem.h"
#include "LuaStack.hpp"
#include "ChaosGenesis.h"
#include "NewUISystem.h"

using namespace luaaa;

CCounterItem gCounterItem;

CCounterItem::CCounterItem() : m_Lua()
{
	this->m_CounterItemInfo.clear();

	this->m_Inventory.clear();
	this->m_Warehouse.clear();
	this->m_ChaosBox.clear();
	this->m_Trade.clear();
	this->m_TradeTarget.clear();
	this->m_TargetPersonalShop.clear();
	this->m_TradeX.clear();
	this->m_TradeXTarget.clear();
	this->m_ChaosBoxGenesis.clear();
}

CCounterItem::~CCounterItem()
{
	this->m_CounterItemInfo.clear();

	this->m_Inventory.clear();
	this->m_Warehouse.clear();
	this->m_ChaosBox.clear();
	this->m_Trade.clear();
	this->m_TradeTarget.clear();
	this->m_TargetPersonalShop.clear();
	this->m_TradeX.clear();
	this->m_TradeXTarget.clear();
	this->m_ChaosBoxGenesis.clear();
}


void RegisterItemCount(int ItemIndex, char* TextPor, char* TextEng, char* TextSpn, int Color, int Bold) 
{
	COUNTER_ITEM_INFO info;

	info.ItemIndex = ItemIndex;

	strcpy_s(info.TextPor, TextPor);

	strcpy_s(info.TextEng, TextEng);

	strcpy_s(info.TextSpn, TextSpn);

	info.Color = Color;

	info.Bold = Bold;

	gCounterItem.m_CounterItemInfo.insert(std::make_pair(info.ItemIndex, info));
}

void CCounterItem::Init()
{
	LuaModule(this->m_Lua.GetState()).fun("RegisterItemCounter", &RegisterItemCount);

	this->m_Lua.DoFile("Data\\Configs\\Lua\\CharacterSystem\\CounterItem.lua");

	this->m_Lua.Generic_Call("StartLoadCounterItem", "");
}

void CCounterItem::RestartLua()
{
	this->m_Lua.CloseLua();

	this->m_Lua.RegisterLua();

	LuaModule(this->m_Lua.GetState()).fun("RegisterItemCounter", &RegisterItemCount);

	this->m_Lua.DoFile("Data\\Configs\\Lua\\CharacterSystem\\CounterItem.lua");

	this->m_Lua.Generic_Call("StartLoadCounterItem", "");

}

void CCounterItem::ClearById(int type)
{
	if (type == 0) {
		this->m_Inventory.clear();
	}
	else if (type == 1) {
		this->m_Warehouse.clear();
	}
	else if (type == 2) {
		this->m_Trade.clear();
	}
	else if (type == 3) {
		this->m_ChaosBox.clear();
	}
	else if (type == 4) {
		this->m_TradeTarget.clear();
	}
	else if (type == 5) {
		this->m_TargetPersonalShop.clear();
	}
	else if (type == 6) {
		this->m_TradeX.clear();
	}
	else if (type == 7) {
		this->m_TradeXTarget.clear();
	}
	else if (type == 8) {
		this->m_ChaosBoxGenesis.clear();
	}
}

COUNTER_ITEM_INFO* CCounterItem::getItemInfo(int ItemIndex)
{
	std::map<int, COUNTER_ITEM_INFO>::iterator it = this->m_CounterItemInfo.find(ItemIndex);

	if (it == this->m_CounterItemInfo.end()) {
		return 0;
	}

	return &it->second;
}

bool CCounterItem::CheckIsItem(int ItemIndex)
{
	std::map<int, COUNTER_ITEM_INFO>::iterator it = this->m_CounterItemInfo.find(ItemIndex);

	if (it == this->m_CounterItemInfo.end()) {
		return 0;
	}

	return 1;
}

void CCounterItem::InserCounterItem(WORD ItemIndex, BYTE Slot, WORD Type, DWORD kills, BYTE Top)
{
	COUNTER_ITEMS info;

	info.ItemIndex = ItemIndex;
	info.Slot = Slot;
	info.Top = Top;
	info.kills = kills;

	if (Type == 0) {
		this->m_Inventory.insert(std::make_pair(info.Slot, info));
	}
	else if (Type == 1) {
		this->m_Warehouse.insert(std::make_pair(info.Slot, info));
	}
	else if (Type == 2) {
		this->m_Trade.insert(std::make_pair(info.Slot, info));
	}
	else if (Type == 3) {
		this->m_ChaosBox.insert(std::make_pair(info.Slot, info));
	}
	else if (Type == 4) {
		this->m_TradeTarget.insert(std::make_pair(info.Slot, info));
	}
	else if (Type == 5) {
		this->m_TargetPersonalShop.insert(std::make_pair(info.Slot, info));
	}
	else if (Type == 6) {
		this->m_TradeX.insert(std::make_pair(info.Slot, info));
	}
	else if (Type == 7) {
		this->m_TradeXTarget.insert(std::make_pair(info.Slot, info));
	}
	else if (Type == 8) {
		this->m_ChaosBoxGenesis.insert(std::make_pair(info.Slot, info));
	}
}

void CCounterItem::getCount(int x, int y, int *count, int *top)
{
	int type = 0;
	int slot = -1;

	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY)) {
		bool slotFind = false;

		if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY)) {
			if (CheckMouseIn(g_pMyShopInventory->GetPos().x, g_pMyShopInventory->GetPos().y, 190, 429)) {
				slot = (y * g_pMyShopInventory->GetInventoryCtrl()->GetNumberOfColumn() + x) + 76;
				slotFind = true;
			}
		}
		else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY)) {
			if (CheckMouseIn(g_pPurchaseShopInventory->GetPos().x, g_pPurchaseShopInventory->GetPos().y, 190, 429)) {
				type = 5;
				slot = (y * g_pPurchaseShopInventory->GetInventoryCtrl()->GetNumberOfColumn() + x) + 76;
				slotFind = true;
			}
		}
		else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)) {
			if (CheckMouseIn(g_pStorageInventory->GetPos().x, g_pStorageInventory->GetPos().y, 190, 429)) {
				type = 1;
				slot = (y * 8 + x);
				slotFind = true;
			}
		}
		else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)) {
			if (CheckMouseIn(g_pMixInventory->GetPos().x, g_pMixInventory->GetPos().y, 190, 429)) {
				type = 3;
				slot = (y * 4 + x);
				slotFind = true;
			}
		}
		else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHAOSGENESIS)) {
			if (g_pChaosGenesis->ItemPointer != -1) {
				slot = g_pChaosGenesis->ItemPointer;
				type = 8;
				slotFind = true;
			}
		}
		else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE)) {
			
			if (CheckMouseIn(g_pTrade->GetPos().x, g_pTrade->GetPos().y, 190, 429)) {
				if (g_pTrade->GetPointedItemIndexMyInven() != -1) {
					type = 2;
					slot = (y * 4 + x);
				}
				else if (g_pTrade->GetPointedItemIndexYourInven() != -1) {
					type = 4;
					slot = (y * 4 + x);
				}

				slotFind = true;
			}
		}

		if (!slotFind) {
			if (CheckMouseIn(g_pMyInventory->GetPos().x, g_pMyInventory->GetPos().y, 190, 429)) {
				if (g_pMyInventory->GetPointedSlot() != -1) {
					slot = g_pMyInventory->GetPointedSlot();
				}
				else {
					slot = (y * g_pMyInventory->GetInventoryCtrl()->GetNumberOfColumn() + x) + 12;
				}
			}
		}
	}

	if (slot == -1) {
		return;
	}

	if (type == 0) {
		std::map<int, COUNTER_ITEMS>::iterator it = this->m_Inventory.find(slot);

		if (it == this->m_Inventory.end()) {
			return;
		}

		*(count) = it->second.kills;
		*(top) = it->second.Top;
		return;
	}
	else if (type == 1) {
		std::map<int, COUNTER_ITEMS>::iterator it = this->m_Warehouse.find(slot);

		if (it == this->m_Warehouse.end()) {
			return;
		}

		*(count) = it->second.kills;
		*(top) = it->second.Top;
		return;
	}
	else if (type == 2) {
		std::map<int, COUNTER_ITEMS>::iterator it = this->m_Trade.find(slot);

		if (it == this->m_Trade.end()) {
			return;
		}

		*(count) = it->second.kills;
		*(top) = it->second.Top;
		return;
	}
	else if (type == 3) {
		std::map<int, COUNTER_ITEMS>::iterator it = this->m_ChaosBox.find(slot);

		if (it == this->m_ChaosBox.end()) {
			return;
		}

		*(count) = it->second.kills;
		*(top) = it->second.Top;
		return;
	}
	else if (type == 4) {
		std::map<int, COUNTER_ITEMS>::iterator it = this->m_TradeTarget.find(slot);

		if (it == this->m_TradeTarget.end()) {
			return;
		}

		*(count) = it->second.kills;
		*(top) = it->second.Top;
		return;
	}
	else if (type == 5) {
		std::map<int, COUNTER_ITEMS>::iterator it = this->m_TargetPersonalShop.find(slot);

		if (it == this->m_TargetPersonalShop.end()) {
			return;
		}

		*(count) = it->second.kills;
		*(top) = it->second.Top;
		return;
	}
	else if (type == 6) {
		std::map<int, COUNTER_ITEMS>::iterator it = this->m_TradeX.find(slot);

		if (it == this->m_TradeX.end()) {
			return;
		}

		*(count) = it->second.kills;
		*(top) = it->second.Top;
		return;
	}
	else if (type == 7) {
		std::map<int, COUNTER_ITEMS>::iterator it = this->m_TradeXTarget.find(slot);

		if (it == this->m_TradeXTarget.end()) {
			return;
		}

		*(count) = it->second.kills;
		*(top) = it->second.Top;
		return;
	}
	else if (type == 8) {
		std::map<int, COUNTER_ITEMS>::iterator it = this->m_ChaosBoxGenesis.find(slot);

		if (it == this->m_ChaosBoxGenesis.end()) {
			return;
		}

		*(count) = it->second.kills;
		*(top) = it->second.Top;
		return;
	}

	return;
}

void CCounterItem::GCItemCounterRecv(PMSG_ITEM_COUNTER_RECV* lpMsg)
{
	this->ClearById(lpMsg->Type);

	for (int n = 0; n < lpMsg->count; n++)
	{
		auto lpInfo = (PMSG_ITEM_COUNTER_INFO*)(((BYTE*)lpMsg) + sizeof(PMSG_ITEM_COUNTER_RECV) + (sizeof(PMSG_ITEM_COUNTER_INFO) * n));

		this->InserCounterItem(lpInfo->ItemIndex, lpInfo->Slot, lpMsg->Type, lpInfo->kills, lpInfo->Top);
	}
}
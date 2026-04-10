#pragma once

#include "Lua.h"
#include "NProtocol.h"

#define MAX_CHAOS_BOX_ITEMS 32
#define MAX_INVENTORY_ITEMS 108
#define MAX_TRADE_ITEMS 32
#define MAX_TRADEX_ITEMS 120
#define MAX_WAREHOUSE_ITEMS 120
#define MAX_CHAOS_BOX_GENESIS_ITEMS 120
#define MAX_PERSONAL_SHOP_ITEMS 32

struct COUNTER_ITEM_INFO {
	int ItemIndex;
	char TextPor[128];
	char TextEng[128];
	char TextSpn[128];
	int Color;
	int Bold;
};

class COUNTER_ITEMS
{
public:
	COUNTER_ITEMS() {
		ItemIndex = -1;
		Slot = -1;
		Top = 0;
		kills = -1;
	};

	~COUNTER_ITEMS() {

	};

	WORD ItemIndex;
	BYTE Slot;
	BYTE Top;
	DWORD kills;
};

class CCounterItem {
public:
	CCounterItem();
	~CCounterItem();

	void Init();
	void RestartLua();
	void ClearById(int type);
	COUNTER_ITEM_INFO * getItemInfo(int ItemIndex);
	bool CheckIsItem(int ItemIndex);
	void InserCounterItem(WORD ItemIndex, BYTE Slot, WORD Type, DWORD kills, BYTE Top);
	void getCount(int x, int y, int *count, int *top);
	void GCItemCounterRecv(PMSG_ITEM_COUNTER_RECV* lpMsg);

public:
	Lua m_Lua;
	std::map<int, COUNTER_ITEM_INFO> m_CounterItemInfo;

private:
	std::map<int, COUNTER_ITEMS> m_Inventory;
	std::map<int, COUNTER_ITEMS> m_Warehouse;
	std::map<int, COUNTER_ITEMS> m_ChaosBox;
	std::map<int, COUNTER_ITEMS> m_Trade;
	std::map<int, COUNTER_ITEMS> m_TradeTarget;
	std::map<int, COUNTER_ITEMS> m_TargetPersonalShop;
	std::map<int, COUNTER_ITEMS> m_TradeX;
	std::map<int, COUNTER_ITEMS> m_TradeXTarget;
	std::map<int, COUNTER_ITEMS> m_ChaosBoxGenesis;
};

extern CCounterItem gCounterItem;
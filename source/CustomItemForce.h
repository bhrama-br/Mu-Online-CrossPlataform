#pragma once

#include "Lua.h"

struct CUSTOM_ITEM_FORCE_INFO 
{
	int ItemIndex;
	int DamageMin;
	int DamageMax;
	int Defense;
	int MagicDefense;
};

class CCustomItemForce 
{
public:
	CCustomItemForce();
	~CCustomItemForce();

	void Init();
	CUSTOM_ITEM_FORCE_INFO* GetItem(int ItemIndex);
	void ItemConverts(ITEM* lpItem, BYTE Option, BYTE Special, BYTE Value);

private:
	Lua m_Lua;

public:
	std::map<int, CUSTOM_ITEM_FORCE_INFO> m_CustomItemForceInfo;
};

extern CCustomItemForce gCustomItemForce;
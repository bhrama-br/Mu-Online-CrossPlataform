// ItemManager.cpp: implementation of the CItemManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemManager.h"
#include "LuaStack.hpp"
#include "ZzzObject.h"

using namespace luaaa;

CItemManager gItemManager;

CItemManager::CItemManager() : m_Lua() // OK
{
	this->m_CustomItemInfo.clear();
}

CItemManager::~CItemManager() // OK
{
	this->m_CustomItemInfo.clear();
}

void SetLoadItem(int ItemIndex, int colorR, int colorG, int ColorB, char* ModelName) 
{
	CUSTOM_ITEM_INFO info;

	info.ItemIndex = ItemIndex;

	info.ColorR = static_cast<float>(static_cast<float>(colorR) / static_cast<float>(255));

	info.ColorG = static_cast<float>(static_cast<float>(colorG) / static_cast<float>(255));

	info.ColorB = static_cast<float>(static_cast<float>(ColorB) / static_cast<float>(255));

	strcpy_s(info.ModelName, ModelName);

	gItemManager.m_CustomItemInfo.insert(std::make_pair(info.ItemIndex, info));
}

void CItemManager::Init() // OK
{
	LuaModule(this->m_Lua.GetState()).fun("LoadItem", &SetLoadItem);

	this->m_Lua.DoFile("Data//Configs//Lua//Configs//LoadItens.lua");

	m_Lua.Generic_Call("StartLoadItens", "");
}

BOOL CItemManager::GetCustomItemColor(int Type, float* ItemColor) // OK
{
	std::map<int, CUSTOM_ITEM_INFO>::iterator it = gItemManager.m_CustomItemInfo.find(Type);

	if (it == gItemManager.m_CustomItemInfo.end()) 
	{
		return 0;
	}

	auto lpInfo = &(*it).second; 

	ItemColor[0] = lpInfo->ColorR;
	ItemColor[1] = lpInfo->ColorG;
	ItemColor[2] = lpInfo->ColorB;

	return 1;
}

int CItemManager::GET_ITEM(int section, int index)
{
	return (((section) * 512) + (index));
}

int CItemManager::GET_ITEM_MODEL(int section, int index)
{
	return ((((section) * 512) + (index)) + MODEL_ITEM);
}

void CItemManager::GetItemColor(int Type, float Alpha, float Bright, vec3_t Light, bool ExtraMon)
{
	if (this->GetCustomItemColor(Type, (float*)Light) == 0)
	{
		PartObjectColor(Type, Alpha, Bright, Light, ExtraMon);
	}
}
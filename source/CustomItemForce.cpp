#include "stdafx.h"
#include "CustomItemForce.h"
#include "LuaStack.hpp"
#include "CustomWing.h"

using namespace luaaa;

CCustomItemForce gCustomItemForce;

CCustomItemForce::CCustomItemForce() : m_Lua()
{
	this->m_CustomItemForceInfo.clear();
}

CCustomItemForce::~CCustomItemForce()
{
	this->m_CustomItemForceInfo.clear();
}

void SetItemForce(int ItemIndex, int DamageMin, int DamageMax, int Defense, int MagicDefense) 
{
	CUSTOM_ITEM_FORCE_INFO info;

	info.ItemIndex = ItemIndex;

	info.DamageMin = DamageMin;

	info.DamageMax = DamageMax;

	info.Defense = Defense;

	info.MagicDefense = MagicDefense;

	gCustomItemForce.m_CustomItemForceInfo.insert(std::make_pair(info.ItemIndex, info));
}

void CCustomItemForce::Init()
{
	LuaModule(this->m_Lua.GetState()).fun("SetItemForce", &SetItemForce);

	this->m_Lua.DoFile("Data//Configs//Lua//Configs//CustomItemForce.lua");

	this->m_Lua.Generic_Call("StartCustomItemForce", "");
}

CUSTOM_ITEM_FORCE_INFO * CCustomItemForce::GetItem(int ItemIndex)
{
	std::map<int, CUSTOM_ITEM_FORCE_INFO>::iterator it = this->m_CustomItemForceInfo.find(ItemIndex);

	if (it == this->m_CustomItemForceInfo.end()) {
		return 0;
	}

	return &it->second;
}

void CCustomItemForce::ItemConverts(ITEM * lpItem, BYTE Option, BYTE Special, BYTE Value)
{
	auto Item = gCustomItemForce.GetItem(lpItem->Type);

	if (Item) 
	{
		lpItem->DamageMin = Item->DamageMin;
		lpItem->DamageMax = Item->DamageMax;
		lpItem->Defense = Item->Defense;
		lpItem->MagicDefense = Item->MagicDefense;

		int exc = Special & 0x3F;
		bool Ancient = 0;
		int Level = (Option >> 3) & 0xF;

		auto itemAttr = &ItemAttribute[lpItem->Type];

		if (lpItem->Type >= 6147 && lpItem->Type <= 6150
			|| lpItem->Type >= 6180 && lpItem->Type <= 6184
			|| lpItem->Type >= 6186 && lpItem->Type <= 6187
			|| lpItem->Type == 19
			|| lpItem->Type == 2066
			|| lpItem->Type == 2570
			|| lpItem->Type == 1037
			|| lpItem->Type == 6686
			|| lpItem->Type >= 6274 && lpItem->Type <= 6278
			|| lpItem->Type >= 6193 && lpItem->Type <= 6194
			|| lpItem->Type == 6279
			|| lpItem->Type == 2596
			|| gCustomWing.CheckCustomWingByItem(lpItem->Type)) 
		{
			exc = 0;
		}

		if (Value % 4 == 1 || Value % 4 == 2) 
		{
			exc = 1;
			Ancient = 1;
		}

		int DropItem = itemAttr->Level + 30;
		int AddExcVal = 0;

		switch (lpItem->Type)
		{
		case 0x406:
			AddExcVal = 15;
			break;
		case 0x806:
			AddExcVal = 30;
			break;
		case 0xA07:
			AddExcVal = 25;
			break;
		}

		if (Item->DamageMin > 0) 
		{
			if (exc > 0 && itemAttr->Level)
			{
				if (AddExcVal) 
				{
					lpItem->DamageMin += AddExcVal;
				}
				else 
				{
					lpItem->DamageMin += (25 * Item->DamageMin / itemAttr->Level) + 5;
				}
			}

			if (Ancient) {
				lpItem->DamageMin += DropItem / 40 + 5;
			}

			lpItem->DamageMin += (min(9, Level) * 3);

			switch (Level)
			{
			case 15:
				lpItem->DamageMin += 9;
			case 14:
				lpItem->DamageMin += 8;
			case 13:
				lpItem->DamageMin += 7;
			case 12:
				lpItem->DamageMin += 6;
			case 11:
				lpItem->DamageMin += 5;
			case 10:
				lpItem->DamageMin += 4;
				break;
			default:
				break;
			}
		}

		if (Item->DamageMax > 0)
		{
			if (exc > 0 && itemAttr->Level) 
			{
				if (AddExcVal) 
				{
					lpItem->DamageMax += AddExcVal;
				}
				else 
				{
					lpItem->DamageMax += (25 * Item->DamageMax / itemAttr->Level) + 5;
				}
			}

			if (Ancient) 
			{
				lpItem->DamageMax += DropItem / 40 + 5;
			}

			lpItem->DamageMax += (min(9, Level) * 3);

			switch (Level)
			{
			case 15:
				lpItem->DamageMax += 9;
			case 14:
				lpItem->DamageMax += 8;
			case 13:
				lpItem->DamageMax += 7;
			case 12:
				lpItem->DamageMax += 6;
			case 11:
				lpItem->DamageMax += 5;
			case 10:
				lpItem->DamageMax += 4;
				break;
			default:
				break;
			}
		}

		if (lpItem->Type == 6686) 
		{
			lpItem->Defense = 15;
		}

		if (Item->Defense > 0)
		{
			if (lpItem->Type < 3072 || lpItem->Type >= 3584) 
			{
				if (exc > 0 && itemAttr->Level) 
				{
					lpItem->Defense += 12 * Item->Defense / itemAttr->Level + itemAttr->Level / 5 + 4;
				}

				if (Ancient) 
				{
					lpItem->Defense += 3 * lpItem->Defense / DropItem + DropItem / 30 + 2;
				}

				if ((lpItem->Type < 6147 || lpItem->Type > 6150) && lpItem->Type != 6186)
				{
					if (lpItem->Type != 6686 && lpItem->Type != 6193)
					{
						if ((lpItem->Type < 6180 || lpItem->Type > 6184) && lpItem->Type != 6187 && lpItem->Type != 6194 && !gCustomWing.CheckCustomWingByItem(lpItem->Type))
						{
							lpItem->Defense += (min(9, Level) * 3);
						}
						else {
							lpItem->Defense += (min(9, Level) * 4);
						}
					}
					else {
						lpItem->Defense += (min(9, Level) * 2);
					}
				}
				else
				{
					lpItem->Defense += (min(9, Level) * 2);
				}

				if ((lpItem->Type < 6180 || lpItem->Type > 6184) && lpItem->Type != 6187 && lpItem->Type != 6194 && !gCustomWing.CheckCustomWingByItem(lpItem->Type))
				{
					switch (Level)
					{
					case 15:
						lpItem->Defense += 9;
					case 14:
						lpItem->Defense += 8;
					case 13:
						lpItem->Defense += 7;
					case 12:
						lpItem->Defense += 6;
					case 11:
						lpItem->Defense += 5;
					case 10:
						lpItem->Defense += 4;
						break;
					default:
						break;
					}
				}
				else
				{
					switch (Level)
					{
					case 15:
						lpItem->Defense += 10;
					case 14:
						lpItem->Defense += 9;
					case 13:
						lpItem->Defense += 8;
					case 12:
						lpItem->Defense += 7;
					case 11:
						lpItem->Defense += 6;
					case 10:
						lpItem->Defense += 5;
						break;
					default:
						break;
					}
				}
			}
			else
			{
				lpItem->Defense += Level;

				if (Ancient) 
				{
					lpItem->Defense += (int)(20 * lpItem->Defense / DropItem) + 2;
				}
			}
		}

		if (Item->MagicDefense > 0)
		{
			lpItem->MagicDefense += (min(9, Level) * 3);

			switch (Level)
			{
			case 15:
				lpItem->MagicDefense += 9;
			case 14:
				lpItem->MagicDefense += 8;
			case 13:
				lpItem->MagicDefense += 7;
			case 12:
				lpItem->MagicDefense += 6;
			case 11:
				lpItem->MagicDefense += 5;
			case 10:
				lpItem->MagicDefense += 4;
				break;
			default:
				break;
			}
		}
	}
}

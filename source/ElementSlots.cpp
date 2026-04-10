#include "stdafx.h"
#include "ElementSlots.h"
#include "LuaStack.hpp"

using namespace luaaa;

cControlSlot gControlSlot;

cControlSlot::cControlSlot() : m_Lua()
{
	this->m_ElementSlots.clear();

	this->m_ElementEquiped[0].x = 56;
	this->m_ElementEquiped[0].y = 44;
	this->m_ElementEquiped[0].width = 22;
	this->m_ElementEquiped[0].height = 22;
	this->m_ElementEquiped[0].Active = 0;
	this->m_ElementEquiped[0].isPet = 0;

	this->m_ElementEquiped[1].x = 116;
	this->m_ElementEquiped[1].y = 175;
	this->m_ElementEquiped[1].width = 22;
	this->m_ElementEquiped[1].height = 22;
	this->m_ElementEquiped[1].Active = 0;
	this->m_ElementEquiped[1].isPet = 0;

	this->m_ElementEquiped[2].x = 56;
	this->m_ElementEquiped[2].y = 115;
	this->m_ElementEquiped[2].width = 22;
	this->m_ElementEquiped[2].height = 22;
	this->m_ElementEquiped[2].Active = 0;
	this->m_ElementEquiped[2].isPet = 0;

	this->m_ElementEquiped[3].x = 116;
	this->m_ElementEquiped[3].y = 115;
	this->m_ElementEquiped[3].width = 22;
	this->m_ElementEquiped[3].height = 22;
	this->m_ElementEquiped[3].Active = 0;
	this->m_ElementEquiped[3].isPet = 0;
}

cControlSlot::~cControlSlot()
{

}

void cControlSlot::Release()
{
	for (std::map<int, ELEMENT_SLOTS_INFO>::iterator it = this->m_ElementSlots.begin(); it != this->m_ElementSlots.end(); it++) {
		auto lpInfo = &(*it).second;

		lpInfo->Clear();
	}

	this->m_ElementSlots.clear();
}


void SetElementSlot(int stack, int x, int y, int width, int height, int active, int pet)
{
	gControlSlot.m_ElementEquiped[stack].x = x;
	gControlSlot.m_ElementEquiped[stack].y = y;
	gControlSlot.m_ElementEquiped[stack].width = width;
	gControlSlot.m_ElementEquiped[stack].height = height;
	gControlSlot.m_ElementEquiped[stack].Active = active;
	gControlSlot.m_ElementEquiped[stack].isPet = pet;
}

void cControlSlot::Init()
{
	luaaa::LuaModule(this->m_Lua.GetState()).fun("SetElementSlot", &SetElementSlot);

	this->m_Lua.DoFile("Data//Configs//Lua//Configs//ElementSlots.lua");

	this->m_Lua.Generic_Call("StartLoadElementSlots", ">");
}


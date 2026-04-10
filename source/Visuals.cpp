#include "stdafx.h"
#include "Visuals.h"
#include "LuaStack.hpp"
#include "ZzzInventory.h"

using namespace luaaa;

cVisuals Visuals;

void CreateVisualDescription(int Line, char* text, int color, int bold) 
{
	wsprintf(TextList[Line], text);
	TextListColor[Line] = color;
	TextBold[Line] = bold;
}

void setVisualInventory(int switchVisual, int originalDescription) 
{
	Visuals.VisualSwitch = switchVisual;

	Visuals.VisualOriginalDescription = originalDescription;
}

void cVisuals::Init()
{
	this->m_LuaGlobal.RegisterGlobal(this->m_Lua.GetState());

	LuaModule(this->m_Lua.GetState()).fun("createDescription", &CreateVisualDescription);

	LuaModule(this->m_Lua.GetState()).fun("setVisualInventory", &setVisualInventory);

	this->m_Lua.DoFile("Data//Configs//Lua//CharacterSystem//DescriptionsItemsVisual.lua");
}

void cVisuals::SetDescriptionsItem(int Line, ITEM* itemObject)
{
	this->DescriptionsCall(Line, itemObject);
}

int cVisuals::DescriptionsCall(int Line, ITEM* itemObject)
{
	if (itemObject == 0)
	{
		return Line;
	}

	for (int i = 0; i < 50; ++i) 
	{
		memset(TextList[i], 0, 100);
		TextListColor[i] = 0;
		TextBold[i] = 0;
	}

	Visuals.m_Lua.Generic_Call("RenderDescriptionVisualItem", "i>", itemObject->Type);
	return Line;
}
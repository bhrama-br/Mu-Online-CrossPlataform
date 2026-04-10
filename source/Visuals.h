#pragma once

#include "Lua.h"
#include "LuaGlobal.h"

class cVisuals
{
public:
	void Init();
	void SetDescriptionsItem(int Line, ITEM* itemObject);
	int DescriptionsCall(int Line, ITEM* itemObject);
public:
	int VisualSwitch;
	int VisualOriginalDescription;
public:
	Lua m_Lua;
	LuaGlobal m_LuaGlobal;
};

extern cVisuals Visuals;
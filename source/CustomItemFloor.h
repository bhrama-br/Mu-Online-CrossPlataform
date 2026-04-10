#pragma once

#include "Lua.h"

struct CUSTOM_ITEM_FLOOR_INFO 
{
	int ItemIndex;
	float AngleX;
	float AngleY;
	float AngleZ;
	float Size;
};

class CCustomItemFloor 
{
public:
	CCustomItemFloor();
	~CCustomItemFloor();

	void Init();
	CUSTOM_ITEM_FLOOR_INFO* getItem(int ItemIndex);
	void AngleItem(OBJECT* object);

private:
	Lua m_Lua;

public:
	std::map<int, CUSTOM_ITEM_FLOOR_INFO> m_CustomItemFloorInfo;
};

extern CCustomItemFloor gCustomItemFloor;
#pragma once

#include "Lua.h"
#include "_struct.h"

class ELEMENT_SLOTS_INFO 
{
public:
	ELEMENT_SLOTS_INFO() 
	{
		this->slot = -1;
	};

	~ELEMENT_SLOTS_INFO() 
	{
	};

	void Clear() 
	{
		this->slot = -1;
	}

	bool ItemEquiped;
	BYTE slot;
	ITEM pItemObj;
};

struct ITEM_ELEMENT_EQUIPED
{
	int x;
	int y;
	int width;
	int height;
	int Active;
	int isPet;
};

class cControlSlot
{

public:
	cControlSlot();
	~cControlSlot();
	void Release();
	void Init();

public:
	std::map<int, ELEMENT_SLOTS_INFO> m_ElementSlots;
	ITEM_ELEMENT_EQUIPED m_ElementEquiped[4];

private:
	Lua m_Lua;
};

extern cControlSlot gControlSlot;
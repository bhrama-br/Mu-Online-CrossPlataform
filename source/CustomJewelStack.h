#pragma once
#include "Lua.h"

class CJewelStack
{
public:
	CJewelStack();
	~CJewelStack() = default;

	void Init();
	bool CheckIsStack(int ItemIndex);
	bool UpdateMouseEvent();
	void Open(int ItemIndex, int ItemSlot, int Durability);
	void ProcessUnSplit(int ItemSlot, int Stack);

	std::vector<int> m_JewelList;

public:
	Lua m_Lua;
	int m_ItemIndex;
	int m_ItemSlot;
	int m_Durability;
	int m_VisibleSplitStack;
	int m_ValueGetStack;
	int m_EventKey;
	int m_EventKey2;
	POINT m_Pos;

};

extern CJewelStack gJewelStack;
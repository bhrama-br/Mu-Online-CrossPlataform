#include "stdafx.h"
#include "CustomBow.h"
#include "LuaStack.hpp"

using namespace luaaa;

CCustomBow gCustomBow;

CCustomBow::CCustomBow() : m_Lua()
{
	this->m_CustomBowInfo.clear();
}

CCustomBow::~CCustomBow()
{
	this->m_CustomBowInfo.clear();
}

void SetBowCross(int ItemIndex, int Type, int Skill) 
{
	CUSTOM_BOW_INFO info;

	info.ItemIndex = ItemIndex;

	info.Type = Type;

	info.Skill = Skill;

	gCustomBow.m_CustomBowInfo.insert(std::make_pair(info.ItemIndex, info));
}

void CCustomBow::Init()
{
	LuaModule(this->m_Lua.GetState()).fun("SetBowCross", &SetBowCross);

	this->m_Lua.DoFile("Data//Configs//Lua//Configs//CustomBowCross.lua");

	this->m_Lua.Generic_Call("StartLoadBowCross", "");
}

bool CCustomBow::CheckIsBow(int ItemIndex)
{
	std::map<int, CUSTOM_BOW_INFO>::iterator it = this->m_CustomBowInfo.find(ItemIndex);

	if (it == this->m_CustomBowInfo.end()) 
	{
		return 0;
	}

	return (it->second.Type == 1 ? 1 : 0);
}

bool CCustomBow::CheckIsCrossBow(int ItemIndex)
{
	std::map<int, CUSTOM_BOW_INFO>::iterator it = this->m_CustomBowInfo.find(ItemIndex);

	if (it == this->m_CustomBowInfo.end()) 
	{
		return 0;
	}

	return (it->second.Type == 2 ? 1 : 0);
}

int CCustomBow::GetSkillNumber(int ItemIndex)
{
	std::map<int, CUSTOM_BOW_INFO>::iterator it = this->m_CustomBowInfo.find(ItemIndex);

	if (it == this->m_CustomBowInfo.end()) 
	{
		return 0;
	}

	return it->second.Skill;
}

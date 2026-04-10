#pragma once

#include "Lua.h"

struct CUSTOM_BOW_INFO 
{
	int ItemIndex;
	int Type;
	int Skill;
};

class CCustomBow 
{
public:
	CCustomBow();
	~CCustomBow();

	void Init();
	bool CheckIsBow(int ItemIndex);
	bool CheckIsCrossBow(int ItemIndex);
	int GetSkillNumber(int ItemIndex);

private:
	Lua m_Lua;

public:
	std::map<int, CUSTOM_BOW_INFO> m_CustomBowInfo;
};

extern CCustomBow gCustomBow;
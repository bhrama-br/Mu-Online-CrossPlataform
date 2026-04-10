#pragma once

#include "Lua.h"
#include "LuaBMD.h"
#include "LuaEffects.h"
#include "DarkSpiritView.h"

struct DARK_SPIRIT_INFO 
{
	int ItemIndex;
	int ItemModel;
	char ModelName[32];
	char ObjectName[32];
};

class CDarkSpirit
{
public:
	CDarkSpirit();
	~CDarkSpirit();

	void Init();
	void CreateDarkSpirit(CHARACTER* m_Owner, int ModelIndex);
	bool CheckExistDarkSpirit(CHARACTER*Owner);
	int GetItemDarkSpirit(CHARACTER*Owner);
	void Register(BoostSmart_Ptr(DARK_SPIRIT_HELPER_VIEW) info);
	void DeleteDarkSpirit(CHARACTER*Owner, int itemType = -1, bool allDelete = false);
	void UnRegister(CHARACTER*Owner, int itemType, bool isUnregistAll = false);
	DARK_SPIRIT_INFO* getDarkSpirit(int ItemIndex);
	bool checkIsDarkSpirit(int ItemIndex);
	int GetItemModelDarkSpirit(int ItemIndex);
	void RenderModel(DWORD BMDStruct, DWORD ObjectStruct, DWORD ItemIndex);
	int CheckNewRaven(CHARACTER* Struct);
	int CheckCreateDarkSpiritNow(int Index);

public:
	Lua m_Lua;
	LuaBMD m_LuaBMD;
	CLuaEffects m_LuaEffects;

public:
	std::map<int, int> m_DarkSpiritModels;
	std::map<int, DARK_SPIRIT_INFO> m_DarkSpiritInfo;
	std::vector<BoostSmart_Ptr(DARK_SPIRIT_HELPER_VIEW)> m_DarkSpirit;
};

extern CDarkSpirit gDarkSpirit;
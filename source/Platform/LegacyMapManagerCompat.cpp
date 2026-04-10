#include "stdafx.h"

#if defined(__ANDROID__)

#include "MapManager.h"

namespace
{
	bool IsMapInRange(int value, int begin, int end)
	{
		return value >= begin && value <= end;
	}
}

CMapManager gMapManager;

CMapManager::CMapManager()
	: WorldActive(-1)
{
}

CMapManager::~CMapManager()
{
}

void CMapManager::Load()
{
}

void CMapManager::LoadWorld(int map)
{
	WorldActive = map;
}

void CMapManager::DeleteObjects()
{
}

bool CMapManager::InChaosCastle(int iMap)
{
	const int map = (iMap >= 0) ? iMap : WorldActive;
	return IsMapInRange(map, WD_18CHAOS_CASTLE, WD_18CHAOS_CASTLE_END) || map == WD_53CAOSCASTLE_MASTER_LEVEL;
}

bool CMapManager::InBloodCastle(int iMap)
{
	const int map = (iMap >= 0) ? iMap : WorldActive;
	return IsMapInRange(map, WD_11BLOODCASTLE1, WD_11BLOODCASTLE_END) || map == WD_52BLOODCASTLE_MASTER_LEVEL;
}

bool CMapManager::InDevilSquare()
{
	return WorldActive == WD_9DEVILSQUARE;
}

bool CMapManager::InHellas(int iMap)
{
	const int map = (iMap >= 0) ? iMap : WorldActive;
	return IsMapInRange(map, WD_24HELLAS, WD_24HELLAS_END) || map == WD_24HELLAS_7;
}

bool CMapManager::InHiddenHellas(int iMap)
{
	const int map = (iMap >= 0) ? iMap : WorldActive;
	return map == WD_24HELLAS_7;
}

bool CMapManager::IsPKField()
{
	return WorldActive == WD_63PK_FIELD;
}

bool CMapManager::IsCursedTemple()
{
	return IsMapInRange(WorldActive, WD_45CURSEDTEMPLE_LV1, WD_45CURSEDTEMPLE_LV6);
}

bool CMapManager::IsEmpireGuardian1()
{
	return WorldActive == WD_69EMPIREGUARDIAN1;
}

bool CMapManager::IsEmpireGuardian2()
{
	return WorldActive == WD_70EMPIREGUARDIAN2;
}

bool CMapManager::IsEmpireGuardian3()
{
	return WorldActive == WD_71EMPIREGUARDIAN3;
}

bool CMapManager::IsEmpireGuardian4()
{
	return WorldActive == WD_72EMPIREGUARDIAN4;
}

bool CMapManager::IsEmpireGuardian()
{
	return IsEmpireGuardian1() || IsEmpireGuardian2() || IsEmpireGuardian3() || IsEmpireGuardian4();
}

bool CMapManager::InBattleCastle(int iMap)
{
	const int map = (iMap >= 0) ? iMap : WorldActive;
	return map == WD_30BATTLECASTLE;
}

const char* CMapManager::GetMapName(int iMap)
{
	switch (iMap)
	{
	case WD_74NEW_CHARACTER_SCENE: return "Character Scene";
	case WD_78NEW_CHARACTER_SCENE: return "Character Scene";
	case WD_73NEW_LOGIN_SCENE: return "Login Scene";
	case WD_77NEW_LOGIN_SCENE: return "Login Scene";
	case WD_55LOGINSCENE: return "Login Scene";
	default: return "Unknown";
	}
}

#endif

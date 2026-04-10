#pragma once

#if !defined(__ANDROID__)
#include "Lua.h"
#include "LuaCharacter.h"
#include "LuaInterface.h"
#include "LuaLoadImage.h"
#include "LuaGlobal.h"
#include "Protocol.h"
#endif

class CCharacterList {
public:
	CCharacterList();
	~CCharacterList();

	void Init();
	static void OpenCharacterSceneData();
	static void MoveCharacterList();
	static void RenderCharacterList();
	void SetCharacterPosition(int slot);

private:
#if !defined(__ANDROID__)
	CLuaCharacter m_LuaCharacter;
	LuaInterface m_LuaInterface;
	LuaLoadImage m_LuaLoadImage;
	LuaGlobal m_LuaGlobal;
	Lua m_Lua;
#endif

public:
	float MountAddSize;
	float FlyingAddSize;
	float FlyingAddHeight;
	int MaxCharacters;
	int MaxCharactersAccount;
};

extern CCharacterList gCharacterList;

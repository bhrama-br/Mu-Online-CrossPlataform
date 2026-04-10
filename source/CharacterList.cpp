#include "stdafx.h"
#include "CharacterList.h"
#include "Console.h"
#include "LuaStack.hpp"
#include "Protect.h"
#include "UIMng.h"

using namespace luaaa;

CCharacterList gCharacterList;

CCharacterList::CCharacterList() : m_Lua()
{

}

CCharacterList::~CCharacterList()
{

}

int CheckCreateCharacterWindow() 
{
	CUIMng& rUIMng = CUIMng::Instance();

	return rUIMng.m_CharMakeWin.IsShow();
}

void SetMaxCharacter(int value) 
{
	gCharacterList.MaxCharacters = value;
}

void SetPetSize(float MountSize, float FlyingSize, float FlyingHeight) 
{
	gCharacterList.MountAddSize = MountSize;
	gCharacterList.FlyingAddSize = FlyingSize;
	gCharacterList.FlyingAddHeight = FlyingHeight;
}

void CCharacterList::Init()
{
	if (gProtect->m_MainInfo.CharListS13 == 1) 
	{
		LuaModule(this->m_Lua.GetState()).fun("SetPetSize", &SetPetSize);

		LuaModule(this->m_Lua.GetState()).fun("SetMaxCharacter", &SetMaxCharacter);

		LuaModule(this->m_Lua.GetState()).fun("CheckCreateCharacterWindow", &CheckCreateCharacterWindow);

		this->m_LuaCharacter.RegisterLuaCharacter(this->m_Lua.GetState());

		this->m_LuaInterface.RegisterInterface(this->m_Lua.GetState());

		this->m_LuaLoadImage.SetFunctions(this->m_Lua.GetState());

		this->m_LuaGlobal.RegisterGlobal(this->m_Lua.GetState());

		this->m_Lua.DoFile("Data//Configs//Lua//CharacterSystem//CharacterList.lua");
	}
}

void CCharacterList::OpenCharacterSceneData()
{
	if (gProtect->m_MainInfo.CharListS13 == 1)
	{
		gCharacterList.m_Lua.Generic_Call("StartLoadImages", ">");
	}
}

void CCharacterList::MoveCharacterList()
{
	if (gProtect->m_MainInfo.CharListS13 == 1 && SceneFlag == CHARACTER_SCENE)
	{
		gCharacterList.m_Lua.Generic_Call("UpdateProc", "i>", gCharacterList.MaxCharactersAccount);
	}
}

void CCharacterList::RenderCharacterList()
{
	if (gProtect->m_MainInfo.CharListS13 == 1 && SceneFlag == CHARACTER_SCENE)
	{
		gCharacterList.m_Lua.Generic_Call("RenderProc", "i>", gCharacterList.MaxCharactersAccount);
	}
}

void CCharacterList::SetCharacterPosition(int slot)
{
	if (gProtect->m_MainInfo.CharListS13 == 1)
	{
		gCharacterList.m_Lua.Generic_Call("SetCharacterPosition", "i>", slot);
	}
}
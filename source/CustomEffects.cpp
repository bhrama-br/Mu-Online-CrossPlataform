#include "stdafx.h"
#include "CustomEffects.h"

CCustomEffects gCustomEffects;

CCustomEffects::CCustomEffects() : m_Lua()
{

}

CCustomEffects::~CCustomEffects()
{

}

void CCustomEffects::Init()
{
	this->m_Lua.DoFile("Data//Configs//Lua//EffectSystem//CharacterEffectItens.lua");

	this->m_LuaEffects.RegisterLuaClass(this->m_Lua.GetState());
}


void CCustomEffects::SetInfo(DWORD_PTR ObjectStruct, BMD* BMDStruct, int Type)
{
	gCustomEffects.m_Lua.Generic_Call("CharacterItensEffect", "jjj>", ObjectStruct, BMDStruct, Type);
}

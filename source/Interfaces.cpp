#include "stdafx.h"
#include "Interfaces.h"
#include "LuaSocket.h"
#include "LuaCharacter.h"

CInterface gInterface;

CInterface::CInterface() : m_Lua() 
{
	this->interfaceLock = 0;
}

CInterface::~CInterface() 
{

}

void CInterface::Init()
{
	gSocket.SetGlobalFunctions(this->m_Lua.GetState());

	m_LuaGlobal.RegisterGlobal(this->m_Lua.GetState());

	m_LuaInterface.RegisterInterface(this->m_Lua.GetState());

	m_LuaCharacter.RegisterLuaCharacter(this->m_Lua.GetState());

	m_LuaUser.RegisterLuaUser(this->m_Lua.GetState());

	m_LoadImage.SetFunctions(this->m_Lua.GetState());

	this->m_Lua.DoFile("Data//Configs//Lua//Controller//Interface.lua");
}

void CInterface::RestartLua()
{
	this->m_Lua.CloseLua();

	this->m_Lua.RegisterLua();

	gSocket.SetGlobalFunctions(this->m_Lua.GetState());

	m_LuaGlobal.RegisterGlobal(this->m_Lua.GetState());

	m_LuaInterface.RegisterInterface(this->m_Lua.GetState());

	m_LuaCharacter.RegisterLuaCharacter(this->m_Lua.GetState());

	m_LuaUser.RegisterLuaUser(this->m_Lua.GetState());

	m_LoadImage.SetFunctions(this->m_Lua.GetState());

	this->m_Lua.DoFile("Data//Configs//Lua//Controller//Interface.lua");

	this->m_Lua.Generic_Call("FinalBoot", ">");
}
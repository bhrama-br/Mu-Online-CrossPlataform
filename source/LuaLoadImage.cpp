#include "stdafx.h"
#include "LuaLoadImage.h"
#include "LuaStack.hpp"
#include "ZzzTexture.h"

LuaLoadImage gLuaLoadImage;

int m_LuaStartImage = 200000;

LuaLoadImage::LuaLoadImage() : m_Lua()
{
	
}

LuaLoadImage::~LuaLoadImage()
{
}

void LuaImage(char* string, int textureID) 
{
	LoadBitmap(string, textureID);
}

int LoadImageByDir(char* string) 
{
	LoadBitmap(string, m_LuaStartImage);

	return m_LuaStartImage++;
}

void LuaLoadImage::SetFunctions(lua_State * lua)
{
	luaaa::LuaModule(lua).fun("LoadImage", &LuaImage);

	luaaa::LuaModule(lua).fun("LoadImageByDir", &LoadImageByDir);
}

void LuaLoadImage::Init()
{
	luaaa::LuaModule(this->m_Lua.GetState()).fun("LoadImage", &LuaImage);

	luaaa::LuaModule(this->m_Lua.GetState()).fun("LoadImageByDir", &LoadImageByDir);

	this->m_Lua.DoFile("Data\\Configs\\Lua\\Controller\\LoadImages.lua");

	this->m_Lua.Generic_Call("LoadImages", ">");
}
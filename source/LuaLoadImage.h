#pragma once

#include "Lua.h"

class LuaLoadImage {
public:
	LuaLoadImage();
	~LuaLoadImage();

	void SetFunctions(lua_State* lua);
	void Init();

private:
	Lua m_Lua;
};

extern LuaLoadImage gLuaLoadImage;
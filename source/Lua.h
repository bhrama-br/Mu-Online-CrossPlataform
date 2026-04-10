#pragma once

extern "C" {
#if defined(__ANDROID__)
#include "../Dependencies/include/luajit/lua.h"
#include "../Dependencies/include/luajit/lua.hpp"
#include "../Dependencies/include/luajit/lauxlib.h"
#else
#include <luajit/lua.h>
#include <luajit/lua.hpp>
#include <luajit/lauxlib.h>
#endif
}

#include "CriticalSection.h"

class Lua {
public:
	Lua();
	virtual ~Lua();

	void RegisterLua();
	void CloseLua();
	lua_State* GetState();
	void DoFile(const char* szFileName);
	bool Generic_Call(const char* func, const char* sig, ...);

private:
	lua_State* m_luaState;

private:
	CCriticalSection m_critical;
};

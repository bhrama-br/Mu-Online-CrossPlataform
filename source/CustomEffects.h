#pragma once

#if !defined(__ANDROID__)
#include "Lua.h"
#include "LuaEffectsNormal.h"
#endif

class BMD;

class CCustomEffects {
public:
	CCustomEffects();
	~CCustomEffects();

	void Init();
	void SetInfo(DWORD_PTR ObjectStruct, BMD* BMDStruct, int Type);

private:
#if !defined(__ANDROID__)
	Lua m_Lua;
	CLuaEffectNormal m_LuaEffects;
#endif
};

extern CCustomEffects gCustomEffects;

#pragma once

#if !defined(__ANDROID__)
#include "Lua.h"
#endif

class CCustomSetEffect {
public:
	CCustomSetEffect();
	~CCustomSetEffect();
	void CreateEffectSetPlayer(DWORD_PTR ObjectStruct, DWORD ItemIndex);

	void Init();

public:
#if !defined(__ANDROID__)
	Lua m_Lua;
#endif
};

extern CCustomSetEffect gCCustomSetEffect;

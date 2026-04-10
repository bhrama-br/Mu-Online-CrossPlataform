#pragma once

#include "Lua.h"

struct FONT_INFO {
	int Index;
	int Bold;
	int AddSize;
	int CharSet;
	std::string Name;
	DWORD Sublinhado;
	DWORD Italico;
	DWORD Riscado;
	float MultiSize;
};

class CCreateFont {
public:
	CCreateFont();
	~CCreateFont();

	void Init();
	void ReloadLua();
	void SetFont(int iFontSize, int FontHeight, int nFixFontSize, int nFixFontHeight);

public:
	Lua m_Lua;
	std::map<int, FONT_INFO> m_fontInfo;
	std::map<int, HFONT> m_newFont;
};

extern CCreateFont gCreateFont;
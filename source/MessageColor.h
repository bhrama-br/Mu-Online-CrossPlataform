#pragma once

#include "Lua.h"

struct CUSTOM_MESSAGE_COLOR_INFO {
	int MessageID;
	int Type;
	int TextR;
	int TextG;
	int TextB;
	int TextA;
	int BgR;
	int BgG;
	int BgB;
	int BgA;
};

class CMessageColor {
public:
	CMessageColor();
	~CMessageColor();
	void SetMessageColor(int m_Message);
	bool CheckIsColor(int Color, int Type);

	void Init();
	CUSTOM_MESSAGE_COLOR_INFO* getInfo(int messageID);

public:
	Lua m_Lua;

	std::map<int, CUSTOM_MESSAGE_COLOR_INFO> m_CustomMessageColorInfo;
};

extern CMessageColor gMessageColor;
#include "stdafx.h"

#if defined(__ANDROID__)

// MSVC allows 'extern float g_fScreenRate_x;' inside 'namespace SEASON3B' to
// resolve to the global-scope g_fScreenRate_x. Clang treats these as separate
// symbols (SEASON3B::g_fScreenRate_x vs ::g_fScreenRate_x).
//
// This file provides the SEASON3B:: and SEASON4A:: namespace definitions that
// the NewUI code expects on Android/Clang.

#include "ZzzBMD.h"

class JewelHarmonyInfo;

namespace SEASON3B
{
	float g_fScreenRate_x = 1.0f;
	float g_fScreenRate_y = 1.0f;
	int MouseY = 0;
	int SelectedCharacter = -1;
	int TextNum = 0;
	char TextList[50][100] = {};
	int TextListColor[50] = {};
	int TextBold[50] = {};
	int ItemHelp = 0;
	int g_iItemInfo[16][17] = {};  // [iMaxLevel+1][iMaxColumn]
	int g_iCancelSkillTarget = 0;
	int g_iLengthAuthorityCode = 20;
	JewelHarmonyInfo* g_pUIJewelHarmonyinfo = nullptr;
}

namespace SEASON4A
{
	float IntensityTransform[MAX_MESH][MAX_VERTICES] = {};
	int g_iLimitAttackTime = 15;
}

#endif

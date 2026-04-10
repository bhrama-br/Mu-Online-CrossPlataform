#pragma once

#if defined(__ANDROID__)

extern CHARACTER* Hero;
extern ITEM PickItem;
extern WORD TerrainWall[];

#ifndef VK_LCONTROL
#define VK_LCONTROL 0xA2
#endif

#ifndef timeGetTime
#define timeGetTime GetTickCount
#endif

template <typename T>
inline T legacy_android_min(T left, T right)
{
	return (left < right) ? left : right;
}

template <typename T>
inline T legacy_android_max(T left, T right)
{
	return (left > right) ? left : right;
}

inline int TERRAIN_INDEX(int x, int y)
{
	return y * TERRAIN_SIZE + x;
}

inline int TERRAIN_INDEX_REPEAT(int x, int y)
{
	return ((y) & TERRAIN_SIZE_MASK) * TERRAIN_SIZE + ((x) & TERRAIN_SIZE_MASK);
}

inline WORD TERRAIN_ATTRIBUTE(float x, float y)
{
	return TerrainWall[TERRAIN_INDEX_REPEAT((int)(x / TERRAIN_SCALE), (int)(y / TERRAIN_SCALE))];
}

#define g_isNotCharacterBuff(o) \
	(o)->m_BuffMap.isBuff()

#define g_isCharacterBuff(o, bufftype) \
	(o)->m_BuffMap.isBuff(bufftype)

#define g_isCharacterBufflist(o, bufftypelist) \
	(o)->m_BuffMap.isBuff(bufftypelist)

#define g_TokenCharacterBuff(o, bufftype) \
	(o)->m_BuffMap.TokenBuff(bufftype)

#define g_CharacterBuffCount(o, bufftype) \
	(o)->m_BuffMap.GetBuffCount(bufftype)

#define g_CharacterBuffSize(o) \
	(o)->m_BuffMap.GetBuffSize()

#define g_CharacterBuff(o, iterindex) \
	(o)->m_BuffMap.GetBuff(iterindex)

#define g_CharacterRegisterBuff(o, bufftype) \
	(o)->m_BuffMap.RegisterBuff(bufftype)

#define g_CharacterRegisterBufflist(o, bufftypelist) \
	(o)->m_BuffMap.RegisterBuff(bufftypelist)

#define g_CharacterUnRegisterBuff(o, bufftype) \
	(o)->m_BuffMap.UnRegisterBuff(bufftype)

#define g_CharacterUnRegisterBuffList(o, bufftypelist) \
	(o)->m_BuffMap.UnRegisterBuff(bufftypelist)

#define g_CharacterCopyBuff(outObj, inObj) \
	(outObj)->m_BuffMap.m_Buff = (inObj)->m_BuffMap.m_Buff

#define g_CharacterClearBuff(o) \
	(o)->m_BuffMap.ClearBuff()

#endif

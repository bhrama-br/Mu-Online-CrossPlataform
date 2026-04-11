#pragma once

#if defined(__ANDROID__)

#include <type_traits>

extern CHARACTER* Hero;
extern ITEM PickItem;
extern WORD TerrainWall[];

// Freestanding wrappers for CCharacterManager member functions
// used unqualified in PBG_ADD_NEWCHAR_MONK code paths
inline int GetBaseClass(int iClass) { return (0x7 & iClass); }
inline bool IsThirdClass(int iClass) { return ((iClass >> 4) & 1) != 0; }

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

// Overloads for mixed-type calls (e.g. max(int, size_t))
template <typename T, typename U>
inline typename std::common_type<T, U>::type legacy_android_min(T left, U right)
{
	typedef typename std::common_type<T, U>::type CT;
	return (static_cast<CT>(left) < static_cast<CT>(right)) ? static_cast<CT>(left) : static_cast<CT>(right);
}

template <typename T, typename U>
inline typename std::common_type<T, U>::type legacy_android_max(T left, U right)
{
	typedef typename std::common_type<T, U>::type CT;
	return (static_cast<CT>(left) > static_cast<CT>(right)) ? static_cast<CT>(left) : static_cast<CT>(right);
}

#ifndef min
#define min(a,b) legacy_android_min(a,b)
#endif
#ifndef max
#define max(a,b) legacy_android_max(a,b)
#endif

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

// Buff macros now provided by _GlobalFunctions.h (included on Android too)

#define g_CharacterClearBuff(o) \
	(o)->m_BuffMap.ClearBuff()

#endif

#include "stdafx.h"

#if defined(__ANDROID__) && !defined(MU_ANDROID_HAS_LEGACYOBJECT_RUNTIME)

#include "w_Buff.h"

#include <list>

Buff::Buff()
{
}

Buff::~Buff()
{
	m_Buff.clear();
}

BuffPtr Buff::Make()
{
	return BuffPtr(new Buff());
}

void Buff::RegisterBuff(eBuffState buffstate)
{
	m_Buff[buffstate] = 1;
}

void Buff::RegisterBuff(std::list<eBuffState> buffstate)
{
	for (std::list<eBuffState>::const_iterator iter = buffstate.begin(); iter != buffstate.end(); ++iter)
	{
		RegisterBuff(*iter);
	}
}

void Buff::UnRegisterBuff(eBuffState buffstate)
{
	m_Buff.erase(buffstate);
}

void Buff::UnRegisterBuff(std::list<eBuffState> buffstate)
{
	for (std::list<eBuffState>::const_iterator iter = buffstate.begin(); iter != buffstate.end(); ++iter)
	{
		UnRegisterBuff(*iter);
	}
}

bool Buff::isBuff()
{
	return !m_Buff.empty();
}

bool Buff::isBuff(eBuffState buffstate)
{
	return m_Buff.find(buffstate) != m_Buff.end();
}

const eBuffState Buff::isBuff(std::list<eBuffState> buffstatelist)
{
	for (std::list<eBuffState>::const_iterator iter = buffstatelist.begin(); iter != buffstatelist.end(); ++iter)
	{
		if (isBuff(*iter))
		{
			return *iter;
		}
	}

	return eBuffNone;
}

void Buff::TokenBuff(eBuffState curbufftype)
{
	RegisterBuff(curbufftype);
}

const DWORD Buff::GetBuffSize()
{
	return static_cast<DWORD>(m_Buff.size());
}

const eBuffState Buff::GetBuff(int iterindex)
{
	if (iterindex < 0 || iterindex >= static_cast<int>(m_Buff.size()))
	{
		return eBuffNone;
	}

	int current_index = 0;
	for (BuffStateMap::const_iterator iter = m_Buff.begin(); iter != m_Buff.end(); ++iter, ++current_index)
	{
		if (current_index == iterindex)
		{
			return iter->first;
		}
	}

	return eBuffNone;
}

const DWORD Buff::GetBuffCount(eBuffState buffstate)
{
	BuffStateMap::const_iterator iter = m_Buff.find(buffstate);
	return (iter != m_Buff.end()) ? iter->second : 0;
}

void Buff::ClearBuff()
{
	m_Buff.clear();
}

bool Buff::IsEqualBuffType(IN int iBuffType, OUT unicode::t_char* szBuffName)
{
	(void)iBuffType;
	if (szBuffName != NULL)
	{
		szBuffName[0] = 0;
	}
	return false;
}

#endif

#include "stdafx.h"
#include "DarkSpiritView.h"

DARK_SPIRIT_HELPER_VIEWPtr DARK_SPIRIT_HELPER_VIEW::Make()
{
	DARK_SPIRIT_HELPER_VIEWPtr DarkSpirit(new DARK_SPIRIT_HELPER_VIEW);
	DarkSpirit->Init();
	return DarkSpirit;
}

bool DARK_SPIRIT_HELPER_VIEW::Create(CHARACTER* m_Owner, int ModelIndex)
{
	if (this->m_pOwner || m_Owner == nullptr) {
		return 0;
	}

	if (this->ModelIndex != -1 || ModelIndex == -1) {
		return 0;
	}

	this->m_pOwner = m_Owner;
	this->ModelIndex = ModelIndex;

	return 1;
}

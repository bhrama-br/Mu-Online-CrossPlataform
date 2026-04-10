#include "stdafx.h"
#include "ElementPet.h"
#include "HelperSystem.h"
#include "ZzzCharacter.h"
#include "CharacterList.h"

CElementPet gElementPetFirst;
CElementPet gElementPetSecond;

CElementPet::CElementPet()
{
	m_ElementPet.clear();
}

CElementPet::~CElementPet()
{
	//this->Relese();
}

void CElementPet::Relese()
{
	for (std::vector<BoostSmart_Ptr(HELPER_VIEW)>::iterator it = m_ElementPet.begin(); it != m_ElementPet.end();) 
	{
		std::vector<BoostSmart_Ptr(HELPER_VIEW)>::iterator tempIT = it;

		++it;

		BoostWeak_Ptr(HELPER_VIEW) element = *tempIT;

		if (element.expired() == false)
		{
			element.lock()->Relese();
		}

		//m_ElementPet.erase(tempIT);
	}

	m_ElementPet.clear();
}

bool CElementPet::CreateHelper(int Index, int ModelIndex, vec3_t Position, CHARACTER* m_Owner, int SubType)
{
	if (m_Owner == nullptr) 
	{
		return 0;
	}

	if (CheckExistsHelper(m_Owner))
	{
		return 1;
	}

	HELPER_VIEWPtr element = HELPER_VIEW::Make();

	if (element == nullptr) 
	{
		return 0;
	}

	if (element->Create(Index, ModelIndex, Position, m_Owner, SubType)) 
	{
		auto elementInfo = gHelperSystem.GetHelper(ModelIndex);

		if (elementInfo == 0) 
		{
			return 0;
		}

		element->m_Type = elementInfo->Type;

		element->m_HeightFloor = elementInfo->HeightFloor;

		element->m_Movement = elementInfo->Movement;

		if (SceneFlag == 4)
		{
			if (elementInfo->Type == 0 && element->m_Movement != 2)
			{
				element->SetScale(elementInfo->Size + gCharacterList.FlyingAddSize);
			}
			else
			{
				element->SetScale(elementInfo->Size + gCharacterList.MountAddSize);
			}
		}
		else 
		{
			element->SetScale(elementInfo->Size);
		}

		element->SetMiniature(elementInfo->Miniature, elementInfo->SizeMiniature, elementInfo->VelocityMiniature);

		this->Register(element);
		return 1;
	}

	return 0;
}

void CElementPet::DeleteHelper(CHARACTER * Owner, int itemType, bool allDelete)
{
	if (Owner == nullptr)
	{
		return;
	}

	UnRegister(Owner, itemType, allDelete);
}

void CElementPet::Register(BoostSmart_Ptr(HELPER_VIEW) info)
{
	this->m_ElementPet.push_back(info);
}

void CElementPet::UnRegister(CHARACTER* Owner, int itemType, bool isUnregistAll)
{
	if (Owner == nullptr)
	{
		return;
	}

	for (std::vector<BoostSmart_Ptr(HELPER_VIEW)>::iterator it = m_ElementPet.begin(); it != m_ElementPet.end();) 
	{
		std::vector<BoostSmart_Ptr(HELPER_VIEW)>::iterator tempIT = it;

		++it;

		BoostWeak_Ptr(HELPER_VIEW) element = *tempIT;

		if (Hero == Owner) 
		{
			if (element.lock()->IsSameOwner(&Owner->Object))
			{
				element.lock()->Relese();
				m_ElementPet.erase(tempIT);
				break;
			}
		}
		else
		{
			if ((-1 == itemType && element.lock()->IsSameOwner(&Owner->Object))	|| element.lock()->IsSameObject(&Owner->Object, itemType))
			{
				element.lock()->Relese();

				m_ElementPet.erase(tempIT);

				if (-1 == itemType || !isUnregistAll) 
				{
					return;
				}
			}
		}
	}
}

bool CElementPet::CheckExistsHelper(CHARACTER * Owner)
{
	if (Owner == nullptr)
	{
		return 0;
	}

	for (std::vector<BoostSmart_Ptr(HELPER_VIEW)>::iterator it = m_ElementPet.begin(); it != m_ElementPet.end();)
	{
		std::vector<BoostSmart_Ptr(HELPER_VIEW)>::iterator tempIT = it;

		++it;

		BoostWeak_Ptr(HELPER_VIEW) element = *tempIT;

		if (element.expired() == false)
		{
			if (element.lock()->IsSameOwner(&Owner->Object))
			{
				return 1;
			}
		}
	}

	return 0;
}

void CElementPet::RenderElement()
{
	for (std::vector<BoostSmart_Ptr(HELPER_VIEW)>::iterator it = m_ElementPet.begin(); it != m_ElementPet.end();) 
	{
		std::vector<BoostSmart_Ptr(HELPER_VIEW)>::iterator tempIT = it;

		++it;

		BoostWeak_Ptr(HELPER_VIEW) element = *tempIT;

		if (element.expired() == false)
		{
			element.lock()->Render();
		}
	}
}

void CElementPet::UpdateElement()
{
	for (std::vector<BoostSmart_Ptr(HELPER_VIEW)>::iterator it = m_ElementPet.begin(); it != m_ElementPet.end();) 
	{
		std::vector<BoostSmart_Ptr(HELPER_VIEW)>::iterator tempIT = it;

		++it;

		BoostWeak_Ptr(HELPER_VIEW) element = *tempIT;

		if (element.expired() == false) 
		{
			element.lock()->Update();
		}
	}
}
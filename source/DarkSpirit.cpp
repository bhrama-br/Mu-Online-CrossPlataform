#include "stdafx.h"
#include "DarkSpirit.h"
#include "LuaStack.hpp"
#include "Console.h"
#include "LuaObject.h"
#include "HelperManager.h"
#include "ZzzCharacter.h"

CDarkSpirit gDarkSpirit;

CDarkSpirit::CDarkSpirit() : m_Lua()
{
	this->m_DarkSpiritInfo.clear();
	this->m_DarkSpirit.clear();
	this->m_DarkSpiritModels.clear();
}

CDarkSpirit::~CDarkSpirit()
{

}

int CDarkSpirit::CheckNewRaven(CHARACTER* Struct)
{
	auto Raven = gDarkSpirit.getDarkSpirit(Struct->Weapon[1].Type - MODEL_ITEM);

	if (Raven != nullptr) {
		return Raven->ItemModel;
	}

	int GetRavenExist = gDarkSpirit.GetItemDarkSpirit(Hero);

	if (GetRavenExist != 0) 
	{
		auto Raven = gDarkSpirit.getDarkSpirit(GetRavenExist - MODEL_ITEM);

		if (Raven != nullptr) 
		{
			return Raven->ItemModel;
		}
	}

	return 0;
}

int CheckCreateDarkSpiritNow(int Index) 
{
	if (gDarkSpirit.CheckExistDarkSpirit(Hero)) 
	{
		return 1;
	}

	if (gDarkSpirit.checkIsDarkSpirit(Index - MODEL_ITEM)) 
	{
		return 1;
	}

	return 0;
}

int CheckDarkSpiritCustomModel(int Index) 
{
	if (gDarkSpirit.checkIsDarkSpirit(Index - MODEL_ITEM)) 
	{
		return 1;
	}

	return 0;
}

int CheckDarkSpiritCustom(int Index) 
{
	if (gDarkSpirit.checkIsDarkSpirit(Index)) 
	{
		return 1;
	}

	return 0;
}

void SetDarkSpirit(int ItemIndex, std::string modelName, std::string objectName) 
{
	DARK_SPIRIT_INFO info;

	info.ItemIndex = ItemIndex;

	strcpy_s(info.ModelName, modelName.c_str());

	strcpy_s(info.ObjectName, objectName.c_str());

	gDarkSpirit.m_DarkSpiritInfo.insert(std::make_pair(ItemIndex, info));
}

void CDarkSpirit::Init()
{
	luaaa::LuaModule(this->m_Lua.GetState()).fun("SetDarkSpirit", &SetDarkSpirit);

	this->m_Lua.DoFile("Data//Configs//Lua//CharacterSystem//DarkSpirit.lua");

	this->m_LuaBMD.RegisterClassBMD(this->m_Lua.GetState());

	RegisterClassObject(this->m_Lua.GetState());

	this->m_LuaEffects.RegisterLuaClass(this->m_Lua.GetState());
}

void CDarkSpirit::CreateDarkSpirit(CHARACTER * m_Owner, int ModelIndex)
{
	if (m_Owner == nullptr) {
		return;
	}

	if (this->CheckExistDarkSpirit(m_Owner)) {
		return;
	}

	DARK_SPIRIT_HELPER_VIEWPtr darkspirit = DARK_SPIRIT_HELPER_VIEW::Make();

	if (darkspirit == nullptr) {
		return;
	}

	if (darkspirit->Create(m_Owner, ModelIndex)) {
		this->Register(darkspirit);
	}
}

bool CDarkSpirit::CheckExistDarkSpirit(CHARACTER* Owner)
{
	if (Owner == nullptr) {
		return 0;
	}

	for (std::vector<BoostSmart_Ptr(DARK_SPIRIT_HELPER_VIEW)>::iterator it = this->m_DarkSpirit.begin(); it != this->m_DarkSpirit.end();) {

		std::vector<BoostSmart_Ptr(DARK_SPIRIT_HELPER_VIEW)>::iterator tempIT = it;

		++it;

		BoostWeak_Ptr(DARK_SPIRIT_HELPER_VIEW) darkspirit = *tempIT;

		if (darkspirit.expired() == false) {
			if (darkspirit.lock()->IsSameOwner(Owner)) {
				return 1;
			}
		}
	}

	return 0;
}

int CDarkSpirit::GetItemDarkSpirit(CHARACTER* Owner)
{
	if (Owner == nullptr) {
		return 0;
	}

	for (std::vector<BoostSmart_Ptr(DARK_SPIRIT_HELPER_VIEW)>::iterator it = this->m_DarkSpirit.begin(); it != this->m_DarkSpirit.end();) {

		std::vector<BoostSmart_Ptr(DARK_SPIRIT_HELPER_VIEW)>::iterator tempIT = it;

		++it;

		BoostWeak_Ptr(DARK_SPIRIT_HELPER_VIEW) darkspirit = *tempIT;

		if (darkspirit.expired() == false) {
			if (darkspirit.lock()->IsSameOwner(Owner)) {
				return darkspirit.lock()->ModelIndex;
			}
		}
	}

	return 0;
}

void CDarkSpirit::Register(BoostSmart_Ptr(DARK_SPIRIT_HELPER_VIEW) info)
{
	this->m_DarkSpirit.push_back(info);
}

void CDarkSpirit::DeleteDarkSpirit(CHARACTER* Owner, int itemType, bool allDelete)
{
	if (Owner == nullptr) {
		return;
	}

	this->UnRegister(Owner, itemType, allDelete);
}

void CDarkSpirit::UnRegister(CHARACTER* Owner, int itemType, bool isUnregistAll)
{
	if (Owner == nullptr) {
		return;
	}

	for (std::vector<BoostSmart_Ptr(DARK_SPIRIT_HELPER_VIEW)>::iterator it = this->m_DarkSpirit.begin(); it != this->m_DarkSpirit.end();) {
		std::vector<BoostSmart_Ptr(DARK_SPIRIT_HELPER_VIEW)>::iterator tempIT = it;

		++it;

		BoostWeak_Ptr(DARK_SPIRIT_HELPER_VIEW) element = *tempIT;

		if (Hero == Owner) {
			if (element.lock()->IsSameOwner(Owner)) {
				element.lock()->Relese();
				this->m_DarkSpirit.erase(tempIT);
				break;
			}
		}
		else {
			if (-1 == itemType && element.lock()->IsSameOwner(Owner))
			{
				element.lock()->Relese();

				this->m_DarkSpirit.erase(tempIT);

				if (-1 == itemType || !isUnregistAll) {
					return;
				}
			}
		}
	}
}

DARK_SPIRIT_INFO * CDarkSpirit::getDarkSpirit(int ItemIndex)
{
	std::map<int, DARK_SPIRIT_INFO>::iterator it = this->m_DarkSpiritInfo.find(ItemIndex);

	if (it == this->m_DarkSpiritInfo.end()) {
		return nullptr;
	}

	return &it->second;
}

bool CDarkSpirit::checkIsDarkSpirit(int ItemIndex)
{
	std::map<int, DARK_SPIRIT_INFO>::iterator it = this->m_DarkSpiritInfo.find(ItemIndex);

	if (it == this->m_DarkSpiritInfo.end()) {
		return 0;
	}

	return 1;
}

int CDarkSpirit::GetItemModelDarkSpirit(int ItemIndex)
{
	return this->m_DarkSpiritModels[ItemIndex];
}

void CDarkSpirit::RenderModel(DWORD BMDStruct, DWORD ObjectStruct, DWORD ItemIndex)
{
	int ModelIndex = this->GetItemModelDarkSpirit(ItemIndex);

	this->m_Lua.Generic_Call("RenderModel", "iii>", BMDStruct, ObjectStruct, ModelIndex);
}


int CDarkSpirit::CheckCreateDarkSpiritNow(int Index)
{
	if (gDarkSpirit.CheckExistDarkSpirit(Hero)) 
	{
		return 1;
	}

	if (gDarkSpirit.checkIsDarkSpirit(Index - MODEL_ITEM)) 
	{
		return 1;
	}

	return 0;
}
#include "stdafx.h"
#include "NewInventory.h"
#include "NewUIMyInventory.h"
#include "NewUISystem.h"
#include "_struct.h"
#include "Console.h"
#include "ElementPet.h"
#include "HelperManager.h"
#include "GOBoid.h"
#include "w_PetProcess.h"
#include "ZzzEffect.h"
#include "DarkSpirit.h"
#include "CharacterManager.h"
#include "GIPetManager.h"
#include "CustomWing.h"

VisualInventory gVisualInventory;

void VisualInventory::AddItem(BYTE slot, BYTE* ItemInfo)
{
	ITEM* pTargetItemSlot = &CharacterMachine->Equipment[slot];

	if (pTargetItemSlot->Type > 0)
	{
		g_pMyInventory->UnequipItem(slot);
	}

	ITEM* pTempItem = g_pNewItemMng->CreateItem(ItemInfo);

	if (NULL == pTempItem)
	{
		return;
	}

	pTempItem->lineal_pos = slot;
	pTempItem->ex_src_type = ITEM_EX_SRC_EQUIPMENT;
	memcpy(pTargetItemSlot, pTempItem, sizeof(ITEM));
	g_pNewItemMng->DeleteItem(pTempItem);

	if (slot == 247 && gCustomWing.CheckCustomWingIsCape(pTargetItemSlot->Type + MODEL_ITEM) > 0) 
	{
		DeleteCloth(Hero, &Hero->Object);
	}

	if (slot == 248)
	{
		this->EffectHelperVisual(pTargetItemSlot);
	}
}

void VisualInventory::AddItemElemental(BYTE slot, BYTE* ItemInfo)
{
	ITEM* pTargetItemSlot = &CharacterMachine->Equipment[slot];

	if (pTargetItemSlot->Type > 0)
	{
		g_pMyInventory->UnequipItem(slot);
	}

	ITEM* pTempItem = g_pNewItemMng->CreateItem(ItemInfo);

	if (NULL == pTempItem)
	{
		return;
	}

	pTempItem->lineal_pos = slot;
	pTempItem->ex_src_type = ITEM_EX_SRC_EQUIPMENT;
	memcpy(pTargetItemSlot, pTempItem, sizeof(ITEM));
	g_pNewItemMng->DeleteItem(pTempItem);

	if (!this->AddDarkSpirit(pTempItem->Type))
	{
		this->EffectPet(slot, pTargetItemSlot);
	}
}

void VisualInventory::EffectPet(int slot, ITEM* pItem)
{
	auto hero = Hero;

	if (slot == 236)
	{
		if (gHelperSystem.CheckIsHelper(pItem->Type + MODEL_ITEM))
		{
			gElementPetFirst.CreateHelper(pItem->Type, gHelperSystem.GetHelperModel(pItem->Type + MODEL_ITEM), hero->Object.Position, hero, 0);
		}
	}
	else if (slot == 237)
	{
		if (gHelperSystem.CheckIsHelper(pItem->Type + MODEL_ITEM))
		{
			gElementPetSecond.CreateHelper(pItem->Type, gHelperSystem.GetHelperModel(pItem->Type + MODEL_ITEM), hero->Object.Position, hero, 0);
		}
	}
}

void VisualInventory::DeleteEffectPets(int slot)
{
	if (slot == 236)
	{
		gElementPetFirst.DeleteHelper(Hero);
	}
	else if (slot == 237)
	{
		gElementPetSecond.DeleteHelper(Hero);
	}
}

void VisualInventory::EffectHelperVisual(ITEM* pItem)
{
	auto c = Hero;
	
	auto getHelper = &CharacterMachine->Equipment[8];

	if (getHelper != nullptr)
	{
		DeleteBug(&c->Object);

		ThePetProcess().DeletePet(c);

		gHelperManager.DeleteHelper(c);
	}

	if (gHelperSystem.CheckIsHelper(pItem->Type + MODEL_ITEM))
	{
		c->Helper.Type = pItem->Type + MODEL_ITEM;

		gHelperManager.CreateHelper(pItem->Type, gHelperSystem.GetHelperModel(c->Helper.Type), c->Object.Position, c, 0);
	}
}

void VisualInventory::RecoveryOldHelper()
{
	auto getHelper = &CharacterMachine->Equipment[8];

	OBJECT* pHeroObject = &Hero->Object;

	if (getHelper != nullptr) 
	{
		auto c = Hero;

		if (gHelperSystem.CheckIsHelper(getHelper->Type + MODEL_ITEM)) 
		{
			DeleteBug(&c->Object);

			ThePetProcess().DeletePet(c);

			c->Helper.Type = getHelper->Type + MODEL_ITEM;

			gHelperManager.CreateHelper(getHelper->Type, gHelperSystem.GetHelperModel(c->Helper.Type), c->Object.Position, c, 0);
		}
		else
		{
			gHelperManager.DeleteHelper(c);

			c->Helper.Type = getHelper->Type + MODEL_ITEM;

			c->Helper.Level = 0;

			switch (getHelper->Type)
			{
			case ITEM_HELPER:
				CreateBug(MODEL_HELPER, pHeroObject->Position, pHeroObject);
				break;
			case ITEM_HELPER + 2:
				CreateBug(MODEL_UNICON, pHeroObject->Position, pHeroObject);
				if (!Hero->SafeZone)
					CreateEffect(BITMAP_MAGIC + 1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
				break;
			case ITEM_HELPER + 3:
				CreateBug(MODEL_PEGASUS, pHeroObject->Position, pHeroObject);
				if (!Hero->SafeZone)
					CreateEffect(BITMAP_MAGIC + 1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
				break;
			case ITEM_HELPER + 4:
				CreateBug(MODEL_DARK_HORSE, pHeroObject->Position, pHeroObject);
				if (!Hero->SafeZone)
					CreateEffect(BITMAP_MAGIC + 1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
				break;
			case ITEM_HELPER + 37:
				Hero->Helper.Option1 = getHelper->Option1;
				if (getHelper->Option1 == 0x01)
				{
					CreateBug(MODEL_FENRIR_BLACK, pHeroObject->Position, pHeroObject);
				}
				else if (getHelper->Option1 == 0x02)
				{
					CreateBug(MODEL_FENRIR_BLUE, pHeroObject->Position, pHeroObject);
				}
				else if (getHelper->Option1 == 0x04)
				{
					CreateBug(MODEL_FENRIR_GOLD, pHeroObject->Position, pHeroObject);
				}
				else
				{
					CreateBug(MODEL_FENRIR_RED, pHeroObject->Position, pHeroObject);
				}

				if (!Hero->SafeZone)
				{
					CreateEffect(BITMAP_MAGIC + 1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
				}
				break;
			case ITEM_HELPER + 64:
				ThePetProcess().CreatePet(getHelper->Type, MODEL_HELPER + 64, pHeroObject->Position, Hero);
				break;
			case ITEM_HELPER + 65:
				ThePetProcess().CreatePet(getHelper->Type, MODEL_HELPER + 65, pHeroObject->Position, Hero);
				break;
			case ITEM_HELPER + 67:
				ThePetProcess().CreatePet(getHelper->Type, MODEL_HELPER + 67, pHeroObject->Position, Hero);
				break;
			case ITEM_HELPER + 80:
				ThePetProcess().CreatePet(getHelper->Type, MODEL_HELPER + 80, pHeroObject->Position, Hero);
				break;
			case ITEM_HELPER + 106:
				ThePetProcess().CreatePet(getHelper->Type, MODEL_HELPER + 106, pHeroObject->Position, Hero);
				break;
			case ITEM_HELPER + 123:
				ThePetProcess().CreatePet(getHelper->Type, MODEL_HELPER + 123, pHeroObject->Position, Hero);
				break;
			}
		}
	}
}

bool VisualInventory::AddDarkSpirit(int ItemIndex)
{
	if (gDarkSpirit.checkIsDarkSpirit(ItemIndex)) 
	{
		auto hero = Hero;
		auto backupWeapon = hero->Weapon[1].Type;

		hero->Weapon[1].Type = ItemIndex + MODEL_ITEM;

		ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];

		PET_INFO* pPetInfo = giPetManager::GetPetInfo(pEquipmentItemSlot);

		giPetManager::CreatePetDarkSpirit(hero);

		if (!gMapManager.InChaosCastle())
		{
			((CSPetSystem*)hero->m_pPet)->SetPetInfo(pPetInfo);
		}

		gDarkSpirit.CreateDarkSpirit(hero, hero->Weapon[1].Type);

		hero->RavenCustom = hero->Weapon[1].Type;

		hero->Weapon[1].Type = backupWeapon;


		//if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD)
		{
			for (int i = 0; i < PET_CMD_END; ++i)
			{
				CharacterAttribute->Skill[AT_PET_COMMAND_DEFAULT + i] = AT_PET_COMMAND_DEFAULT + i;
			}
		}

		return 1;
	}

	return 0;
}


void VisualInventory::DeleteDarkSpirit(int ItemIndex)
{
	if (gDarkSpirit.checkIsDarkSpirit(ItemIndex))
	{
		DeletePet(Hero);

		if (gDarkSpirit.CheckExistDarkSpirit(Hero))
		{
			gDarkSpirit.DeleteDarkSpirit(Hero, -1, false);
			Hero->RavenCustom = -1;
		}
	}
}

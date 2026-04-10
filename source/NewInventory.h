#pragma once

class ITEMS_SLOTS_INFO
{
public:
	ITEMS_SLOTS_INFO()
	{
		this->slot = -1;
	};

	~ITEMS_SLOTS_INFO()
	{
	};

	void Clear()
	{
		this->slot = -1;
	}

	bool ItemEquiped;
	BYTE slot;
	ITEM zlpViewObj;
};

struct ITEM_INVENTORY_EQUIP
{
	int x;
	int y;
	int width;
	int height;
	int ImageID;
	int Active;
	int isPet;
};

class VisualInventory
{
public:
	void AddItem(BYTE slot, BYTE* ItemInfo);
	void AddItemElemental(BYTE slot, BYTE* ItemInfo);
	void EffectPet(int slot, ITEM* pItem);
	void DeleteEffectPets(int slot);
	void EffectHelperVisual(ITEM* pItem);
	void RecoveryOldHelper();
	bool AddDarkSpirit(int ItemIndex);
	void DeleteDarkSpirit(int ItemIndex);
public:
	ITEM_INVENTORY_EQUIP m_ItemInventoryEquip[12];
	std::map<int, ITEMS_SLOTS_INFO> m_ItemsInfo;
public:
	int InventoryVisualOpen;
	int InventoryVisualSlot;
public:

	int CheckVisualOpen()
	{
		return this->InventoryVisualOpen;
	};

	ITEM* getItem(int slot)
	{
		std::map<int, ITEMS_SLOTS_INFO>::iterator it = this->m_ItemsInfo.find(slot);

		if (it == this->m_ItemsInfo.end()) 
		{
			return nullptr;
		}

		return &it->second.zlpViewObj;
	};

	bool CheckItemExist(int slot)
	{
		std::map<int, ITEMS_SLOTS_INFO>::iterator it = this->m_ItemsInfo.find(slot);

		if (it == this->m_ItemsInfo.end())
		{
			return 0;
		}

		if (it->second.zlpViewObj.Type != -1)
		{
			return 1;
		}

		return 0;
	};
};

extern VisualInventory gVisualInventory;
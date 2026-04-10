// NewUIMyInventory.cpp: implementation of the CNewUIMyInventory class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIMyInventory.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "CSChaosCastle.h"
#include "GOBoid.h"
#include "ZzzEffect.h"
#include "GIPetManager.h"
#include "CSParts.h"
#include "UIJewelHarmony.h"
#include "CDirection.h"
#include "GMCryWolf1st.h"
#include "GMCryingWolf2nd.h"
#include "ZzzInventory.h"
#include "wsclientinline.h"
#include "MixMgr.h"
#include "ZzzLodTerrain.h"
#include "CSQuest.h"
#include "UIGuildInfo.h"
#include "UIManager.h"
#include "CSItemOption.h"
#include "MapManager.h"
#include "w_PetProcess.h"
#include "SocketSystem.h"
#include "w_CursedTemple.h"
#include "PortalMgr.h"
#ifdef CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
#include "Event.h"
#endif // CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
#include "GMUnitedMarketPlace.h"
#include "ChangeRingManager.h"
#ifdef PBG_ADD_NEWCHAR_MONK
#include "MonkSystem.h"
#endif //PBG_ADD_NEWCHAR_MONK
#include "CharacterManager.h"
#include "NewUIPCPoint.h"
#include "NewInventory.h"
#include "Platform/RenderColorCompat.h"
#include "Console.h"
#include "HelperManager.h"
#include "CustomWing.h"
#include "ElementPet.h"
#include "ItemMove.h"
#include "ElementSlots.h"
#include "TradeX.h"
#include "Visuals.h"

using namespace SEASON3B;

SEASON3B::CNewUIMyInventory::CNewUIMyInventory()
{ 
	m_pNewUIMng = NULL;
	m_pNewUI3DRenderMng = NULL;
	m_pNewInventoryCtrl = NULL;
	m_Pos.x = m_Pos.y = 0;

	memset(&m_EquipmentSlots, 0, sizeof(EQUIPMENT_ITEM)*MAX_EQUIPMENT_INDEX);
	m_iPointedSlot = -1;

	m_MyShopMode = MYSHOP_MODE_OPEN;
	m_RepairMode = REPAIR_MODE_OFF;
	m_dwStandbyItemKey = 0;

	m_bRepairEnableLevel = false;
	m_bMyShopOpen = false;
	//
	gVisualInventory.InventoryVisualOpen = 0;
	gVisualInventory.InventoryVisualSlot = 12;
}

SEASON3B::CNewUIMyInventory::~CNewUIMyInventory() 
{ 
	Release(); 
}

bool SEASON3B::CNewUIMyInventory::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng, int x, int y)
{
	if(NULL == pNewUIMng || NULL == pNewUI3DRenderMng || NULL == g_pNewItemMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_INVENTORY, this);

	m_pNewUI3DRenderMng = pNewUI3DRenderMng;
	m_pNewUI3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

	m_pNewInventoryCtrl = new CNewUIInventoryCtrl;
	if(false == m_pNewInventoryCtrl->Create(m_pNewUI3DRenderMng, g_pNewItemMng, this, x+15, y+200, 8, 8))
	{
		SAFE_DELETE(m_pNewInventoryCtrl);
		return false;
	}

	SetPos(x, y);
	LoadImages();
	SetEquipmentSlotInfo();
	SetButtonInfo();
	Show(false);
	return true;
}

void SEASON3B::CNewUIMyInventory::Release()
{
	if(m_pNewUI3DRenderMng)
		m_pNewUI3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);

	UnequipAllItems();
	DeleteAllItems();

	UnloadImages();

	SAFE_DELETE(m_pNewInventoryCtrl);

	if(m_pNewUI3DRenderMng)
	{
		m_pNewUI3DRenderMng->Remove3DRenderObj(this);
		m_pNewUI3DRenderMng = NULL;
	}
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

bool SEASON3B::CNewUIMyInventory::EquipItem(int iIndex, BYTE* pbyItemPacket)
{
	if(iIndex >= 0 && iIndex < MAX_EQUIPMENT_INDEX && g_pNewItemMng && CharacterMachine)
	{
		ITEM* pTargetItemSlot = &CharacterMachine->Equipment[iIndex];
		if(pTargetItemSlot->Type > 0)
		{
			UnequipItem(iIndex);
		}

		ITEM* pTempItem = g_pNewItemMng->CreateItem(pbyItemPacket);		

		if(NULL == pTempItem)	
		//if(NULL == pTempItem || false == IsEquipable(iIndex, pTempItem))	
		{
			return false;
		}

		if( pTempItem->Type == ITEM_HELPER+4 )
			SendRequestPetInfo( PET_TYPE_DARK_HORSE, 0, iIndex );

		if( pTempItem->Type == ITEM_HELPER+5 )
		{
			giPetManager::CreatePetDarkSpirit( Hero );
			SendRequestPetInfo( PET_TYPE_DARK_SPIRIT, 0, iIndex );
		}
		
		pTempItem->lineal_pos = iIndex;
		pTempItem->ex_src_type = ITEM_EX_SRC_EQUIPMENT;
		memcpy(pTargetItemSlot, pTempItem, sizeof(ITEM));
		g_pNewItemMng->DeleteItem(pTempItem);

		CreateEquippingEffect(pTargetItemSlot);
	}
	return false;
}

void SEASON3B::CNewUIMyInventory::UnequipItem(int iIndex)
{
	if (gVisualInventory.InventoryVisualOpen == 1)
	{
		if (iIndex >= NEW_EQUIPMENT_WEAPON_RIGHT && iIndex <= NEW_EQUIPMENT_HELPER)
		{
			ITEM* pEquippedItem = &CharacterMachine->Equipment[iIndex];

			if (iIndex == 248)
			{
				if (gHelperSystem.CheckIsHelper(pEquippedItem->Type + MODEL_ITEM))
				{
					gHelperManager.DeleteHelper(Hero);
				}
			}

			if (pEquippedItem && pEquippedItem->Type != -1)
			{
				pEquippedItem->Type = -1;
				pEquippedItem->Level = 0;
				pEquippedItem->Number = -1;
				pEquippedItem->Option1 = 0;
				pEquippedItem->ExtOption = 0;
				pEquippedItem->SocketCount = 0;
				for (int i = 0; i < MAX_SOCKETS; ++i)
				{
					pEquippedItem->SocketSeedID[i] = SOCKET_EMPTY;
					pEquippedItem->SocketSphereLv[i] = 0;
				}
				pEquippedItem->SocketSeedSetOption = 0;

				DeleteEquippingEffect();
			}

			if (iIndex == 247) 
			{
				DeleteCloth(Hero, &Hero->Object);
			}

			SetCharacterClass(Hero);

			if (iIndex == 248) 
			{
				gVisualInventory.RecoveryOldHelper();
			}
		}
	}
	else
	{
		if (iIndex >= 0 && iIndex < MAX_EQUIPMENT_INDEX && g_pNewItemMng && CharacterMachine)
		{
			ITEM* pEquippedItem = &CharacterMachine->Equipment[iIndex];

			if (pEquippedItem && pEquippedItem->Type != -1)
			{
				if (pEquippedItem->Type == ITEM_HELPER + 4)
				{
					Hero->InitPetInfo(PET_TYPE_DARK_HORSE);
				}
				else if (pEquippedItem->Type == ITEM_HELPER + 5)
				{
					giPetManager::DeletePet(Hero);
					Hero->InitPetInfo(PET_TYPE_DARK_SPIRIT);
				}

				if (pEquippedItem->Type != ITEM_HELPER + 5)
				{
					DeleteEquippingEffectBug(pEquippedItem);
				}


				pEquippedItem->Type = -1;
				pEquippedItem->Level = 0;
				pEquippedItem->Number = -1;
				pEquippedItem->Option1 = 0;
				pEquippedItem->ExtOption = 0;
				pEquippedItem->SocketCount = 0;
				for (int i = 0; i < MAX_SOCKETS; ++i)
				{
					pEquippedItem->SocketSeedID[i] = SOCKET_EMPTY;
					pEquippedItem->SocketSphereLv[i] = 0;
				}
				pEquippedItem->SocketSeedSetOption = 0;
				DeleteEquippingEffect();
			}
		}

		if (iIndex >= NEW_EQUIPMENT_HELPER_SP && iIndex <= NEW_EQUIPMENT_RING_LEFT && g_pNewItemMng && CharacterMachine)
		{
			ITEM* pEquippedItem = &CharacterMachine->Equipment[iIndex];

			if (pEquippedItem && pEquippedItem->Type != -1)
			{
				if (iIndex == 236)
				{
					gElementPetFirst.DeleteHelper(Hero);
				}
				if (iIndex == 237)
				{
					gElementPetSecond.DeleteHelper(Hero);
				}

				gVisualInventory.DeleteDarkSpirit(pEquippedItem->Type);

				pEquippedItem->Type = -1;
				pEquippedItem->Level = 0;
				pEquippedItem->Number = -1;
				pEquippedItem->Option1 = 0;
				pEquippedItem->ExtOption = 0;
				pEquippedItem->SocketCount = 0;
				for (int i = 0; i < MAX_SOCKETS; ++i)
				{
					pEquippedItem->SocketSeedID[i] = SOCKET_EMPTY;
					pEquippedItem->SocketSphereLv[i] = 0;
				}
				pEquippedItem->SocketSeedSetOption = 0;
				DeleteEquippingEffect();
			}
		}
	}
}
void SEASON3B::CNewUIMyInventory::UnequipAllItems()
{
	if(CharacterMachine)
	{
		for(int i=0; i<MAX_EQUIPMENT_INDEX; i++)
		{
			UnequipItem( i );
		}

		for (int i = NEW_EQUIPMENT_HELPER_SP; i < NEW_EQUIPMENT_WEAPON_RIGHT; i++)
		{
			UnequipItem(i);
		}

		for (int i = NEW_EQUIPMENT_WEAPON_RIGHT; i < MAX_NEW_EQUIPMENT_INDEX; i++)
		{
			UnequipItem(i);
		}
	}
}

bool SEASON3B::CNewUIMyInventory::IsEquipable(int iIndex, ITEM* pItem)
{
	if(pItem == NULL)
		return false;
	
	ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
	bool bEquipable = false;
	if( pItemAttr->RequireClass[gCharacterManager.GetBaseClass(Hero->Class)] )
		bEquipable = true;

	else if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK && pItemAttr->RequireClass[CLASS_WIZARD] 
		&& pItemAttr->RequireClass[CLASS_KNIGHT])
		bEquipable = true;

	BYTE byFirstClass = gCharacterManager.GetBaseClass(Hero->Class);
	BYTE byStepClass = gCharacterManager.GetStepClass(Hero->Class);
	if( pItemAttr->RequireClass[byFirstClass] > byStepClass)
	{
		return false;
	}

	if(bEquipable == false)
		return false;

	bEquipable = false;
	if (pItemAttr->m_byItemSlot == iIndex) {

		bEquipable = true;
	}

	else if(pItemAttr->m_byItemSlot == EQUIPMENT_WEAPON_RIGHT && iIndex == EQUIPMENT_WEAPON_LEFT)
	{
		if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK
#ifdef PBG_ADD_NEWCHAR_MONK
			|| GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
			)
		{
			if (!pItemAttr->TwoHand)
				bEquipable = true;
#ifdef PBG_FIX_EQUIP_TWOHANDSWORD
			else
			{
				bEquipable = false;
				return false;
			}
#endif //PBG_FIX_EQUIP_TWOHANDSWORD
		}
		else if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_SUMMONER &&
			!(pItem->Type >= ITEM_STAFF && pItem->Type <= ITEM_STAFF+MAX_ITEM_INDEX))
			bEquipable = true;
	}
	else if(pItemAttr->m_byItemSlot == EQUIPMENT_RING_RIGHT && iIndex == EQUIPMENT_RING_LEFT)
		bEquipable = true;

	if (pItemAttr->m_byItemSlot == iIndex && iIndex == EQUIPMENT_WEAPON_RIGHT && pItemAttr->TwoHand)
	{
		ITEM* pEquipment = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];

		if (pEquipment && pEquipment->Type != -1)
		{
			ITEM_ATTRIBUTE* pItemAttrLeft = &ItemAttribute[pEquipment->Type];

			if (pItemAttrLeft->m_byItemSlot == EQUIPMENT_WEAPON_RIGHT)
			{
				bEquipable = false;
			}
		}
	}

	if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF)
	{
		//ITEM *r = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
		ITEM *l = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT ];
		if (iIndex == EQUIPMENT_WEAPON_RIGHT && l->Type != ITEM_BOW+7
			&& (l->Type >= ITEM_BOW && l->Type < ITEM_BOW+MAX_ITEM_INDEX))
		{
			if (pItem->Type != ITEM_BOW+15)
				bEquipable = false;
		}
	}
#ifdef PBG_ADD_NEWCHAR_MONK
	if(GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
	{
		if(iIndex == EQUIPMENT_GLOVES)
			bEquipable = false;
		else if(pItemAttr->m_byItemSlot == EQUIPMENT_WEAPON_RIGHT)
			bEquipable = g_CMonkSystem.RageEquipmentWeapon(iIndex, pItem->Type);
	}
#endif //PBG_ADD_NEWCHAR_MONK

	if(bEquipable == false)
		return false;

	DWORD wStrength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
	DWORD wDexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
	DWORD wEnergy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
	DWORD wVitality = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
	DWORD wCharisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
	WORD wLevel = CharacterAttribute->Level;
	
	int iItemLevel = (pItem->Level>>3)&15;
	
	int iDecNeedStrength = 0, iDecNeedDex = 0;

	extern JewelHarmonyInfo*   g_pUIJewelHarmonyinfo;
	if( iItemLevel >= pItem->Jewel_Of_Harmony_OptionLevel )
	{
		StrengthenCapability SC;
		g_pUIJewelHarmonyinfo->GetStrengthenCapability( &SC, pItem, 0 );
		
		if( SC.SI_isNB ) 
		{
			iDecNeedStrength = SC.SI_NB.SI_force;
			iDecNeedDex      = SC.SI_NB.SI_activity;
		}
	}
	if (pItem->SocketCount > 0)
	{
		for (int i = 0; i < pItem->SocketCount; ++i)
		{
			if (pItem->SocketSeedID[i] == 38)
			{
				int iReqStrengthDown = g_SocketItemMgr.GetSocketOptionValue(pItem, i);
				iDecNeedStrength += iReqStrengthDown;
			}
			else if (pItem->SocketSeedID[i] == 39)
			{
				int iReqDexterityDown = g_SocketItemMgr.GetSocketOptionValue(pItem, i);
				iDecNeedDex += iReqDexterityDown;
			}
		}
	}

	if ( pItem->RequireStrength - iDecNeedStrength > wStrength )
		return false;
	if ( pItem->RequireDexterity - iDecNeedDex > wDexterity )
		return false;
	if ( pItem->RequireEnergy > wEnergy )
		return false;
	if ( pItem->RequireVitality > wVitality )
		return false;
	if( pItem->RequireCharisma > wCharisma )
		return false;
	if( pItem->RequireLevel > wLevel )
		return false;

	if(pItem->Type == ITEM_HELPER+5 )
	{
		PET_INFO* pPetInfo = giPetManager::GetPetInfo( pItem );
		WORD wRequireCharisma = (185+(pPetInfo->m_wLevel*15));
		if( wRequireCharisma > wCharisma ) 
			return false;
	}

	if (gMapManager.WorldActive==WD_7ATLANSE && (pItem->Type >= ITEM_HELPER+2 && pItem->Type <= ITEM_HELPER+3))
	{ 
		return false;
	}
	else if(pItem->Type==ITEM_HELPER+2 && gMapManager.WorldActive==WD_10HEAVEN)
	{
		return false;
	}
	else if(pItem->Type==ITEM_HELPER+2 && g_Direction.m_CKanturu.IsMayaScene())
	{
		return false;
	}
	else if ( gMapManager.InChaosCastle() || (M34CryWolf1st::Get_State_Only_Elf()
		&& g_isCharacterBuff((&Hero->Object), eBuff_CrywolfHeroContracted) ) )
	{ 
		if( (pItem->Type>=ITEM_HELPER+2 && pItem->Type<=ITEM_HELPER+5) || pItem->Type==ITEM_HELPER+37)
			return false;
	}
	else if( ( pItem->Type>=ITEM_HELPER+2 && pItem->Type<=ITEM_HELPER+4 || pItem->Type == ITEM_HELPER+37 )
		&& Hero->Object.CurrentAction>=PLAYER_SIT1 && Hero->Object.CurrentAction<=PLAYER_SIT_FEMALE2 )
	{
		return false;
	}

	return bEquipable;
}

bool SEASON3B::CNewUIMyInventory::InsertItem(int iIndex, BYTE* pbyItemPacket)
{
	if(m_pNewInventoryCtrl)
	{
		return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
	}

	return false;
}
void SEASON3B::CNewUIMyInventory::DeleteItem(int iIndex)
{
	if(m_pNewInventoryCtrl)
	{
		ITEM* pItem = m_pNewInventoryCtrl->FindItem(iIndex);
		if(pItem != NULL)
		{
			m_pNewInventoryCtrl->RemoveItem(pItem);
		}
		else
		{
			CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
			if(pPickedItem)
			{
				if(pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
				{
					if(pPickedItem->GetSourceLinealPos() == iIndex)
					{
						CNewUIInventoryCtrl::DeletePickedItem();
					}
				}
			}
		}
	}
}
void SEASON3B::CNewUIMyInventory::DeleteAllItems()
{
	if(m_pNewInventoryCtrl)
		m_pNewInventoryCtrl->RemoveAllItems();
}

void SEASON3B::CNewUIMyInventory::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
	
	SetEquipmentSlotInfo();
	
	m_pNewInventoryCtrl->SetPos(x+15, y+200);
	m_BtnExit.SetPos(m_Pos.x+13, m_Pos.y+392);
	m_BtnRepair.SetPos(m_Pos.x+45, m_Pos.y+392);
	m_BtnMyShop.SetPos(m_Pos.x+77, m_Pos.y+392);

	if (Visuals.VisualSwitch == 1)
	{
		m_BtnPrev.SetPos(m_Pos.x + 20, m_Pos.y + 10);
		m_BtnNext.SetPos(m_Pos.x + 145, m_Pos.y + 10);
	}
}

const POINT& SEASON3B::CNewUIMyInventory::GetPos() const
{ 
	return m_Pos; 
}

CNewUIMyInventory::REPAIR_MODE SEASON3B::CNewUIMyInventory::GetRepairMode() const
{ 
	return m_RepairMode; 
}

void SEASON3B::CNewUIMyInventory::SetRepairMode(bool bRepair)
{
	if(bRepair)
	{
		m_RepairMode = REPAIR_MODE_ON;
		if(m_pNewInventoryCtrl)
		{
			m_pNewInventoryCtrl->SetRepairMode(true);
		}
	}
	else
	{
		m_RepairMode = REPAIR_MODE_OFF;
		if(m_pNewInventoryCtrl)
		{
			m_pNewInventoryCtrl->SetRepairMode(false);
		}
	}
}

bool SEASON3B::CNewUIMyInventory::UpdateMouseEvent()
{
	if(m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->UpdateMouseEvent())
		return false;


	if(true == EquipmentWindowProcess())
		return false;
	if(true == InventoryProcess())
		return false;
	if(true == BtnProcess())
		return false;
	
	CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
	if(pPickedItem && SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(0, 0, GetScreenWidth(), 429))
	{
		if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) == true
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE) == true 
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DEVILSQUARE) == true
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE) == true
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY) == true
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHAOSGENESIS) == true
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE) ==true
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY) == true
#ifdef LEM_ADD_LUCKYITEM
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) ==true
#endif // LEM_ADD_LUCKYITEM
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY) == true )
		{
			ResetMouseLButton();
			return false;
		}

		ITEM* pItemObj = pPickedItem->GetItem();
		if( pItemObj && pItemObj->Jewel_Of_Harmony_Option != 0 )
		{
			g_pChatListBox->AddText("", GlobalText[2217], SEASON3B::TYPE_ERROR_MESSAGE);
			
			ResetMouseLButton();
			return false;
		}
		else if(pItemObj && IsHighValueItem(pItemObj) == true)
		{
			g_pChatListBox->AddText("", GlobalText[269], SEASON3B::TYPE_ERROR_MESSAGE);
			CNewUIInventoryCtrl::BackupPickedItem();
			
			ResetMouseLButton();
			return false;
		}
		else if(pItemObj && IsDropBan(pItemObj))
		{
			g_pChatListBox->AddText("", GlobalText[1915], SEASON3B::TYPE_ERROR_MESSAGE);
			CNewUIInventoryCtrl::BackupPickedItem();
			
			ResetMouseLButton();
			return false;
		}
		else if(pItemObj && pItemObj->Type == ITEM_POTION+28 && gMapManager.IsCursedTemple() == true)
		{
			ResetMouseLButton();
			return false;
		}
		else
		{
			RenderTerrain(true);
			if(RenderTerrainTile(SelectXF,SelectYF,(int)SelectXF,(int)SelectYF,1.f,1,true))
			{
				int iSourceIndex = pPickedItem->GetSourceLinealPos();
				int tx = (int)(CollisionPosition[0]/TERRAIN_SCALE);
				int ty = (int)(CollisionPosition[1]/TERRAIN_SCALE);
				if(pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
				{
					if( Hero->Dead == false )
					{
						SendRequestDropItem(MAX_EQUIPMENT_INDEX+iSourceIndex,tx,ty);
					}
				}
				else if(pItemObj && pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
				{
					SendRequestDropItem(iSourceIndex,tx,ty);
				}
				MouseUpdateTime = 0;
				MouseUpdateTimeMax = 6;

				ResetMouseLButton();
				return false;
			}
		}
	}

	g_csItemOption.SetViewOptionList(false);

	if(CheckMouseIn(m_Pos.x, m_Pos.y+20, INVENTORY_WIDTH * 0.5f, 15) == true)
	{
		g_csItemOption.SetViewOptionList(true);	
	}

	if(CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT))
	{
		if(SEASON3B::IsPress(VK_RBUTTON))
		{
			ResetMouseRButton();
			return false;
		}

		if(SEASON3B::IsNone(VK_LBUTTON) == false)
		{
			return false;
		}

		return false;
	}

	return true;
}

bool SEASON3B::CNewUIMyInventory::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY))
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			if (g_pNPCShop->IsSellingItem() == false)
			{
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY);
				PlayBuffer(SOUND_CLICK01);
			}
			if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_UNMIXSTACK))
			{
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_UNMIXSTACK);
				PlayBuffer(SOUND_CLICK01);
			}
			return false;
		}
		else if(SEASON3B::IsPress('R') == true)
		{
			if(m_bRepairEnableLevel == true && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) == false
				&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY) == false
				&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHAOSGENESIS) == false
#ifdef LEM_ADD_LUCKYITEM
				&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) == false
#endif // LEM_ADD_LUCKYITEM
				)
			{
				ToggleRepairMode();
				
				return false;
			}
		}

		if(CanOpenMyShopInterface() == true && SEASON3B::IsPress('S') )	
		{
			if( m_bMyShopOpen )
			{
				if(m_MyShopMode == MYSHOP_MODE_OPEN)
				{	
					ChangeMyShopButtonStateClose();
				}
				else if(m_MyShopMode == MYSHOP_MODE_CLOSE)
				{
					ChangeMyShopButtonStateOpen();
				}
				g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
				PlayBuffer(SOUND_CLICK01);
			}
			return false;
		}

		if(CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT) == false)
		{
			return true;
		}

		if(SEASON3B::IsRepeat(VK_CONTROL))
		{
			int iHotKey = -1;
			if(SEASON3B::IsPress('Q'))
			{
				iHotKey = HOTKEY_Q;
			}
			else if(SEASON3B::IsPress('W'))
			{
				iHotKey = HOTKEY_W;
			}
			else if(SEASON3B::IsPress('E'))
			{
				iHotKey = HOTKEY_E;
			}
			//else if(SEASON3B::IsPress('R'))
			//{
			//	iHotKey = HOTKEY_R;
			//}
			
			if(iHotKey != -1)
			{
				ITEM* pItem = m_pNewInventoryCtrl->FindItemAtPt(MouseX, MouseY);
				if(pItem == NULL)
				{
					return false;
				}

				if(CanRegisterItemHotKey(pItem->Type) == true)
				{
					int iItemLevel = (pItem->Level >> 3) & 15;
					g_pMainFrame->SetItemHotKey(iHotKey, pItem->Type, iItemLevel);
					return false;
				}

			}
		}
	}
	
	return true;
}

bool SEASON3B::CNewUIMyInventory::Update()
{
	if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
	{
		return false;
	}

	if (IsVisible())
	{
		m_iPointedSlot = -1;

		if (gVisualInventory.InventoryVisualOpen == 0)
		{
			for (int i = 0; i < NEW_EQUIPMENT_WEAPON_RIGHT; i++)
			{
				if (i >= 0 && i < 12)
				{
					if (CheckMouseIn(m_EquipmentSlots[i].x + 1, m_EquipmentSlots[i].y, m_EquipmentSlots[i].width - 4, m_EquipmentSlots[i].height - 4))
					{
						m_iPointedSlot = i;
						break;
					}
				}

				if (i >= NEW_EQUIPMENT_HELPER_SP && i <= NEW_EQUIPMENT_RING_LEFT)
				{
					if (CheckMouseIn(m_EquipmentSlots[i].x + 1, m_EquipmentSlots[i].y, m_EquipmentSlots[i].width - 4, m_EquipmentSlots[i].height - 4))
					{
						m_iPointedSlot = i;
						break;
					}
				}
			}
		}
		else
		{
			for (int j = 0; j < 9; j++)
			{
				if (CheckMouseIn(m_EquipmentSlots[j].x + 1, m_EquipmentSlots[j].y, m_EquipmentSlots[j].width - 4, m_EquipmentSlots[j].height - 4))
				{
					m_iPointedSlot = NEW_EQUIPMENT_WEAPON_RIGHT + j;
					break;
				}
			}
		}
	}

	return true;
}

bool SEASON3B::CNewUIMyInventory::UpdateVisual()
{

	return true;
}


bool SEASON3B::CNewUIMyInventory::Render()
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	RenderFrame();
	RenderInventoryDetails();
	//RenderSetOption();
	//RenderSocketOption();
	RenderButtons();

	if(m_pNewInventoryCtrl)
		m_pNewInventoryCtrl->Render();

	RenderEquippedItem();
	DisableAlphaBlend();
	return true;
}

void SEASON3B::CNewUIMyInventory::RenderSetOption()
{
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	if(g_csItemOption.GetSetOptionANum() > 0 || g_csItemOption.GetSetOptionBNum() > 0)
	{
		g_pRenderText->SetTextColor(255, 204, 25, 255);
	}
	else 
	{
		g_pRenderText->SetTextColor(100, 100, 100, 255);
	}

	unicode::t_char strText[128];
	unicode::_sprintf(strText, "[%s]", GlobalText[989]);
	g_pRenderText->RenderText(m_Pos.x + INVENTORY_WIDTH * 0.2f, m_Pos.y+25, strText, INVENTORY_WIDTH * 0.3f, 0, RT3_SORT_CENTER);

	if(g_csItemOption.IsViewOptionList() == true)
	{
		m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, -1, ITEM_SET_OPTION);
	}

}

void SEASON3B::CNewUIMyInventory::RenderSocketOption()
{
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

	if(g_SocketItemMgr.IsSocketSetOptionEnabled())
	{
		g_pRenderText->SetTextColor(255, 204, 25, 255);
	}
	else 
	{
		g_pRenderText->SetTextColor(100, 100, 100, 255);
	}

	unicode::t_char strText[128];
	unicode::_sprintf(strText, "[%s]", GlobalText[2651]);
	g_pRenderText->RenderText(m_Pos.x + INVENTORY_WIDTH * 0.5f, m_Pos.y+25, strText, INVENTORY_WIDTH * 0.3f, 0, RT3_SORT_CENTER);

	if(CheckMouseIn(m_Pos.x + INVENTORY_WIDTH * 0.5f, m_Pos.y+20, INVENTORY_WIDTH * 0.5f, 15) == true)
	{
		m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, -1, ITEM_SOCKET_SET_OPTION);
	}
}

void SEASON3B::CNewUIMyInventory::Render3DItem()
{
	for (int i = NEW_EQUIPMENT_WEAPON_RIGHT; i < MAX_NEW_EQUIPMENT_INDEX; i++)
	{
		ITEM* pEquippedItem = &CharacterMachine->Equipment[i];

		if (pEquippedItem)
		{
			if ((i != 5 || gCharacterManager.GetBaseClass((Hero)->Class) != 6))
			{
				glColor4f(1.0, 1.0, 1.0, 1.0);

				if (pEquippedItem->Type >= 0)
				{
					float y = 0.f;
					if (i == NEW_EQUIPMENT_ARMOR)
					{
						y = m_EquipmentSlots[i].y - 10.f;
					}
					else
					{
						y = m_EquipmentSlots[i].y;
					}

					glColor4f(1.f, 1.f, 1.f, 1.f);
					RenderItem3D(
						m_EquipmentSlots[i].x + 1,
						y,
						m_EquipmentSlots[i].width - 4,
						m_EquipmentSlots[i].height - 4,
						pEquippedItem->Type,
						pEquippedItem->Level,
						pEquippedItem->Option1,
						pEquippedItem->ExtOption,
						false);
				}
			}
		}
	}
}

bool SEASON3B::CNewUIMyInventory::CheckRenderItensVisual()
{
	if (gVisualInventory.InventoryVisualOpen == 1)
	{
		Render3DItem();
		return 0;
	}

	return 1;
}

void SEASON3B::CNewUIMyInventory::Render3D()
{
	if (gVisualInventory.InventoryVisualOpen == 1)
	{
		for (int i = NEW_EQUIPMENT_WEAPON_RIGHT; i < MAX_NEW_EQUIPMENT_INDEX; i++)
		{
			ITEM* pEquippedItem = &CharacterMachine->Equipment[i];

			if (pEquippedItem->Type >= 0)
			{
				float y = 0.f;

				y = m_EquipmentSlots[i].y;

				glColor4f(1.f, 1.f, 1.f, 1.f);
				RenderItem3D(
					m_EquipmentSlots[i].x + 1,
					y,
					m_EquipmentSlots[i].width - 4,
					m_EquipmentSlots[i].height - 4,
					pEquippedItem->Type,
					pEquippedItem->Level,
					pEquippedItem->Option1,
					pEquippedItem->ExtOption,
					false);
			}
		}

	}
	else
	{
		for (int i = 0; i < MAX_EQUIPMENT_INDEX; i++)
		{
			ITEM* pEquippedItem = &CharacterMachine->Equipment[i];
			if (pEquippedItem->Type >= 0)
			{
				float y = 0.f;
				if (i == EQUIPMENT_ARMOR)
				{
					y = m_EquipmentSlots[i].y - 10.f;
				}
				else
				{
					y = m_EquipmentSlots[i].y;
				}

				glColor4f(1.f, 1.f, 1.f, 1.f);
				RenderItem3D(
					m_EquipmentSlots[i].x + 1,
					y,
					m_EquipmentSlots[i].width - 4,
					m_EquipmentSlots[i].height - 4,
					pEquippedItem->Type,
					pEquippedItem->Level,
					pEquippedItem->Option1,
					pEquippedItem->ExtOption,
					false);
			}
		}

		for (int i = NEW_EQUIPMENT_HELPER_SP; i < NEW_EQUIPMENT_WEAPON_RIGHT; i++)
		{
			ITEM* pEquippedItem = &CharacterMachine->Equipment[i];

			if (pEquippedItem->Type >= 0)
			{
				float y = 0.f;
				y = m_EquipmentSlots[i].y;			

				glColor4f(1.f, 1.f, 1.f, 1.f);
				RenderItem3D(
					m_EquipmentSlots[i].x + 1,
					y,
					m_EquipmentSlots[i].width - 4,
					m_EquipmentSlots[i].height - 4,
					pEquippedItem->Type,
					pEquippedItem->Level,
					pEquippedItem->Option1,
					pEquippedItem->ExtOption,
					false);
			}
		}
	}

}

bool SEASON3B::CNewUIMyInventory::IsVisible() const
{ 
	return CNewUIObj::IsVisible(); 
}

void SEASON3B::CNewUIMyInventory::OpenningProcess()
{
	SetRepairMode(false);

	m_MyShopMode = MYSHOP_MODE_OPEN;
	ChangeMyShopButtonStateOpen();

	WORD wLevel = CharacterAttribute->Level;

	if(wLevel >= 50)
	{
		m_bRepairEnableLevel = true;
	}
	else
	{
		m_bRepairEnableLevel = false;	
	}
	
	if(wLevel >= 6)
	{
		m_bMyShopOpen = true;
	}
	else
	{
		m_bMyShopOpen = false;
	}

	if (g_QuestMng.IsIndexInCurQuestIndexList(0x1000F))
	{
		if (g_QuestMng.IsEPRequestRewardState(0x1000F))
		{
			SendSatisfyQuestRequestFromClient(0x1000F);
			g_QuestMng.SetEPRequestRewardState(0x1000F, false);	
		}
	}
}

void SEASON3B::CNewUIMyInventory::ClosingProcess()
{
	m_pNewInventoryCtrl->BackupPickedItem();

	RepairEnable = 0;

	SetRepairMode(false);

	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_UNMIXSTACK))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_UNMIXSTACK);
	}
}

float SEASON3B::CNewUIMyInventory::GetLayerDepth() 
{ 
	return 4.2f; 
}

CNewUIInventoryCtrl* SEASON3B::CNewUIMyInventory::GetInventoryCtrl() const
{ 
	return m_pNewInventoryCtrl; 
}

ITEM* SEASON3B::CNewUIMyInventory::FindItem(int iLinealPos)
{
	if(m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->FindItem(iLinealPos);
	return NULL;
}

ITEM* SEASON3B::CNewUIMyInventory::FindItemByKey(DWORD dwKey)
{
	if(m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->FindItemByKey(dwKey);
	return NULL;
}

int SEASON3B::CNewUIMyInventory::FindItemIndex( short int siType, int iLevel )
{
	if(m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->FindItemIndex( siType, iLevel );
	return -1;
}

int SEASON3B::CNewUIMyInventory::FindItemReverseIndex(short sType, int iLevel)
{
	if(m_pNewInventoryCtrl)
	{
		return m_pNewInventoryCtrl->FindItemReverseIndex( sType, iLevel );
	}

	return -1;
}

int SEASON3B::CNewUIMyInventory::FindEmptySlot(IN int cx, IN int cy)
{
	if(m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->FindEmptySlot(cx, cy);
	return -1;
}

int SEASON3B::CNewUIMyInventory::FindEmptySlot(ITEM* pItem)
{
	ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
	if(m_pNewInventoryCtrl)
	{
		return m_pNewInventoryCtrl->FindEmptySlot(pItemAttr->Width, pItemAttr->Height);
	}

	return -1;
}

void SEASON3B::CNewUIMyInventory::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
	if(pClass)
	{
		CNewUIMyInventory* pMyInventory = (CNewUIMyInventory*)(pClass);

		if(dwParamB == ITEM_SET_OPTION)
		{
			g_csItemOption.RenderSetOptionList(pMyInventory->GetPos().x, pMyInventory->GetPos().y);
		}
		else if (dwParamB == ITEM_SOCKET_SET_OPTION)
		{
			g_SocketItemMgr.RenderToolTipForSocketSetOption(pMyInventory->GetPos().x, pMyInventory->GetPos().y);
		}
		else
		{
			pMyInventory->RenderItemToolTip(dwParamA);
		}
	}
}

void SEASON3B::CNewUIMyInventory::SetStandbyItemKey(DWORD dwItemKey)
{ 
	m_dwStandbyItemKey = dwItemKey; 
}

DWORD SEASON3B::CNewUIMyInventory::GetStandbyItemKey() const
{ 
	return m_dwStandbyItemKey; 
}

int SEASON3B::CNewUIMyInventory::GetStandbyItemIndex()
{
	ITEM* pItem = GetStandbyItem();
	if(pItem)
		return pItem->y*m_pNewInventoryCtrl->GetNumberOfColumn()+pItem->x;
	return -1;
}

ITEM* SEASON3B::CNewUIMyInventory::GetStandbyItem()
{
	if(m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->FindItemByKey(m_dwStandbyItemKey);
	return NULL;
}

void SEASON3B::CNewUIMyInventory::CreateEquippingEffect(ITEM* pItem)
{
	SetCharacterClass(Hero);
	OBJECT* pHeroObject = &Hero->Object;
	if(false == gMapManager.InChaosCastle())
	{
		if (gHelperSystem.CheckIsHelper(pItem->Type + MODEL_ITEM))
		{
			gHelperSystem.InvCreateEquippingEffect(pItem->Type);
		}

		switch(pItem->Type)
		{
		case ITEM_HELPER:
			CreateBug(MODEL_HELPER, pHeroObject->Position, pHeroObject);
			break;
		case ITEM_HELPER+2:
			CreateBug(MODEL_UNICON, pHeroObject->Position, pHeroObject);
			if(!Hero->SafeZone)
				CreateEffect(BITMAP_MAGIC+1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
			break;
		case ITEM_HELPER+3:
			CreateBug(MODEL_PEGASUS, pHeroObject->Position, pHeroObject);
			if(!Hero->SafeZone)
				CreateEffect(BITMAP_MAGIC+1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
			break;
		case ITEM_HELPER+4:
			CreateBug(MODEL_DARK_HORSE, pHeroObject->Position, pHeroObject);
			if(!Hero->SafeZone)
				CreateEffect(BITMAP_MAGIC+1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
			break;
		case ITEM_HELPER+37:
			Hero->Helper.Option1 = pItem->Option1;
			if(pItem->Option1 == 0x01)
			{
				CreateBug(MODEL_FENRIR_BLACK, pHeroObject->Position, pHeroObject);
			}
			else if(pItem->Option1 == 0x02)
			{
				CreateBug(MODEL_FENRIR_BLUE, pHeroObject->Position, pHeroObject);
			}
			else if(pItem->Option1 == 0x04)
			{
				CreateBug(MODEL_FENRIR_GOLD, pHeroObject->Position, pHeroObject);
			}
			else
			{
				CreateBug(MODEL_FENRIR_RED, pHeroObject->Position, pHeroObject);
			}

			if(!Hero->SafeZone)
			{
				CreateEffect(BITMAP_MAGIC+1,pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
			}
			break;
		case ITEM_HELPER+64:
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER+64, pHeroObject->Position, Hero);
			break;
		case ITEM_HELPER+65:
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER+65, pHeroObject->Position, Hero);
			break;
		case ITEM_HELPER+67:
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER+67, pHeroObject->Position, Hero);
			break;
		case ITEM_HELPER+80:
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER+80, pHeroObject->Position, Hero);
			break;
		case ITEM_HELPER+106:
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER+106, pHeroObject->Position, Hero);
			break;
		case ITEM_HELPER+123:
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER+123, pHeroObject->Position, Hero);
			break;
		}
	}
	if(Hero->EtcPart<=0 || Hero->EtcPart>3)
	{
		if ( pItem->Type==ITEM_HELPER+20 && (pItem->Level>>3)==3 )
		{
			DeleteParts ( Hero );
			Hero->EtcPart = PARTS_LION;
		}
	}
	if (pItem->Type == ITEM_WING + 39 || pItem->Type == ITEM_HELPER + 30 || pItem->Type == ITEM_WING + 130 || pItem->Type == ITEM_WING + 40 || gCustomWing.CheckCustomWingIsCape(pItem->Type + MODEL_ITEM) > 0)
	{
		DeleteCloth(Hero, &Hero->Object);
	}
}

void SEASON3B::CNewUIMyInventory::DeleteEquippingEffectBug(ITEM* pItem)
{
	if( ThePetProcess().IsPet(pItem->Type) == true )
	{
		ThePetProcess().DeletePet( Hero, pItem->Type );
	}
	if (gHelperSystem.CheckIsHelper(pItem->Type + MODEL_ITEM))
	{
		gHelperManager.DeleteHelper(Hero, pItem->Type);
	}

	switch(pItem->Type)
	{
	case ITEM_HELPER+30:
	case ITEM_WING+39:	
	case ITEM_WING+40:
	case ITEM_WING+130:
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	case ITEM_WING+49:
	case ITEM_WING+50:
	case ITEM_WING+135:
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		DeleteCloth(Hero, &Hero->Object);
		return;
	}

	if (gCustomWing.CheckCustomWingIsCape(pItem->Type + MODEL_ITEM) > 0) 
	{
		DeleteCloth(Hero, &Hero->Object);
		return;
	}


	if(IsBug(pItem) == true)
	{
		DeleteBug(&Hero->Object);
	}
}

void SEASON3B::CNewUIMyInventory::DeleteEquippingEffect()
{
	if(Hero->EtcPart < PARTS_ATTACK_TEAM_MARK)
	{
		DeleteParts(Hero);
		if(Hero->EtcPart > 3)
		{
			Hero->EtcPart = 0;
		}
	}

	SetCharacterClass(Hero);
}

void SEASON3B::CNewUIMyInventory::SetEquipmentSlotInfo()
{
	m_EquipmentSlots[EQUIPMENT_HELPER].x = m_Pos.x+15;
	m_EquipmentSlots[EQUIPMENT_HELPER].y = m_Pos.y+44;
	m_EquipmentSlots[EQUIPMENT_HELPER].width = 46;
	m_EquipmentSlots[EQUIPMENT_HELPER].height = 46;
	m_EquipmentSlots[EQUIPMENT_HELPER].dwBgImage = IMAGE_INVENTORY_ITEM_FAIRY;

	m_EquipmentSlots[EQUIPMENT_HELM].x = m_Pos.x+75;
	m_EquipmentSlots[EQUIPMENT_HELM].y = m_Pos.y+44;
	m_EquipmentSlots[EQUIPMENT_HELM].width = 46;
	m_EquipmentSlots[EQUIPMENT_HELM].height = 46;
	m_EquipmentSlots[EQUIPMENT_HELM].dwBgImage = IMAGE_INVENTORY_ITEM_HELM;

	m_EquipmentSlots[EQUIPMENT_WING].x = m_Pos.x+120;
	m_EquipmentSlots[EQUIPMENT_WING].y = m_Pos.y+44;
	m_EquipmentSlots[EQUIPMENT_WING].width = 61;
	m_EquipmentSlots[EQUIPMENT_WING].height = 46;
	m_EquipmentSlots[EQUIPMENT_WING].dwBgImage = IMAGE_INVENTORY_ITEM_WING;

	m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].x = m_Pos.x+135;
	m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].y = m_Pos.y+87;
	m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].width = 46;
	m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].height = 66;
	m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].dwBgImage = IMAGE_INVENTORY_ITEM_LEFT;

	m_EquipmentSlots[EQUIPMENT_ARMOR].x = m_Pos.x+75;
	m_EquipmentSlots[EQUIPMENT_ARMOR].y = m_Pos.y+87;
	m_EquipmentSlots[EQUIPMENT_ARMOR].width = 46;
	m_EquipmentSlots[EQUIPMENT_ARMOR].height = 66;
	m_EquipmentSlots[EQUIPMENT_ARMOR].dwBgImage = IMAGE_INVENTORY_ITEM_ARMOR;

	m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].x = m_Pos.x+15;
	m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].y = m_Pos.y+87;
	m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].width = 46;
	m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].height = 66;
	m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].dwBgImage = IMAGE_INVENTORY_ITEM_RIGHT;

	m_EquipmentSlots[EQUIPMENT_GLOVES].x = m_Pos.x+15;
	m_EquipmentSlots[EQUIPMENT_GLOVES].y = m_Pos.y+150;
	m_EquipmentSlots[EQUIPMENT_GLOVES].width = 46;
	m_EquipmentSlots[EQUIPMENT_GLOVES].height = 46;
	m_EquipmentSlots[EQUIPMENT_GLOVES].dwBgImage = IMAGE_INVENTORY_ITEM_GLOVES;

	m_EquipmentSlots[EQUIPMENT_PANTS].x = m_Pos.x+75;
	m_EquipmentSlots[EQUIPMENT_PANTS].y = m_Pos.y+150;
	m_EquipmentSlots[EQUIPMENT_PANTS].width = 46;
	m_EquipmentSlots[EQUIPMENT_PANTS].height = 46;
	m_EquipmentSlots[EQUIPMENT_PANTS].dwBgImage = IMAGE_INVENTORY_ITEM_PANTS;

	m_EquipmentSlots[EQUIPMENT_BOOTS].x = m_Pos.x + 135;
	m_EquipmentSlots[EQUIPMENT_BOOTS].y = m_Pos.y + 150;
	m_EquipmentSlots[EQUIPMENT_BOOTS].width = 46;
	m_EquipmentSlots[EQUIPMENT_BOOTS].height = 46;
	m_EquipmentSlots[EQUIPMENT_BOOTS].dwBgImage = IMAGE_INVENTORY_ITEM_BOOT;

	m_EquipmentSlots[EQUIPMENT_RING_LEFT].x = m_Pos.x + 119;
	m_EquipmentSlots[EQUIPMENT_RING_LEFT].y = m_Pos.y + 151;
	m_EquipmentSlots[EQUIPMENT_RING_LEFT].width = 20;
	m_EquipmentSlots[EQUIPMENT_RING_LEFT].height = 20;
	m_EquipmentSlots[EQUIPMENT_RING_LEFT].dwBgImage = IMAGE_INVENTORY_ITEM_RING;

	m_EquipmentSlots[EQUIPMENT_AMULET].x = m_Pos.x + 59;
	m_EquipmentSlots[EQUIPMENT_AMULET].y = m_Pos.y + 88;
	m_EquipmentSlots[EQUIPMENT_AMULET].width = 20;
	m_EquipmentSlots[EQUIPMENT_AMULET].height = 20;
	m_EquipmentSlots[EQUIPMENT_AMULET].dwBgImage = IMAGE_INVENTORY_ITEM_NECKLACE;

	m_EquipmentSlots[EQUIPMENT_RING_RIGHT].x = m_Pos.x + 59;
	m_EquipmentSlots[EQUIPMENT_RING_RIGHT].y = m_Pos.y + 151;
	m_EquipmentSlots[EQUIPMENT_RING_RIGHT].width = 20;
	m_EquipmentSlots[EQUIPMENT_RING_RIGHT].height = 20;
	m_EquipmentSlots[EQUIPMENT_RING_RIGHT].dwBgImage = IMAGE_INVENTORY_ITEM_RING;
	
	//	- Element Slot

	for (int i = NEW_EQUIPMENT_HELPER_SP; i < NEW_EQUIPMENT_WEAPON_RIGHT; i++)
	{
		m_EquipmentSlots[i].dwBgImage = -1;
	}

	if (gControlSlot.m_ElementEquiped[0].Active == 1)
	{
		m_EquipmentSlots[NEW_EQUIPMENT_HELPER_SP].x				= m_Pos.x + gControlSlot.m_ElementEquiped[0].x;
		m_EquipmentSlots[NEW_EQUIPMENT_HELPER_SP].y				= m_Pos.y + gControlSlot.m_ElementEquiped[0].y;
		m_EquipmentSlots[NEW_EQUIPMENT_HELPER_SP].width			= gControlSlot.m_ElementEquiped[0].width;
		m_EquipmentSlots[NEW_EQUIPMENT_HELPER_SP].height		= gControlSlot.m_ElementEquiped[0].height;
		m_EquipmentSlots[NEW_EQUIPMENT_HELPER_SP].dwBgImage		= 65535;
	}

	if (gControlSlot.m_ElementEquiped[1].Active == 1)
	{
		m_EquipmentSlots[NEW_EQUIPMENT_HELPER_SUP].x			= m_Pos.x + gControlSlot.m_ElementEquiped[1].x;
		m_EquipmentSlots[NEW_EQUIPMENT_HELPER_SUP].y			= m_Pos.y + gControlSlot.m_ElementEquiped[1].y;
		m_EquipmentSlots[NEW_EQUIPMENT_HELPER_SUP].width		= gControlSlot.m_ElementEquiped[1].width;
		m_EquipmentSlots[NEW_EQUIPMENT_HELPER_SUP].height		= gControlSlot.m_ElementEquiped[1].height;
		m_EquipmentSlots[NEW_EQUIPMENT_HELPER_SUP].dwBgImage	= 65536;
	}

	if (gControlSlot.m_ElementEquiped[2].Active == 1)
	{
		m_EquipmentSlots[NEW_EQUIPMENT_RING_LEFT].x				= m_Pos.x + gControlSlot.m_ElementEquiped[2].x;
		m_EquipmentSlots[NEW_EQUIPMENT_RING_LEFT].y				= m_Pos.y + gControlSlot.m_ElementEquiped[2].y;
		m_EquipmentSlots[NEW_EQUIPMENT_RING_LEFT].width			= gControlSlot.m_ElementEquiped[2].width;
		m_EquipmentSlots[NEW_EQUIPMENT_RING_LEFT].height		= gControlSlot.m_ElementEquiped[2].height;
		m_EquipmentSlots[NEW_EQUIPMENT_RING_LEFT].dwBgImage		= 65537;
	}

	if (gControlSlot.m_ElementEquiped[3].Active == 1)
	{
		m_EquipmentSlots[NEW_EQUIPMENT_RING_RIGHT].x			= m_Pos.x + gControlSlot.m_ElementEquiped[3].x;
		m_EquipmentSlots[NEW_EQUIPMENT_RING_RIGHT].y			= m_Pos.y + gControlSlot.m_ElementEquiped[3].y;
		m_EquipmentSlots[NEW_EQUIPMENT_RING_RIGHT].width		= gControlSlot.m_ElementEquiped[3].width;
		m_EquipmentSlots[NEW_EQUIPMENT_RING_RIGHT].height		= gControlSlot.m_ElementEquiped[3].height;
		m_EquipmentSlots[NEW_EQUIPMENT_RING_RIGHT].dwBgImage	= 65537;
	}

	//	- Inventário Visual
	m_EquipmentSlots[NEW_EQUIPMENT_HELPER].x = m_Pos.x + 15;
	m_EquipmentSlots[NEW_EQUIPMENT_HELPER].y = m_Pos.y + 44;
	m_EquipmentSlots[NEW_EQUIPMENT_HELPER].width = 46;
	m_EquipmentSlots[NEW_EQUIPMENT_HELPER].height = 46;
	m_EquipmentSlots[NEW_EQUIPMENT_HELPER].dwBgImage = IMAGE_INVENTORY_ITEM_FAIRY;

	m_EquipmentSlots[NEW_EQUIPMENT_HELM].x = m_Pos.x + 75;
	m_EquipmentSlots[NEW_EQUIPMENT_HELM].y = m_Pos.y + 44;
	m_EquipmentSlots[NEW_EQUIPMENT_HELM].width = 46;
	m_EquipmentSlots[NEW_EQUIPMENT_HELM].height = 46;
	m_EquipmentSlots[NEW_EQUIPMENT_HELM].dwBgImage = IMAGE_INVENTORY_ITEM_HELM;

	m_EquipmentSlots[NEW_EQUIPMENT_WING].x = m_Pos.x + 120;
	m_EquipmentSlots[NEW_EQUIPMENT_WING].y = m_Pos.y + 44;
	m_EquipmentSlots[NEW_EQUIPMENT_WING].width = 61;
	m_EquipmentSlots[NEW_EQUIPMENT_WING].height = 46;
	m_EquipmentSlots[NEW_EQUIPMENT_WING].dwBgImage = IMAGE_INVENTORY_ITEM_WING;

	m_EquipmentSlots[NEW_EQUIPMENT_WEAPON_LEFT].x = m_Pos.x + 135;
	m_EquipmentSlots[NEW_EQUIPMENT_WEAPON_LEFT].y = m_Pos.y + 87;
	m_EquipmentSlots[NEW_EQUIPMENT_WEAPON_LEFT].width = 46;
	m_EquipmentSlots[NEW_EQUIPMENT_WEAPON_LEFT].height = 66;
	m_EquipmentSlots[NEW_EQUIPMENT_WEAPON_LEFT].dwBgImage = IMAGE_INVENTORY_ITEM_LEFT;

	m_EquipmentSlots[NEW_EQUIPMENT_ARMOR].x = m_Pos.x + 75;
	m_EquipmentSlots[NEW_EQUIPMENT_ARMOR].y = m_Pos.y + 87;
	m_EquipmentSlots[NEW_EQUIPMENT_ARMOR].width = 46;
	m_EquipmentSlots[NEW_EQUIPMENT_ARMOR].height = 66;
	m_EquipmentSlots[NEW_EQUIPMENT_ARMOR].dwBgImage = IMAGE_INVENTORY_ITEM_ARMOR;

	m_EquipmentSlots[NEW_EQUIPMENT_WEAPON_RIGHT].x = m_Pos.x + 15;
	m_EquipmentSlots[NEW_EQUIPMENT_WEAPON_RIGHT].y = m_Pos.y + 87;
	m_EquipmentSlots[NEW_EQUIPMENT_WEAPON_RIGHT].width = 46;
	m_EquipmentSlots[NEW_EQUIPMENT_WEAPON_RIGHT].height = 66;
	m_EquipmentSlots[NEW_EQUIPMENT_WEAPON_RIGHT].dwBgImage = IMAGE_INVENTORY_ITEM_RIGHT;

	m_EquipmentSlots[NEW_EQUIPMENT_GLOVES].x = m_Pos.x + 15;
	m_EquipmentSlots[NEW_EQUIPMENT_GLOVES].y = m_Pos.y + 150;
	m_EquipmentSlots[NEW_EQUIPMENT_GLOVES].width = 46;
	m_EquipmentSlots[NEW_EQUIPMENT_GLOVES].height = 46;
	m_EquipmentSlots[NEW_EQUIPMENT_GLOVES].dwBgImage = IMAGE_INVENTORY_ITEM_GLOVES;

	m_EquipmentSlots[NEW_EQUIPMENT_PANTS].x = m_Pos.x + 75;
	m_EquipmentSlots[NEW_EQUIPMENT_PANTS].y = m_Pos.y + 150;
	m_EquipmentSlots[NEW_EQUIPMENT_PANTS].width = 46;
	m_EquipmentSlots[NEW_EQUIPMENT_PANTS].height = 46;
	m_EquipmentSlots[NEW_EQUIPMENT_PANTS].dwBgImage = IMAGE_INVENTORY_ITEM_PANTS;

	m_EquipmentSlots[NEW_EQUIPMENT_BOOTS].x = m_Pos.x + 135;
	m_EquipmentSlots[NEW_EQUIPMENT_BOOTS].y = m_Pos.y + 150;
	m_EquipmentSlots[NEW_EQUIPMENT_BOOTS].width = 46;
	m_EquipmentSlots[NEW_EQUIPMENT_BOOTS].height = 46;
	m_EquipmentSlots[NEW_EQUIPMENT_BOOTS].dwBgImage = IMAGE_INVENTORY_ITEM_BOOT;
}

void SEASON3B::CNewUIMyInventory::SetButtonInfo()
{
	m_BtnExit.ChangeButtonImgState(true, IMAGE_INVENTORY_EXIT_BTN, false);
	m_BtnExit.ChangeButtonInfo(m_Pos.x+13, m_Pos.y+392, 36, 29);
	m_BtnExit.ChangeToolTipText(GlobalText[225], true);

	m_BtnRepair.ChangeButtonImgState(true, IMAGE_INVENTORY_REPAIR_BTN, false);
	m_BtnRepair.ChangeButtonInfo(m_Pos.x+45, m_Pos.y+392, 36, 29);
	m_BtnRepair.ChangeToolTipText(GlobalText[233], true);

	m_BtnMyShop.ChangeButtonImgState(true, IMAGE_INVENTORY_MYSHOP_OPEN_BTN, false);
	m_BtnMyShop.ChangeButtonInfo(m_Pos.x+77, m_Pos.y+392, 36, 29);

	if (Visuals.VisualSwitch == 1)
	{
		m_BtnPrev.ChangeButtonImgState(true, 33028, true, false, false);
		m_BtnPrev.ChangeButtonInfo(m_Pos.x + 20, m_Pos.y + 10, 25, 24);

		m_BtnNext.ChangeButtonImgState(true, 33029, true, false, false);
		m_BtnNext.ChangeButtonInfo(m_Pos.x + 145, m_Pos.y + 10, 25, 24);
	}

	m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);
}

void SEASON3B::CNewUIMyInventory::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_INVENTORY_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_INVENTORY_BACK_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back04.tga", IMAGE_INVENTORY_BACK_TOP2, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_INVENTORY_BACK_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_INVENTORY_BACK_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_INVENTORY_BACK_BOTTOM, GL_LINEAR);

#ifdef ORIGIN_INTERFACE
	LoadBitmap("Interface\\newui_item_boots.tga", IMAGE_INVENTORY_ITEM_BOOT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_cap.tga", IMAGE_INVENTORY_ITEM_HELM, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_fairy.tga", IMAGE_INVENTORY_ITEM_FAIRY, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_wing.tga", IMAGE_INVENTORY_ITEM_WING, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_weapon(L).tga", IMAGE_INVENTORY_ITEM_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_weapon(R).tga", IMAGE_INVENTORY_ITEM_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_upper.tga", IMAGE_INVENTORY_ITEM_ARMOR, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_gloves.tga", IMAGE_INVENTORY_ITEM_GLOVES, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_lower.tga", IMAGE_INVENTORY_ITEM_PANTS, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_ring.tga", IMAGE_INVENTORY_ITEM_RING, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_necklace.tga", IMAGE_INVENTORY_ITEM_NECKLACE, GL_LINEAR);
#endif
	LoadBitmap("Custom\\NewInterface\\item_boots_v2.jpg", IMAGE_INVENTORY_ITEM_BOOT, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\item_cap_v2.jpg", IMAGE_INVENTORY_ITEM_HELM, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\item_fairy_v2.jpg", IMAGE_INVENTORY_ITEM_FAIRY, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\item_wing_v2.jpg", IMAGE_INVENTORY_ITEM_WING, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\item_weapon01_v2.jpg", IMAGE_INVENTORY_ITEM_RIGHT, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\item_weapon02_v2.jpg", IMAGE_INVENTORY_ITEM_LEFT, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\item_upper_v2.jpg", IMAGE_INVENTORY_ITEM_ARMOR, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\item_gloves_v2.jpg", IMAGE_INVENTORY_ITEM_GLOVES, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\item_lower_v2.jpg", IMAGE_INVENTORY_ITEM_PANTS, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\item_ring_v2.jpg", IMAGE_INVENTORY_ITEM_RING, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\item_necklace_v2.jpg", IMAGE_INVENTORY_ITEM_NECKLACE, GL_LINEAR);
	//
	LoadBitmap("Custom\\NewInterface\\item_elementslot2_v2.jpg", 65535, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\item_elementslot3_v2.jpg", 65536, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\item_earring_v2.jpg", 65537, GL_LINEAR);
	//
	LoadBitmap("Custom\\NewInterface\\btn_previous.jpg", 33028, GL_LINEAR, GL_REPEAT);
	LoadBitmap("Custom\\NewInterface\\btn_next.jpg", 33029, GL_LINEAR, GL_REPEAT);

	LoadBitmap("Interface\\newui_item_money.tga", IMAGE_INVENTORY_MONEY, GL_LINEAR);
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_INVENTORY_EXIT_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_repair_00.tga", IMAGE_INVENTORY_REPAIR_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_Bt_openshop.tga", IMAGE_INVENTORY_MYSHOP_OPEN_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_Bt_closeshop.tga", IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, GL_LINEAR);
	//
	LoadBitmap("Custom\\NewInterface\\item_back01_v2.jpg", 931300, GL_LINEAR, GL_CLAMP);
	LoadBitmap("Custom\\NewInterface\\item_back02_v2.jpg", 931301, GL_LINEAR, GL_CLAMP);
	LoadBitmap("Custom\\NewInterface\\item_back02_v2.tga", 931302, GL_LINEAR, GL_CLAMP);
	LoadBitmap("Custom\\NewInterface\\btn_mix_v2.jpg", 931309, GL_LINEAR, GL_CLAMP, 1, 0);
	LoadBitmap("Custom\\NewInterface\\btn_deposit_v2.jpg", 931310, GL_LINEAR, GL_CLAMP, 1, 0);
	LoadBitmap("Custom\\NewInterface\\btn_withdraw_v2.jpg", 931311, GL_LINEAR, GL_CLAMP, 1, 0);
	LoadBitmap("Custom\\NewInterface\\btn_lock_v2.jpg", 931312, GL_LINEAR, GL_CLAMP, 1, 0);
	LoadBitmap("Custom\\NewInterface\\btn_lock2_v2.jpg", 931313, GL_LINEAR, GL_CLAMP, 1, 0);
	LoadBitmap("Custom\\NewInterface\\btn_closestore_v2.jpg", 931314, GL_LINEAR);
	LoadBitmap("Custom\\NewInterface\\item_box_v2.jpg", 931315, GL_LINEAR, GL_CLAMP);
	LoadBitmap("Custom\\NewInterface\\item_money_v2.tga", 931316, GL_LINEAR, GL_CLAMP);
	LoadBitmap("Custom\\NewInterface\\btn_reload2.jpg", 931317, GL_LINEAR, GL_CLAMP);
}

void SEASON3B::CNewUIMyInventory::UnloadImages()
{
	DeleteBitmap(65535);
	DeleteBitmap(65536);
	DeleteBitmap(65537);
	DeleteBitmap(931309);
	DeleteBitmap(931310);
	DeleteBitmap(931311);
	DeleteBitmap(931312);
	DeleteBitmap(931313);
	DeleteBitmap(931314);

	DeleteBitmap(33028);
	DeleteBitmap(33029);
	//
	DeleteBitmap(931300);
	DeleteBitmap(931301);
	DeleteBitmap(931302);
	DeleteBitmap(931315);
	DeleteBitmap(931316);
	DeleteBitmap(931317);
	//
	DeleteBitmap(IMAGE_INVENTORY_MYSHOP_CLOSE_BTN);
	DeleteBitmap(IMAGE_INVENTORY_MYSHOP_OPEN_BTN);
	DeleteBitmap(IMAGE_INVENTORY_REPAIR_BTN);
	DeleteBitmap(IMAGE_INVENTORY_EXIT_BTN);
	DeleteBitmap(IMAGE_INVENTORY_MONEY);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_NECKLACE);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_RING);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_PANTS);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_GLOVES);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_ARMOR);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_RIGHT);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_LEFT);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_WING);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_FAIRY);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_HELM);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_BOOT);
	DeleteBitmap(IMAGE_INVENTORY_BACK_BOTTOM);
	DeleteBitmap(IMAGE_INVENTORY_BACK_RIGHT);
	DeleteBitmap(IMAGE_INVENTORY_BACK_LEFT);
	DeleteBitmap(IMAGE_INVENTORY_BACK_TOP2);
	DeleteBitmap(IMAGE_INVENTORY_BACK_TOP);
	DeleteBitmap(IMAGE_INVENTORY_BACK);
}

void SEASON3B::CNewUIMyInventory::RenderFrame()
{
	RenderImage(IMAGE_INVENTORY_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
	RenderImage(IMAGE_INVENTORY_BACK_TOP2, m_Pos.x, m_Pos.y, 190.f, 64.f);
	RenderImage(IMAGE_INVENTORY_BACK_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_INVENTORY_BACK_RIGHT, m_Pos.x+INVENTORY_WIDTH-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_INVENTORY_BACK_BOTTOM, m_Pos.x, m_Pos.y+INVENTORY_HEIGHT-45, 190.f, 45.f);
}

void SEASON3B::CNewUIMyInventory::RenderEquippedItem()
{
	if (gVisualInventory.InventoryVisualOpen == 0)
	{
		for (int i = NEW_EQUIPMENT_HELPER_SP; i < NEW_EQUIPMENT_WEAPON_RIGHT; i++)
		{
			EnableAlphaTest();

			RenderImageA(m_EquipmentSlots[i].dwBgImage, m_EquipmentSlots[i].x, m_EquipmentSlots[i].y, m_EquipmentSlots[i].width, m_EquipmentSlots[i].height);

			DisableAlphaBlend();
		}
	}

	for (int i = 0; i < gVisualInventory.InventoryVisualSlot; i++)
	{
		if(i == EQUIPMENT_HELM)
		{
			if(gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK)
			{
				continue;
			}
		}

		EnableAlphaTest();
		
		RenderImageA(m_EquipmentSlots[i].dwBgImage, m_EquipmentSlots[i].x, m_EquipmentSlots[i].y, m_EquipmentSlots[i].width, m_EquipmentSlots[i].height);

		DisableAlphaBlend();

		ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[i];
		if(pEquipmentItemSlot->Type != -1)
		{
			ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pEquipmentItemSlot->Type];
			int iLevel = (pEquipmentItemSlot->Level>>3)&15;
			int iMaxDurability = calcMaxDurability(pEquipmentItemSlot, pItemAttr, iLevel);
			
			if( i == EQUIPMENT_RING_LEFT || i == EQUIPMENT_RING_RIGHT)
			{
				if( pEquipmentItemSlot->Type == ITEM_HELPER+20 && iLevel == 1 
					|| iLevel == 2 )
				{
					continue;
				}
			}
			
			if( (pEquipmentItemSlot->bPeriodItem == true) && (pEquipmentItemSlot->bExpiredPeriod == false) )
				continue;
			
			if(pEquipmentItemSlot->Durability <= 0)
				glColor4f(1.f,0.f,0.f,0.25f);
			else if(pEquipmentItemSlot->Durability<=(iMaxDurability*0.2f))
				glColor4f(1.f,0.15f,0.f,0.25f);
			else if(pEquipmentItemSlot->Durability<=(iMaxDurability*0.3f))
				glColor4f(1.f,0.5f,0.f,0.25f);
			else if(pEquipmentItemSlot->Durability<=(iMaxDurability*0.5f))
				glColor4f(1.f,1.f,0.f,0.25f);
			else if(IsEquipable(i, pEquipmentItemSlot) == false)
				glColor4f(1.f,0.f,0.f,0.25f);
			else
			{
				continue;
			}
			
			EnableAlphaTest();
			RenderColor(m_EquipmentSlots[i].x+1, m_EquipmentSlots[i].y, m_EquipmentSlots[i].width-4, m_EquipmentSlots[i].height-4);
			EndRenderColor();
		}
	}

	if(CNewUIInventoryCtrl::GetPickedItem() && m_iPointedSlot != -1)
	{
		if (m_iPointedSlot > 12 && m_iPointedSlot < 236)
		{
			return;
		}

		ITEM* pItemObj = CNewUIInventoryCtrl::GetPickedItem()->GetItem();
		ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[m_iPointedSlot];
		if( pItemObj && (pEquipmentItemSlot->Type != -1 || false == IsEquipable(m_iPointedSlot, pItemObj)))
		{
			glColor4f(0.9f, 0.1f, 0.1f, 0.4f);
			EnableAlphaTest();
			RenderColor(m_EquipmentSlots[m_iPointedSlot].x + 1, m_EquipmentSlots[m_iPointedSlot].y, m_EquipmentSlots[m_iPointedSlot].width - 4, m_EquipmentSlots[m_iPointedSlot].height - 4);
			EndRenderColor();
		}
	}

	if(m_iPointedSlot != -1 && m_pNewUI3DRenderMng)
	{	
		m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, m_iPointedSlot, 0);
	}
}

void SEASON3B::CNewUIMyInventory::RenderButtons()
{
	EnableAlphaTest();

	//	Inventário Visual
	if (Visuals.VisualSwitch == 1)
	{
		m_BtnPrev.Render();
		m_BtnNext.Render();
	}

	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP)==false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE)==false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHAOSGENESIS) == false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DEVILSQUARE)==false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE)==false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)==false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)==false)
	{
		if(m_bRepairEnableLevel == true)
		{
			m_BtnRepair.Render();
		}
		if(m_bMyShopOpen == true)
		{
			m_BtnMyShop.Render();
		}
	}
	m_BtnExit.Render();

	DisableAlphaBlend();
}

void SEASON3B::CNewUIMyInventory::RenderInventoryDetails()
{
	EnableAlphaTest();

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 12, (gVisualInventory.InventoryVisualOpen == 1 ? "Visual" : GlobalText[223]), INVENTORY_WIDTH, 0, RT3_SORT_CENTER);

	RenderImage(IMAGE_INVENTORY_MONEY, m_Pos.x + 42, m_Pos.y + 364, 170.f, 26.f);

	CPcPoints& rPCRoomPtSys = CPcPoints::Instance();
	if (rPCRoomPtSys.IsPCRoomPointShopMode())
	{
		g_pRenderText->SetBgColor(40, 40, 40, 255);
		g_pRenderText->SetTextColor(255, 128, 255, 255);
		g_pRenderText->RenderText((int)m_Pos.x + 50, (int)m_Pos.y + 371, GlobalText[2326]);
		unicode::t_char Text[256] = { 0, };
		unicode::_sprintf(Text, "%d", rPCRoomPtSys.GetNowPoint());
		g_pRenderText->RenderText((int)m_Pos.x + 82, (int)m_Pos.y + 371, Text);
	}
	else
	{
		DWORD dwZen = CharacterMachine->Gold;

		unicode::t_char Text[256] = { 0, };
		ConvertGold(dwZen, Text);

		g_pRenderText->SetTextColor(getGoldColor(dwZen));
		g_pRenderText->RenderText((int)m_Pos.x + 50, (int)m_Pos.y + 371, Text);
	}
	g_pRenderText->SetFont(g_hFont);
	
	DisableAlphaBlend();
}

bool SEASON3B::CNewUIMyInventory::EquipmentWindowProcess()
{
	if (gItemMove.ItemMove)
	{
		return true;
	}

	if (m_iPointedSlot != -1)
	{
		if (IsRelease(VK_LBUTTON))
		{
			CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
			if (pPickedItem)
			{
				if (gVisualInventory.InventoryVisualOpen == 1)
				{
					if (m_iPointedSlot >= NEW_EQUIPMENT_WEAPON_RIGHT && m_iPointedSlot <= NEW_EQUIPMENT_HELPER)
					{
						ITEM* pItemObj = pPickedItem->GetItem();

						ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[m_iPointedSlot];

						if (pEquipmentItemSlot && pEquipmentItemSlot->Type != -1)
						{
							return true;
						}

						if (g_ChangeRingMgr->CheckChangeRing(pPickedItem->GetItem()->Type))
						{
							ITEM* pItemRingLeft = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
							ITEM* pItemRingRight = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];

							if (g_ChangeRingMgr->CheckChangeRing(pItemRingLeft->Type) || g_ChangeRingMgr->CheckChangeRing(pItemRingRight->Type))
							{
								g_pChatListBox->AddText("", GlobalText[3089], SEASON3B::TYPE_ERROR_MESSAGE);
								CNewUIInventoryCtrl::BackupPickedItem();

								ResetMouseLButton();
								return false;
							}
						}

						if (IsEquipable(m_iPointedSlot, pItemObj))
						{
							if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
							{
								int iSourceIndex = pPickedItem->GetSourceLinealPos();
								int iTargetIndex = m_iPointedSlot;
								SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX + iSourceIndex, pItemObj, REQUEST_EQUIPMENT_INVENTORY, iTargetIndex);

								return true;
							}
							else if (pPickedItem->GetOwnerInventory() == g_pMixInventory->GetInventoryCtrl())
							{
								int iSourceIndex = pPickedItem->GetSourceLinealPos();
								int iTargetIndex = m_iPointedSlot;

								SendRequestEquipmentItem(g_MixRecipeMgr.GetMixInventoryEquipmentIndex(), iSourceIndex, pItemObj, REQUEST_EQUIPMENT_INVENTORY, iTargetIndex);

								return true;
							}

							else if (pPickedItem->GetOwnerInventory() == g_pStorageInventory->GetInventoryCtrl())
							{
								int iSourceIndex = pPickedItem->GetSourceLinealPos();
								int iTargetIndex = m_iPointedSlot;

								g_pStorageInventory->SendRequestItemToMyInven(
									pItemObj, iSourceIndex, iTargetIndex);

								return true;
							}

							else if (pPickedItem->GetOwnerInventory() == g_pTrade->GetMyInvenCtrl())
							{
								int iSourceIndex = pPickedItem->GetSourceLinealPos();
								int iTargetIndex = m_iPointedSlot;

								g_pTrade->SendRequestItemToMyInven(
									pItemObj, iSourceIndex, iTargetIndex);

								return true;
							}

							else if (pPickedItem->GetOwnerInventory() == g_pMyShopInventory->GetInventoryCtrl())
							{
								int iSourceIndex = pPickedItem->GetSourceLinealPos();
								int iTargetIndex = m_iPointedSlot;

								SendRequestEquipmentItem(REQUEST_EQUIPMENT_MYSHOP, MAX_MY_INVENTORY_INDEX + iSourceIndex, pItemObj,
									REQUEST_EQUIPMENT_INVENTORY, iTargetIndex);

								return true;
							}

							else if (pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT && EquipmentItem == false)
							{
								if (pPickedItem->GetSourceLinealPos() == m_iPointedSlot)
								{
									CNewUIInventoryCtrl::BackupPickedItem();
								}
								else
								{
									int iSourceIndex = pItemObj->lineal_pos;
									int iTargetIndex = m_iPointedSlot;
									SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, iSourceIndex, pItemObj, REQUEST_EQUIPMENT_INVENTORY, iTargetIndex);

									return true;
								}
							}
						}
					}
				}
				else
				{
					ITEM* pItemObj = pPickedItem->GetItem();

					ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[m_iPointedSlot];

					if (pEquipmentItemSlot && pEquipmentItemSlot->Type != -1)
					{
						return true;
					}

					if (g_ChangeRingMgr->CheckChangeRing(pPickedItem->GetItem()->Type))
					{
						ITEM* pItemRingLeft = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
						ITEM* pItemRingRight = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];

						if (g_ChangeRingMgr->CheckChangeRing(pItemRingLeft->Type) || g_ChangeRingMgr->CheckChangeRing(pItemRingRight->Type))
						{
							g_pChatListBox->AddText("", GlobalText[3089], SEASON3B::TYPE_ERROR_MESSAGE);
							CNewUIInventoryCtrl::BackupPickedItem();

							ResetMouseLButton();
							return false;
						}
					}

					if (IsEquipable(m_iPointedSlot, pItemObj))
					{
						if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
						{
							int iSourceIndex = pPickedItem->GetSourceLinealPos();
							int iTargetIndex = m_iPointedSlot;
							SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX + iSourceIndex, pItemObj, REQUEST_EQUIPMENT_INVENTORY, iTargetIndex);

							return true;
						}
						else if (pPickedItem->GetOwnerInventory() == g_pMixInventory->GetInventoryCtrl())
						{
							int iSourceIndex = pPickedItem->GetSourceLinealPos();
							int iTargetIndex = m_iPointedSlot;

							SendRequestEquipmentItem(g_MixRecipeMgr.GetMixInventoryEquipmentIndex(), iSourceIndex, pItemObj, REQUEST_EQUIPMENT_INVENTORY, iTargetIndex);

							return true;
						}

						else if (pPickedItem->GetOwnerInventory() == g_pStorageInventory->GetInventoryCtrl())
						{
							int iSourceIndex = pPickedItem->GetSourceLinealPos();
							int iTargetIndex = m_iPointedSlot;

							g_pStorageInventory->SendRequestItemToMyInven(
								pItemObj, iSourceIndex, iTargetIndex);

							return true;
						}

						else if (pPickedItem->GetOwnerInventory() == g_pTrade->GetMyInvenCtrl())
						{
							int iSourceIndex = pPickedItem->GetSourceLinealPos();
							int iTargetIndex = m_iPointedSlot;

							g_pTrade->SendRequestItemToMyInven(
								pItemObj, iSourceIndex, iTargetIndex);

							return true;
						}

						else if (pPickedItem->GetOwnerInventory() == g_pMyShopInventory->GetInventoryCtrl())
						{
							int iSourceIndex = pPickedItem->GetSourceLinealPos();
							int iTargetIndex = m_iPointedSlot;

							SendRequestEquipmentItem(REQUEST_EQUIPMENT_MYSHOP, MAX_MY_INVENTORY_INDEX + iSourceIndex, pItemObj,	REQUEST_EQUIPMENT_INVENTORY, iTargetIndex);

							return true;
						}

						else if (pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT && EquipmentItem == false)
						{
							if (pPickedItem->GetSourceLinealPos() == m_iPointedSlot)
							{
								CNewUIInventoryCtrl::BackupPickedItem();
							}
							else
							{
								int iSourceIndex = pItemObj->lineal_pos;
								int iTargetIndex = m_iPointedSlot;
								SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, iSourceIndex, pItemObj, REQUEST_EQUIPMENT_INVENTORY, iTargetIndex);

								return true;
							}
						}
					}
				}
			}
			else // pPickedItem == NULL
			{
				if (GetRepairMode() == REPAIR_MODE_ON)
				{
					ITEM* pEquippedItem = &CharacterMachine->Equipment[m_iPointedSlot];

					if (pEquippedItem == NULL)
					{
						return true;
					}


					if (IsRepairBan(pEquippedItem) == true)
					{
						return true;
					}


					if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) && g_pNPCShop->IsRepairShop())
					{
						SendRequestRepair(m_iPointedSlot, 0);
					}
					else if (m_bRepairEnableLevel == true)
					{
						SendRequestRepair(m_iPointedSlot, 1);
					}

					return true;
				}
				else
				{
					if (gVisualInventory.InventoryVisualOpen == 1)
					{
						if (m_iPointedSlot >= NEW_EQUIPMENT_WEAPON_RIGHT && m_iPointedSlot <= NEW_EQUIPMENT_HELPER)
						{
							ITEM* pEquippedItem = &CharacterMachine->Equipment[m_iPointedSlot];

							if (pEquippedItem->Type >= 0)	//. ½½·Ô¿¡ ¾ÆÀÌÅÛÀÌ ÀÖ´Ù¸é
							{
								if (gMapManager.WorldActive == WD_10HEAVEN)
								{
									ITEM* pEquippedPetItem = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
									bool bPicked = true;

									if (m_iPointedSlot == EQUIPMENT_HELPER || m_iPointedSlot == EQUIPMENT_WING)
									{
										if (((m_iPointedSlot == EQUIPMENT_HELPER) && !gCharacterManager.IsEquipedWing()))
										{
											bPicked = false;
										}

										else if (((m_iPointedSlot == EQUIPMENT_WING) &&
											!((pEquippedPetItem->Type == ITEM_HELPER + 3)				// µð³ë¶õÆ®
												|| (pEquippedPetItem->Type == ITEM_HELPER + 4)			// ´ÙÅ©È£½º
												|| (pEquippedPetItem->Type == ITEM_HELPER + 37)))			// Ææ¸±
											)
										{
											bPicked = false;
										}
									}

									if (bPicked == true)
									{
										if (CNewUIInventoryCtrl::CreatePickedItem(NULL, pEquippedItem))
										{
											UnequipItem(m_iPointedSlot);
										}
									}
								}
								else
								{
									if (CNewUIInventoryCtrl::CreatePickedItem(NULL, pEquippedItem))
									{
										UnequipItem(m_iPointedSlot);
									}
								}

							}
						}
					}
					else
					{
						ITEM* pEquippedItem = &CharacterMachine->Equipment[m_iPointedSlot];

						if (pEquippedItem->Type >= 0)	//. ½½·Ô¿¡ ¾ÆÀÌÅÛÀÌ ÀÖ´Ù¸é
						{
							if (gMapManager.WorldActive == WD_10HEAVEN)
							{
								ITEM* pEquippedPetItem = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
								bool bPicked = true;

								if (m_iPointedSlot == EQUIPMENT_HELPER || m_iPointedSlot == EQUIPMENT_WING)
								{
									if (((m_iPointedSlot == EQUIPMENT_HELPER) && !gCharacterManager.IsEquipedWing()))
									{
										bPicked = false;
									}

									else if (((m_iPointedSlot == EQUIPMENT_WING) &&
										!((pEquippedPetItem->Type == ITEM_HELPER + 3)				// µð³ë¶õÆ®
											|| (pEquippedPetItem->Type == ITEM_HELPER + 4)			// ´ÙÅ©È£½º
											|| (pEquippedPetItem->Type == ITEM_HELPER + 37)))			// Ææ¸±
										)
									{
										bPicked = false;
									}
								}

								if (bPicked == true)
								{
									if (CNewUIInventoryCtrl::CreatePickedItem(NULL, pEquippedItem))
									{
										UnequipItem(m_iPointedSlot);
									}
								}
							}
							else
							{
								if (CNewUIInventoryCtrl::CreatePickedItem(NULL, pEquippedItem))
								{
									UnequipItem(m_iPointedSlot);
								}
							}

						}
					}
				}

			}
		}
		if (IsRelease(VK_RBUTTON))
		{
			CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

			if (GetRepairMode() != REPAIR_MODE_ON && EquipmentItem == false
				&& pPickedItem == NULL)
			{
				ResetMouseRButton();

				if (gVisualInventory.InventoryVisualOpen == 1)
				{
					if (m_iPointedSlot >= NEW_EQUIPMENT_WEAPON_RIGHT && m_iPointedSlot <= NEW_EQUIPMENT_HELPER)
					{
						ITEM* pEquippedItem = &CharacterMachine->Equipment[m_iPointedSlot];

						if (pEquippedItem->Type >= 0)
						{
							if (gMapManager.WorldActive == WD_10HEAVEN)
							{
								ITEM* pEquippedPetItem = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
								bool bPicked = true;

								if (m_iPointedSlot == EQUIPMENT_HELPER || m_iPointedSlot == EQUIPMENT_WING)
								{
									if (((m_iPointedSlot == EQUIPMENT_HELPER) && !gCharacterManager.IsEquipedWing()))
									{
										bPicked = false;
									}

									else if (((m_iPointedSlot == EQUIPMENT_WING) &&
										!((pEquippedPetItem->Type == ITEM_HELPER + 3)				// µð³ë¶õÆ®
											|| (pEquippedPetItem->Type == ITEM_HELPER + 4)			// ´ÙÅ©È£½º
											|| (pEquippedPetItem->Type == ITEM_HELPER + 37)))			// Ææ¸±
										)
									{
										bPicked = false;
									}
								}

								if (bPicked == true)
								{
									int nDstIndex = FindEmptySlot(pEquippedItem);

									if (-1 != nDstIndex)
									{
										if (CNewUIInventoryCtrl::CreatePickedItem(NULL, pEquippedItem))
										{
											CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

											UnequipItem(m_iPointedSlot);
											pPickedItem->HidePickedItem();
										}
										SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, m_iPointedSlot, pEquippedItem, REQUEST_EQUIPMENT_INVENTORY, nDstIndex + 12);

										return true;
									}
								}
							}
							else
							{
								int nDstIndex = FindEmptySlot(pEquippedItem);

								if (-1 != nDstIndex)
								{
									if (CNewUIInventoryCtrl::CreatePickedItem(NULL, pEquippedItem))
									{
										CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

										UnequipItem(m_iPointedSlot);
										pPickedItem->HidePickedItem();
									}
									SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, m_iPointedSlot, pEquippedItem, REQUEST_EQUIPMENT_INVENTORY, nDstIndex + 12);
									return true;
								}
							}
						}
					}
				}
				else
				{					
					ITEM* pEquippedItem = &CharacterMachine->Equipment[m_iPointedSlot];

					if (pEquippedItem->Type >= 0)
					{
						if (gMapManager.WorldActive == WD_10HEAVEN)
						{
							ITEM* pEquippedPetItem = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
							bool bPicked = true;

							if (m_iPointedSlot == EQUIPMENT_HELPER || m_iPointedSlot == EQUIPMENT_WING)
							{
								if (((m_iPointedSlot == EQUIPMENT_HELPER) && !gCharacterManager.IsEquipedWing()))
								{
									bPicked = false;
								}

								else if (((m_iPointedSlot == EQUIPMENT_WING) &&
									!((pEquippedPetItem->Type == ITEM_HELPER + 3)				// µð³ë¶õÆ®
										|| (pEquippedPetItem->Type == ITEM_HELPER + 4)			// ´ÙÅ©È£½º
										|| (pEquippedPetItem->Type == ITEM_HELPER + 37)))			// Ææ¸±
									)
								{
									bPicked = false;
								}
							}

							if (bPicked == true)
							{
								int nDstIndex = FindEmptySlot(pEquippedItem);

								if (-1 != nDstIndex)
								{
									if (CNewUIInventoryCtrl::CreatePickedItem(NULL, pEquippedItem))
									{
										CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

										UnequipItem(m_iPointedSlot);
										pPickedItem->HidePickedItem();
									}
									SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, m_iPointedSlot, pEquippedItem, REQUEST_EQUIPMENT_INVENTORY, nDstIndex + 12);

									return true;
								}
							}
						}
						else
						{
							int nDstIndex = FindEmptySlot(pEquippedItem);

							if (-1 != nDstIndex)
							{
								if (CNewUIInventoryCtrl::CreatePickedItem(NULL, pEquippedItem))
								{
									CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

									UnequipItem(m_iPointedSlot);
									pPickedItem->HidePickedItem();
								}
								SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, m_iPointedSlot, pEquippedItem, REQUEST_EQUIPMENT_INVENTORY, nDstIndex + 12);
								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
}

bool SEASON3B::CNewUIMyInventory::InventoryProcess()
{
	if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT) == false)
	{
		return false;
	}

	CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
	if (m_pNewInventoryCtrl && pPickedItem && SEASON3B::IsRelease(VK_LBUTTON))
	{
		ITEM* pPickItem = pPickedItem->GetItem();
		if (pPickItem == NULL)
		{
			return false;
		}
		if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
		{
			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);

			if (pPickItem->Type == ITEM_POTION + 13
				|| pPickItem->Type == ITEM_POTION + 16
				|| pPickItem->Type == ITEM_POTION + 42
				|| pPickItem->Type == ITEM_POTION + 43
				|| pPickItem->Type == ITEM_POTION + 44
				|| gCustomJewel.CheckCustomJewelByItem(pPickItem->Type)
				|| gJewelStack.CheckIsStack(pPickItem->Type)
				|| pPickItem->Type == ITEM_POTION + 14
#ifdef LEM_ADD_LUCKYITEM
				|| pPickItem->Type == ITEM_POTION + 160
				|| pPickItem->Type == ITEM_POTION + 161
#endif // LEM_ADD_LUCKYITEM
				)
			{
				ITEM* pItem = m_pNewInventoryCtrl->FindItem(iTargetIndex);
				if (pItem)
				{
					int	iType = pItem->Type;
					int	iLevel = (pItem->Level >> 3) & 15;
					int	iDurability = pItem->Durability;

					bool bSuccess = true;

					if (iType > ITEM_WING + 6
						&& iType != ITEM_HELPER + 30
						&& !(iType >= ITEM_WING + 36 && iType <= ITEM_WING + 43 || gCustomWing.CheckCustomWingByItem(iType))
						&& !(ITEM_WING + 130 <= iType && iType <= ITEM_WING + 134))
					{
						bSuccess = false;
					}

					if (iType == ITEM_BOW + 7 || iType == ITEM_BOW + 15)
					{
						bSuccess = false;
					}

					if ((pPickItem->Type == ITEM_POTION + 13 && iLevel >= 6) || (pPickItem->Type == ITEM_POTION + 14 && iLevel >= 9))
					{
						bSuccess = false;
					}

					if (pPickItem->Type == ITEM_POTION + 13 && iType == ITEM_HELPER + 37 && iDurability != 255)
					{
						SEASON3B::CFenrirRepairMsgBox* pMsgBox = NULL;
						SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CFenrirRepairMsgBoxLayout), &pMsgBox);
						pMsgBox->SetSourceIndex(iSourceIndex);

						int iIndex = pItem->x + pItem->y * m_pNewInventoryCtrl->GetNumberOfColumn();
						pMsgBox->SetTargetIndex(MAX_EQUIPMENT_INDEX + iIndex);

						pPickedItem->HidePickedItem();
						return true;
					}

					if (pPickItem->Type == ITEM_POTION + 42)
					{
						if (g_SocketItemMgr.IsSocketItem(pItem))
						{
							bSuccess = false;
						}
						else
							if (pItem->Jewel_Of_Harmony_Option != 0)
							{
								bSuccess = false;
							}
							else
							{
								StrengthenItem strengthitem = g_pUIJewelHarmonyinfo->GetItemType(static_cast<int>(pItem->Type));

								if (strengthitem == SI_None)
								{
									bSuccess = false;
								}
							}
					}

					if (pPickItem->Type == ITEM_POTION + 43 || pPickItem->Type == ITEM_POTION + 44)
					{
						if (g_SocketItemMgr.IsSocketItem(pItem))
						{
							bSuccess = false;
						}
						else if (pItem->Jewel_Of_Harmony_Option == 0)
						{
							bSuccess = false;
						}
					}

					if (gCustomJewel.CheckCustomJewelApplyItem(pPickItem->Type, iType)) 
					{
						bSuccess = true;
					}

#ifdef LEM_ADD_LUCKYITEM
					if (Check_LuckyItem(pItem->Type))
					{
						bSuccess = false;
						if (pPickItem->Type == ITEM_POTION + 161)
						{
							if (pItem->Jewel_Of_Harmony_Option == 0)	bSuccess = true;
						}
						else if (pPickItem->Type == ITEM_POTION + 160)
						{
							if (pItem->Durability > 0)					bSuccess = true;
						}
						}
#endif // LEM_ADD_LUCKYITEM

					if (bSuccess)
					{
						int iTargetBaseIndex = m_pNewInventoryCtrl->FindBaseIndexByITEM(pItem);
						SendRequestUse(iSourceIndex, MAX_EQUIPMENT_INDEX + iTargetBaseIndex);
						PlayBuffer(SOUND_GET_ITEM01);
						return true;
					}
					}
			}
			else if (iTargetIndex != -1)
			{
				ITEM* pItem = m_pNewInventoryCtrl->FindItem(iTargetIndex);
				if (pItem)
				{
					bool bOverlay = m_pNewInventoryCtrl->IsOverlayItem(pPickItem, pItem);

					if (bOverlay)
					{
						SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX + iSourceIndex, pPickItem, REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX + iTargetIndex);
					}
				}
			}

			if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pPickItem))
			{
				if (pPickedItem->GetSourceLinealPos() != iTargetIndex)
				{
					SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX + iSourceIndex, pPickItem, REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX + iTargetIndex);
				}
				else
				{
					SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
				}

				return true;
			}
		}
		else if (pPickedItem->GetOwnerInventory() == g_pStorageInventory->GetInventoryCtrl())
		{
			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
			if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pPickItem))
			{
				g_pStorageInventory->SendRequestItemToMyInven(pPickItem, iSourceIndex, MAX_EQUIPMENT_INDEX + iTargetIndex);
				return true;
		}
	}
		else if (pPickedItem->GetOwnerInventory() == g_pTrade->GetMyInvenCtrl())
		{
			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
			if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pPickItem))
			{
				g_pTrade->SendRequestItemToMyInven(pPickItem, iSourceIndex, MAX_EQUIPMENT_INDEX + iTargetIndex);

				return true;
			}
		}
#ifdef LEM_ADD_LUCKYITEM
		else if (pPickedItem->GetOwnerInventory() == g_pLuckyItemWnd->GetInventoryCtrl())
		{
			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
			if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pPickItem))
			{
				SendRequestEquipmentItem(g_pLuckyItemWnd->SetWndAction(eLuckyItem_Move), iSourceIndex,
					pPickItem, REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX + iTargetIndex);
				return true;
			}
		}
#endif // LEM_ADD_LUCKYITEM
		else if (pPickedItem->GetOwnerInventory() == g_pMixInventory->GetInventoryCtrl())
		{
			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
			if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pPickItem))
			{
				SendRequestEquipmentItem(g_MixRecipeMgr.GetMixInventoryEquipmentIndex(), iSourceIndex,
					pPickItem, REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX + iTargetIndex);
				return true;
			}
		}
		else if (pPickedItem->GetOwnerInventory() == g_pMyShopInventory->GetInventoryCtrl())
		{
			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);

			if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pPickItem))
			{
				SendRequestEquipmentItem(REQUEST_EQUIPMENT_MYSHOP, MAX_MY_INVENTORY_INDEX + iSourceIndex,
					pPickItem, REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX + iTargetIndex);
				return true;
			}
		}
		else if (pPickItem->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
		{
			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
			if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pPickItem))
			{
				SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, iSourceIndex,
					pPickItem, REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX + iTargetIndex);
				return true;
			}
		}
}
	else if (m_pNewInventoryCtrl && GetRepairMode() == REPAIR_MODE_OFF && SEASON3B::IsRelease(VK_RBUTTON))
	{
		ResetMouseRButton();

		if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE))
		{
			g_pStorageInventory->ProcessMyInvenItemAutoMove();
		}
		else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHAOSGENESIS))
		{
			g_pChaosGenesis->ProcessMyInvenItemAutoMove();
		}
		else if (gTradeX.isVisible())
		{
			gTradeX.MoveItemFromInventory();
			MouseRButton = false;
			MouseRButtonPop = false;
			MouseRButtonPush = false;
		}
		else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY)
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHAOSGENESIS) == false
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) == false
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE) == false
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DEVILSQUARE) == false
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE) == false
#ifdef LEM_ADD_LUCKYITEM
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) == false
#endif // LEM_ADD_LUCKYITEM
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY) == false)
		{
			ITEM* pItem = m_pNewInventoryCtrl->FindItemAtPt(MouseX, MouseY);

			if (pItem == NULL)
			{
				return false;
			}

			int iIndex = pItem->x + pItem->y * m_pNewInventoryCtrl->GetNumberOfColumn();

			if (pItem->Type == ITEM_POTION + 10)
			{
				if (!Teleport)
				{
					SendRequestUse(iIndex, 0);
					return true;
				}
			}

			if ((pItem->Type >= ITEM_POTION + 0 && pItem->Type <= ITEM_POTION + 9)
				|| (pItem->Type == ITEM_POTION + 20 && ((pItem->Level >> 3) & 15) == 0)
				|| (pItem->Type >= ITEM_POTION + 35 && pItem->Type <= ITEM_POTION + 40)
				|| (pItem->Type >= ITEM_POTION + 46 && pItem->Type <= ITEM_POTION + 50)
				|| (pItem->Type == ITEM_POTION + 11 && ((pItem->Level >> 3) & 15) == 14)
				|| (pItem->Type >= ITEM_POTION + 70 && pItem->Type <= ITEM_POTION + 71)
				|| (pItem->Type >= ITEM_POTION + 72 && pItem->Type <= ITEM_POTION + 77)
				|| pItem->Type == ITEM_HELPER + 60
				|| pItem->Type == ITEM_POTION + 94
				|| (pItem->Type >= ITEM_POTION + 85 && pItem->Type <= ITEM_POTION + 87)
				|| (pItem->Type >= ITEM_POTION + 97 && pItem->Type <= ITEM_POTION + 98)
				|| pItem->Type == ITEM_HELPER + 81
				|| pItem->Type == ITEM_HELPER + 82
				|| pItem->Type == ITEM_POTION + 133
				)
			{
				SendRequestUse(iIndex, 0);
				return true;
			}
			else if ((pItem->Type >= ITEM_POTION + 78 && pItem->Type <= ITEM_POTION + 82))
			{
				std::list<eBuffState> secretPotionbufflist;
				secretPotionbufflist.push_back(eBuff_SecretPotion1);
				secretPotionbufflist.push_back(eBuff_SecretPotion2);
				secretPotionbufflist.push_back(eBuff_SecretPotion3);
				secretPotionbufflist.push_back(eBuff_SecretPotion4);
				secretPotionbufflist.push_back(eBuff_SecretPotion5);

				if (g_isCharacterBufflist((&Hero->Object), secretPotionbufflist) == eBuffNone) {
					SendRequestUse(iIndex, 0);
					return true;
				}
				else {
					SEASON3B::CreateOkMessageBox(GlobalText[2530], RGBA(255, 30, 0, 255));
				}
			}
			else if ((pItem->Type >= ITEM_HELPER + 54 && pItem->Type <= ITEM_HELPER + 57) || (pItem->Type == ITEM_HELPER + 58 && gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD))
			{
				bool result = true;
				WORD point[5] = { 0, };

				point[0] = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
				point[1] = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
				point[2] = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
				point[3] = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
				point[4] = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;

#ifdef PBG_ADD_NEWCHAR_MONK
				char nStat[MAX_CLASS][5] =
#else //PBG_ADD_NEWCHAR_MONK
				char nStat[6][5] =
#endif //PBG_ADD_NEWCHAR_MONK
				{
					18, 18, 15, 30,	0,
					28, 20, 25, 10,	0,
					22, 25, 20, 15,	0,
					26, 26, 26, 26,	0,
					26, 20, 20, 15, 25,
					21, 21, 18, 23,	0,
#ifdef PBG_ADD_NEWCHAR_MONK
					32, 27, 25, 20, 0,
#endif //PBG_ADD_NEWCHAR_MONK
				};
				point[pItem->Type - (ITEM_HELPER + 54)] -= nStat[gCharacterManager.GetBaseClass(Hero->Class)][pItem->Type - (ITEM_HELPER + 54)];

				if (point[pItem->Type - (ITEM_HELPER + 54)] < (pItem->Durability * 10))
				{
					result = false;
				}

				if (result == false) {
					SetStandbyItemKey(pItem->Key);
					SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUsePartChargeFruitMsgBoxLayout));
				}
				else {
					SendRequestUse(iIndex, 0);
					return true;
				}
			}
			else if (pItem->Type == ITEM_HELPER + 58 && gCharacterManager.GetBaseClass(Hero->Class) != CLASS_DARK_LORD)
			{
				SEASON3B::CreateOkMessageBox(GlobalText[1905]);
				return true;
			}
			else if (pItem->Type == ITEM_HELPER + 29)
			{
				if (true == IsUnitedMarketPlace())
				{
					char	szOutputText[512];
					sprintf(szOutputText, "%s %s", GlobalText[3014], GlobalText[3015]);

					SEASON3B::CreateOkMessageBox(szOutputText);
					return true;
				}

				if (Hero->SafeZone == false)
				{
					SEASON3B::CreateOkMessageBox(GlobalText[2330]);
				}
				else
				{
					SendRequestEventZoneOpenTime(4, ((pItem->Level >> 3) & 15));
					SetStandbyItemKey(pItem->Key);
					return true;
				}
			}
			else if (pItem->Type == ITEM_HELPER + 46)
			{
				BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_DEVILSQUARE);
				SendRequestEventZoneOpenTime(1, byPossibleLevel);
			}
			else if (pItem->Type == ITEM_HELPER + 47)
			{
				BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_BLOODCASTLE);
				SendRequestEventZoneOpenTime(2, byPossibleLevel);
			}
			else if (pItem->Type == ITEM_HELPER + 48)
			{
				if (Hero->SafeZone || gMapManager.InHellas())
				{
					g_pChatListBox->AddText("", GlobalText[1238], SEASON3B::TYPE_ERROR_MESSAGE);
				}
				else
				{
					SendRequestUse(iIndex, 0);
				}
			}
			else if (pItem->Type == ITEM_HELPER + 61)
			{
				BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_CURSEDTEMPLE);
				SendRequestEventZoneOpenTime(5, byPossibleLevel);
			}

			else if (pItem->Type == ITEM_HELPER + 121)
			{
				if (Hero->SafeZone == false)
				{
					SEASON3B::CreateOkMessageBox(GlobalText[2330]);
				}
				else
				{
					SendRequestEventZoneOpenTime(4, ((pItem->Level >> 3) & 15));
					SetStandbyItemKey(pItem->Key);
					return true;
				}
			}
			else if (pItem->Type == ITEM_HELPER + 51)
			{
				SendRequestEventZoneOpenTime(5, ((pItem->Level >> 3) & 15));
				return true;
			}
			else if (pItem->Type == ITEM_POTION + 19)
			{
				SendRequestEventZoneOpenTime(1, ((pItem->Level >> 3) & 15));
				return true;
			}
			else if (pItem->Type == ITEM_HELPER + 18)
			{
				if (pItem->Level == 0)
				{
					g_pChatListBox->AddText("", GlobalText[2089], SEASON3B::TYPE_ERROR_MESSAGE);
				}
				else
				{
					SendRequestEventZoneOpenTime(2, ((pItem->Level >> 3) & 15) - 1);
				}
				return true;
			}
			else if ((pItem->Type >= ITEM_ETC + 0 && pItem->Type < ITEM_ETC + 37)
				|| (pItem->Type >= ITEM_WING + 7 && pItem->Type <= ITEM_WING + 14)
				|| (pItem->Type >= ITEM_WING + 16 && pItem->Type <= ITEM_WING + 19)
				|| (pItem->Type == ITEM_WING + 20)
				|| (pItem->Type >= ITEM_WING + 21 && pItem->Type <= ITEM_WING + 24)
				|| (pItem->Type == ITEM_WING + 35)
				|| (pItem->Type == ITEM_WING + 44)
				|| (pItem->Type == ITEM_WING + 47)
				|| (pItem->Type == ITEM_WING + 46)
				|| (pItem->Type == ITEM_WING + 45)
				|| (pItem->Type == ITEM_WING + 48)
				|| (pItem->Type == ITEM_ETC + 29)
				|| (pItem->Type == ITEM_ETC + 28)
				)
			{
				bool bReadBookGem = true;

				if ((pItem->Type == ITEM_ETC + 18)
					|| (pItem->Type == ITEM_ETC + 28)
					|| pItem->Type == ITEM_WING + 45
					|| (pItem->Type == ITEM_WING + 46)
					|| (pItem->Type == ITEM_WING + 44)
					)
				{
					if (g_csQuest.getQuestState2(QUEST_CHANGE_UP_3) != QUEST_END)

						bReadBookGem = false;
				}
				if (pItem->Type == ITEM_WING + 48)
				{
					int Level = CharacterAttribute->Level;
					if (Level < 220)
						bReadBookGem = false;
				}
				if (bReadBookGem)
				{

					DWORD Strength, Energy;
					Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
					Energy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;

					if (CharacterAttribute->Level >= ItemAttribute[pItem->Type].RequireLevel &&
						Energy >= pItem->RequireEnergy &&
						Strength >= pItem->RequireStrength)
					{
						SendRequestUse(iIndex, 0);
					}

					return true;
				}
			}
			else if (pItem->Type == ITEM_HELPER + 15)
			{
				int Level = CharacterAttribute->Level;

				if (Level >= 10)
				{
					bool bEquipmentEmpty = true;
					for (int i = 0; i < MAX_EQUIPMENT; i++)
					{
						if (CharacterMachine->Equipment[i].Type != -1)
						{
							bEquipmentEmpty = false;
						}
					}
					int Class = CharacterAttribute->Class;

					if (bEquipmentEmpty == true)
					{
						if (pItem->Level == 32)
						{
							if (gCharacterManager.GetBaseClass(Class) != CLASS_DARK_LORD)
							{
								SEASON3B::CreateOkMessageBox(GlobalText[1905]);
								return true;
							}
						}
						SetStandbyItemKey(pItem->Key);
						SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseFruitMsgBoxLayout));
						return true;
					}
					else
					{
						SEASON3B::CreateOkMessageBox(GlobalText[1909]);
						return true;
					}
				}
				else
				{
					SEASON3B::CreateOkMessageBox(GlobalText[749]);
					return true;
				}
			}
			else if (pItem->Type == ITEM_HELPER + 11)
			{
				bool bUse = false;
				int  Level = (pItem->Level >> 3) & 15;
				switch (Level)
				{
				case 0:
					bUse = true;
					break;
				case 1:
					if (Hero->GuildStatus != G_MASTER)
						bUse = true;
					break;
					}
				if (bUse)
				{
					SendRequestUse(iIndex, 0);
					return true;
				}
				}
			else if (pItem->Type == ITEM_HELPER + 69)
			{
				if (g_PortalMgr.IsRevivePositionSaved())
				{
					if (g_PortalMgr.IsPortalUsable())
					{
						SetStandbyItemKey(pItem->Key);
						SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseReviveCharmMsgBoxLayout));
					}
					else
					{
						SEASON3B::CreateOkMessageBox(GlobalText[2608]);
					}
				}
			}
			else if (pItem->Type == ITEM_HELPER + 70)
			{
				if (g_PortalMgr.IsPortalUsable())
				{
					if (pItem->Durability == 2)
					{
						if (g_PortalMgr.IsPortalPositionSaved())
						{
							SEASON3B::CreateOkMessageBox(GlobalText[2610]);
						}
						else
						{
							SetStandbyItemKey(pItem->Key);
							SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUsePortalCharmMsgBoxLayout));
						}
					}
					else if (pItem->Durability == 1)
					{
						SetStandbyItemKey(pItem->Key);
						SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CReturnPortalCharmMsgBoxLayout));
					}
				}
				else
				{
					SEASON3B::CreateOkMessageBox(GlobalText[2608]);
				}
			}
			else if (pItem->Type == ITEM_HELPER + 66)
			{
				SetStandbyItemKey(pItem->Key);
				SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseSantaInvitationMsgBoxLayout));
			}

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
			else if (g_pMyInventory->IsInvenItem(pItem->Type))
			{
#ifdef LJH_FIX_APP_SHUTDOWN_WEQUIPPING_INVENITEM_WITH_CLICKING_MOUSELBTN
				if (MouseLButton || MouseLButtonPop || MouseLButtonPush)
					return false;
#endif //LJH_FIX_APP_SHUTDOWN_WEQUIPPING_INVENITEM_WITH_CLICKING_MOUSELBTN
				if (pItem->Durability == 0)
					return false;

				int iChangeInvenItemStatus = 0;
				(pItem->Durability == 255) ? iChangeInvenItemStatus = 254 : iChangeInvenItemStatus = 255;

				SendRequestEquippingInventoryItem(MAX_EQUIPMENT_INDEX + iIndex, iChangeInvenItemStatus);
			}
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
			//-- Equip item
			if (EquipmentItem == false)
			{
				int iSrcIndex = iIndex;

				ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];

				int nDstIndex = pItemAttr->m_byItemSlot;

				if (nDstIndex >= 0 && nDstIndex < 12)
				{
					ITEM* pEquipment = &CharacterMachine->Equipment[nDstIndex];

					if (pEquipment && pEquipment->Type != -1)
					{
						if (nDstIndex == EQUIPMENT_WEAPON_RIGHT)
						{
							if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK
#ifdef PBG_ADD_NEWCHAR_MONK
								|| GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
								)
							{
								if (!pItemAttr->TwoHand)
								{
									ITEM_ATTRIBUTE* pItemAttrRight = &ItemAttribute[pEquipment->Type];
									if (!pItemAttrRight->TwoHand) {
										nDstIndex = EQUIPMENT_WEAPON_LEFT;
										pEquipment = &CharacterMachine->Equipment[nDstIndex];

										if (!(pEquipment && pEquipment->Type != -1))
										{
											goto LABEL_32;
										}
									}
									else {
										nDstIndex = -1;
									}
								}
							}
						}
						else if (nDstIndex == EQUIPMENT_RING_RIGHT)
						{
							nDstIndex = EQUIPMENT_RING_LEFT;
							pEquipment = &CharacterMachine->Equipment[nDstIndex];

							if (!(pEquipment && pEquipment->Type != -1))
							{
								goto LABEL_32;
							}
						}
						return true;
					}
				LABEL_32:
					if (IsEquipable(nDstIndex, pItem))
					{
						if (CNewUIInventoryCtrl::CreatePickedItem(NULL, pItem))
						{
							m_pNewInventoryCtrl->RemoveItem(pItem);

							CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
							pPickedItem->HidePickedItem();
						}
						SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, iSrcIndex + MAX_EQUIPMENT_INDEX, pItem, REQUEST_EQUIPMENT_INVENTORY, nDstIndex);
					}
				}

				if (nDstIndex >= NEW_EQUIPMENT_HELPER_SP && nDstIndex <= NEW_EQUIPMENT_RING_LEFT)
				{
					if (gControlSlot.m_ElementEquiped[nDstIndex - NEW_EQUIPMENT_HELPER_SP].Active == 0)
					{
						return false;
					}

					ITEM* pEquipment = &CharacterMachine->Equipment[nDstIndex];

					if (pEquipment && pEquipment->Type != -1)
					{
						if (nDstIndex == NEW_EQUIPMENT_WEAPON_RIGHT)
						{
							if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK)
							{
								if (!pItemAttr->TwoHand)
								{
									nDstIndex = NEW_EQUIPMENT_WEAPON_LEFT;
									pEquipment = &CharacterMachine->Equipment[nDstIndex];

									if (!(pEquipment && pEquipment->Type != -1))
									{
										goto LABEL_34;
									}
								}
							}
						}
						return true;
					}
				LABEL_34:
					if (IsEquipable(nDstIndex, pItem))
					{
						if (CNewUIInventoryCtrl::CreatePickedItem(NULL, pItem))
						{
							m_pNewInventoryCtrl->RemoveItem(pItem);

							CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
							pPickedItem->HidePickedItem();
						}
						SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, iSrcIndex + MAX_EQUIPMENT_INDEX, pItem, REQUEST_EQUIPMENT_INVENTORY, nDstIndex);
					}
				}


				if (nDstIndex >= NEW_EQUIPMENT_WEAPON_RIGHT && nDstIndex < MAX_NEW_EQUIPMENT_INDEX)
				{
					if (Visuals.VisualSwitch == 0)
					{
						return false;
					}

					ITEM* pEquipment = &CharacterMachine->Equipment[nDstIndex];

					if (pEquipment && pEquipment->Type != -1)
					{
						if (nDstIndex == NEW_EQUIPMENT_WEAPON_RIGHT)
						{
							if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK)
							{
								if (!pItemAttr->TwoHand)
								{
									nDstIndex = NEW_EQUIPMENT_WEAPON_LEFT;
									pEquipment = &CharacterMachine->Equipment[nDstIndex];

									if (!(pEquipment && pEquipment->Type != -1))
									{
										goto LABEL_33;
									}
								}
							}
						}
						return true;
					}
				LABEL_33:
					if (IsEquipable(nDstIndex, pItem))
					{
						if (CNewUIInventoryCtrl::CreatePickedItem(NULL, pItem))
						{
							m_pNewInventoryCtrl->RemoveItem(pItem);

							CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
							pPickedItem->HidePickedItem();
						}
						SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, iSrcIndex + MAX_EQUIPMENT_INDEX, pItem, REQUEST_EQUIPMENT_INVENTORY, nDstIndex);
					}
				}
				return true;
			}
			}
	}
	else if (m_pNewInventoryCtrl && GetRepairMode() == REPAIR_MODE_ON && SEASON3B::IsPress(VK_LBUTTON))
	{
		ITEM* pItem = m_pNewInventoryCtrl->FindItemAtPt(MouseX, MouseY);
		if (pItem == NULL)
		{
			return false;
		}

		if (IsRepairBan(pItem) == true)
		{
			return false;
		}

		int iIndex = MAX_EQUIPMENT + pItem->x + pItem->y * m_pNewInventoryCtrl->GetNumberOfColumn();

		if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) && g_pNPCShop->IsRepairShop())
		{
			SendRequestRepair(iIndex, 0);
		}
		else if (m_bRepairEnableLevel == true)
		{
			SendRequestRepair(iIndex, 1);
		}

		return true;
	}

	return false;
}

bool SEASON3B::CNewUIMyInventory::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };

	if (Visuals.VisualSwitch == 1)
	{
		if (m_BtnPrev.UpdateMouseEvent() == true)
		{
			gVisualInventory.InventoryVisualOpen = 0;
			gVisualInventory.InventoryVisualSlot = 12;
			return true;
		}

		if (m_BtnNext.UpdateMouseEvent() == true)
		{
			gVisualInventory.InventoryVisualOpen = 1;
			gVisualInventory.InventoryVisualSlot = 9;
			return true;
		}
	}

	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY);
		return true;
	}
	if(m_BtnExit.UpdateMouseEvent() == true)
	{
		if (gTradeX.isVisible())
		{
			gTradeX.TradeXHiddenWindow();
		}

		if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY))
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
		}
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY);
		return true;
	}

	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP)==false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE)==false 
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DEVILSQUARE)==false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE)==false
#ifdef LEM_ADD_LUCKYITEM
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)==false
#endif // LEM_ADD_LUCKYITEM
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHAOSGENESIS) == false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)==false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)==false)
	{
		if(m_bRepairEnableLevel == true && m_BtnRepair.UpdateMouseEvent() == true)
		{
			ToggleRepairMode();
			return true;
		}

		if( m_bMyShopOpen == true && m_BtnMyShop.UpdateMouseEvent() == true )
		{
			if(m_MyShopMode == MYSHOP_MODE_OPEN)
			{	
				ChangeMyShopButtonStateClose();
				g_pNewUISystem->Show(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
			}
			else if(m_MyShopMode == MYSHOP_MODE_CLOSE)
			{
				ChangeMyShopButtonStateOpen();
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY);
			}

			return true;
		}
	}
	
	return false;
}

void SEASON3B::CNewUIMyInventory::RenderItemVisualToolTip(int iSlotIndex)
{
	auto itemObj = &CharacterMachine->Equipment[iSlotIndex];

	if (itemObj)
	{
		int StackEquiped = iSlotIndex - NEW_EQUIPMENT_WEAPON_RIGHT;

		if (StackEquiped < 0) {
			StackEquiped = 0;
		}

		if (StackEquiped > 8) {
			StackEquiped = 8;
		}

		int iTargetX = m_EquipmentSlots[StackEquiped].x + m_EquipmentSlots[StackEquiped].width / 2;
		int iTargetY = m_EquipmentSlots[StackEquiped].y + m_EquipmentSlots[StackEquiped].height / 2;

		itemObj->bySelectedSlotIndex = iSlotIndex;

		RenderItemInfo(iTargetX, iTargetY, itemObj, false);
	}
}

void SEASON3B::CNewUIMyInventory::RenderItemToolTip(int iSlotIndex)
{
	if(m_iPointedSlot != -1)
	{
		ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[iSlotIndex];

		if (CheckTooltipItensVisual(iSlotIndex) == 0)
		{
			return;
		}


		if(pEquipmentItemSlot->Type != -1)
		{
			int iTargetX = m_EquipmentSlots[iSlotIndex].x + m_EquipmentSlots[iSlotIndex].width / 2;
			int iTargetY = m_EquipmentSlots[iSlotIndex].y + m_EquipmentSlots[iSlotIndex].height / 2;

			pEquipmentItemSlot->bySelectedSlotIndex = iSlotIndex;

			if(m_RepairMode == REPAIR_MODE_OFF)
			{
				RenderItemInfo(iTargetX, iTargetY, pEquipmentItemSlot, false);
			}
			else
			{
				RenderRepairInfo(iTargetX, iTargetY, pEquipmentItemSlot, false);
			}
		}
	}
}

bool SEASON3B::CNewUIMyInventory::CheckTooltipItensVisual(int iSlotIndex)
{
	if (gVisualInventory.InventoryVisualOpen == 1)
	{
		this->RenderItemVisualToolTip(iSlotIndex);
		return 0;
	}

	return 1;
}

bool SEASON3B::CNewUIMyInventory::CanRegisterItemHotKey(int iType)
{
	switch(iType)
	{
	case ITEM_POTION+0:
	case ITEM_POTION+1:
	case ITEM_POTION+2:
	case ITEM_POTION+3:
	case ITEM_POTION+4:
	case ITEM_POTION+5:
	case ITEM_POTION+6:
	case ITEM_POTION+7:
	case ITEM_POTION+8:
	case ITEM_POTION+9:
	case ITEM_POTION+10:
	case ITEM_POTION+20:
	case ITEM_POTION+35:
	case ITEM_POTION+36:
	case ITEM_POTION+37:
	case ITEM_POTION+38:
	case ITEM_POTION+39:
	case ITEM_POTION+40:
	case ITEM_POTION+46:
	case ITEM_POTION+47:
	case ITEM_POTION+48:
	case ITEM_POTION+49:
	case ITEM_POTION+50:
	case ITEM_POTION+70:
	case ITEM_POTION+71:
	case ITEM_POTION+78:
	case ITEM_POTION+79:
	case ITEM_POTION+80:
	case ITEM_POTION+81:
	case ITEM_POTION+82:
	case ITEM_POTION+94:
	case ITEM_POTION+85:
	case ITEM_POTION+86:
	case ITEM_POTION+87:
	case ITEM_POTION+133:
		return true;
	}

	return false;
}

bool SEASON3B::CNewUIMyInventory::CanOpenMyShopInterface()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP)	
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHAOSGENESIS)
#ifdef LEM_ADD_LUCKYITEM
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
#endif // LEM_ADD_LUCKYITEM
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE)
		|| gMapManager.IsCursedTemple() 
		)
	{
		return false;
	}
	return true;
}

bool SEASON3B::CNewUIMyInventory::IsRepairEnableLevel()
{
	return m_bRepairEnableLevel;
}

void SEASON3B::CNewUIMyInventory::SetRepairEnableLevel(bool bOver)
{
	m_bRepairEnableLevel = bOver;
}

void SEASON3B::CNewUIMyInventory::ChangeMyShopButtonStateOpen()
{
	m_MyShopMode = MYSHOP_MODE_OPEN;	
	m_BtnMyShop.UnRegisterButtonState();
	m_BtnMyShop.RegisterButtonState(BUTTON_STATE_UP, IMAGE_INVENTORY_MYSHOP_OPEN_BTN, 0);
	m_BtnMyShop.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_INVENTORY_MYSHOP_OPEN_BTN, 1);
	m_BtnMyShop.ChangeImgIndex(IMAGE_INVENTORY_MYSHOP_OPEN_BTN, 0);
	m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);
}

void SEASON3B::CNewUIMyInventory::ChangeMyShopButtonStateClose()
{
	m_MyShopMode = MYSHOP_MODE_CLOSE;
	m_BtnMyShop.UnRegisterButtonState();
	m_BtnMyShop.RegisterButtonState(BUTTON_STATE_UP, IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, 0);
	m_BtnMyShop.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, 1);
	m_BtnMyShop.ChangeImgIndex(IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, 0);
	m_BtnMyShop.ChangeToolTipText(GlobalText[1127], true);
}

void SEASON3B::CNewUIMyInventory::LockMyShopButtonOpen()
{
	m_BtnMyShop.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
	m_BtnMyShop.ChangeTextColor(RGBA(100, 100, 100, 255));
	m_BtnMyShop.Lock();
	// 1125 "°³ÀÎ»óÁ¡¿­±â(S)"
	m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);	
}

void SEASON3B::CNewUIMyInventory::UnlockMyShopButtonOpen()
{
	m_BtnMyShop.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnMyShop.ChangeTextColor(RGBA(255, 255, 255, 255));
	m_BtnMyShop.UnLock();
	// 1125 "°³ÀÎ»óÁ¡¿­±â(S)"	
	m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);	
}

void SEASON3B::CNewUIMyInventory::ToggleRepairMode()
{
	//. Åä±Û ¼ö¸®¸ðµå
	if(m_RepairMode == REPAIR_MODE_OFF)
	{
		SetRepairMode(true);
	}
	else if(m_RepairMode == REPAIR_MODE_ON)
	{
		SetRepairMode(false);
	}
}

bool SEASON3B::CNewUIMyInventory::IsItem( short int siType, bool bcheckPick )
{
	if( bcheckPick == true )
	{
		CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

		if( pPickedItem )
		{
			ITEM* pItemObj = pPickedItem->GetItem();

			if( pItemObj->Type == siType ) return true;	
		}
	}
		
	ITEM* pholyitemObj = m_pNewInventoryCtrl->FindTypeItem( siType );

	if( pholyitemObj ) return true;	

	return false;
}

int SEASON3B::CNewUIMyInventory::GetNumItemByKey( DWORD dwItemKey )
{
	return m_pNewInventoryCtrl->GetNumItemByKey( dwItemKey );
}

int SEASON3B::CNewUIMyInventory::GetNumItemByType(short sItemType)
{
	return m_pNewInventoryCtrl->GetNumItemByType(sItemType);
}

BYTE SEASON3B::CNewUIMyInventory::GetDurabilityPointedItem()
{
	ITEM* pItem = NULL;	
	
	if(m_iPointedSlot != -1)
	{
		pItem = &CharacterMachine->Equipment[m_iPointedSlot];
		BYTE byDurability = pItem->Durability;

		return byDurability;
	}

	pItem = m_pNewInventoryCtrl->FindItemPointedSquareIndex();
	if(pItem != NULL)
	{
		BYTE byDurability = pItem->Durability;
		return byDurability;
	}
	
	return 0;
}

int SEASON3B::CNewUIMyInventory::GetPointedSlot()
{
	if (m_iPointedSlot != -1)
	{
		return m_iPointedSlot;
	}

	return -1;
}

int SEASON3B::CNewUIMyInventory::GetPointedItemIndex()
{
	if(m_iPointedSlot != -1)
	{
		return m_iPointedSlot;
	}

	int iPointedItemIndex = m_pNewInventoryCtrl->GetPointedSquareIndex();
	if(iPointedItemIndex != -1)
	{
		iPointedItemIndex += MAX_EQUIPMENT_INDEX;
		return iPointedItemIndex;
	}

	return -1;
}

int SEASON3B::CNewUIMyInventory::FindManaItemIndex()
{
	for(int i=ITEM_POTION+6; i>=ITEM_POTION+4; i--)
	{
		int iIndex = FindItemReverseIndex(i);
		if(iIndex != -1)
		{
			return iIndex;
		}
	}

	return -1;
}

void SEASON3B::CNewUIMyInventory::ResetMouseLButton()
{
	MouseLButton = false;
	MouseLButtonPop = false;
	MouseLButtonPush = false;	
}

void SEASON3B::CNewUIMyInventory::ResetMouseRButton()
{
	MouseRButton = false;
	MouseRButtonPop = false;
	MouseRButtonPush = false;
}


#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
BOOL SEASON3B::CNewUIMyInventory::IsInvenItem( const short sType )
{
	BOOL bInvenItem = FALSE;

	if (FALSE 
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM	
		|| (sType == ITEM_HELPER+128 || sType == ITEM_HELPER+129 || sType == ITEM_HELPER+134)
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
		|| (sType >= ITEM_HELPER+130 && sType <= ITEM_HELPER+133 )
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
	)
		bInvenItem = TRUE;
	
	return bInvenItem;
}
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY

#include "stdafx.h"

#if defined(__ANDROID__) && !defined(MU_ANDROID_HAS_LEGACYCLIENT_RUNTIME)

#include "HelperSystem.h"
#include "ItemManager.h"
#include "CharacterList.h"
#include "Patente.h"
#include "CustomEffects.h"
#include "CustomSetEffect.h"

namespace SEASON3B
{
	class CNewUIChatLogWindow;
}

SEASON3B::CNewUIChatLogWindow* g_pChatListBox = NULL;

CHelperSystem gHelperSystem;
CItemManager gItemManager;
CCharacterList gCharacterList;
CPatente gPatente;
CCustomEffects gCustomEffects;
CCustomSetEffect gCCustomSetEffect;

CCustomEffects::CCustomEffects()
{
}

CCustomEffects::~CCustomEffects()
{
}

void CCustomEffects::Init()
{
}

void CCustomEffects::SetInfo(DWORD_PTR, BMD*, int)
{
}

CCustomSetEffect::CCustomSetEffect()
{
}

CCustomSetEffect::~CCustomSetEffect()
{
}

void CCustomSetEffect::CreateEffectSetPlayer(DWORD_PTR, DWORD)
{
}

void CCustomSetEffect::Init()
{
}

CPatente::CPatente()
{
}

CPatente::~CPatente()
{
}

void CPatente::Init()
{
	m_PatentModelInfo.clear();
	m_PatentImageInfo.clear();
}

void CPatente::Clear()
{
	m_PatentModelInfo.clear();
	m_PatentImageInfo.clear();
}

void CPatente::StartLoadImage()
{
}

void CPatente::GCPatentePlayerRecv(PMSG_VIEWPORT_RECV*)
{
}

void CPatente::PatentModelSetIndex(int PatentIndex, int ModelIndex)
{
	m_PatentModelInfo[PatentIndex].PatentIndex = PatentIndex;
	(void)ModelIndex;
}

PATENTE_PLAYER_INFO* CPatente::GetPlayer(int)
{
	return NULL;
}

bool CPatente::IsPlayer(int, int, BYTE)
{
	return false;
}

void CPatente::DeletePlayer(int)
{
}

int CPatente::PatentModelRender(DWORD, DWORD, DWORD)
{
	return 0;
}

void CPatente::NewCreatePartsFactory(CHARACTER*)
{
}

bool CPatente::CheckImagesModel(int)
{
	return false;
}

PATENT_IMAGE_INFO* CPatente::GetPatentImageInfoByImageIndex(int)
{
	return NULL;
}

CCharacterList::CCharacterList()
	: MountAddSize(0.0f)
	, FlyingAddSize(0.0f)
	, FlyingAddHeight(0.0f)
	, MaxCharacters(10)
	, MaxCharactersAccount(10)
{
}

CCharacterList::~CCharacterList()
{
}

void CCharacterList::Init()
{
}

void CCharacterList::OpenCharacterSceneData()
{
}

void CCharacterList::MoveCharacterList()
{
}

void CCharacterList::RenderCharacterList()
{
}

void CCharacterList::SetCharacterPosition(int)
{
}

CItemManager::CItemManager()
{
	m_CustomItemInfo.clear();
}

CItemManager::~CItemManager()
{
}

void CItemManager::Init()
{
}

BOOL CItemManager::GetCustomItemColor(int, float*)
{
	return FALSE;
}

int CItemManager::GET_ITEM(int section, int index)
{
	return section * MAX_ITEM_INDEX + index;
}

int CItemManager::GET_ITEM_MODEL(int section, int index)
{
	return MODEL_ITEM + GET_ITEM(section, index);
}

void CItemManager::GetItemColor(int, float, float, vec3_t Light, bool)
{
	if (Light != NULL)
	{
		Vector(1.0f, 1.0f, 1.0f, Light);
	}
}

CHelperSystem::CHelperSystem()
{
	m_HelperInfo.clear();
}

CHelperSystem::~CHelperSystem()
{
}

void CHelperSystem::Init()
{
}

int CHelperSystem::GetHelperModel(int)
{
	return 0;
}

bool CHelperSystem::CheckIsHelper(int)
{
	return false;
}

bool CHelperSystem::CheckHelperType(int, int)
{
	return false;
}

HELPER_INFO* CHelperSystem::GetHelper(int)
{
	return NULL;
}

void CHelperSystem::InvCreateEquippingEffect(DWORD)
{
}

int CHelperSystem::CheckIsFenrirOrDino(int)
{
	return 0;
}

#endif

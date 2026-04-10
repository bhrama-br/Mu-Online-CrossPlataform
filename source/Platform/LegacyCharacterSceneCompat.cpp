#include "stdafx.h"

#if defined(__ANDROID__)

#include "CharacterManager.h"
#include "DarkSpirit.h"
#include "GMNewTown.h"
#include "GMSwampOfQuiet.h"
#include "PhysicsManager.h"
#include "Platform/LegacyAndroidGameCompat.h"
#include "WSclient.h"
#include "w_MapProcess.h"
#include "ZzzCharacter.h"
#include "ZzzInfomation.h"

#include <algorithm>
#include <cstring>

namespace
{
	CHARACTER_MACHINE g_android_character_machine = {};
	ITEM_ATTRIBUTE g_android_item_attributes[MAX_ITEM + 1024] = {};
	SKILL_ATTRIBUTE g_android_skill_attributes[MAX_SKILLS] = {};
	GATE_ATTRIBUTE g_android_gate_attributes[MAX_GATES] = {};
	MONSTER_SCRIPT g_android_monster_scripts[MAX_MONSTER] = {};

	template <typename T>
	void ZeroStruct(T* value)
	{
		if (value != NULL)
		{
			memset(value, 0, sizeof(T));
		}
	}
}

float FPS_ANIMATION_FACTOR = 1.0f;
double WorldTime = 0.0;
int EditFlag = 0;
int CurrentProtocolState = RECEIVE_CHARACTERS_LIST;

ITEM_ATTRIBUTE* ItemAttribute = g_android_item_attributes;
SKILL_ATTRIBUTE* SkillAttribute = g_android_skill_attributes;
GATE_ATTRIBUTE* GateAttribute = g_android_gate_attributes;
MONSTER_SCRIPT MonsterScript[MAX_MONSTER] = {};
CHARACTER_MACHINE* CharacterMachine = &g_android_character_machine;
CHARACTER_ATTRIBUTE* CharacterAttribute = &g_android_character_machine.Character;

bool rand_fps_check(int reference_frames)
{
	if (reference_frames <= 1)
	{
		return true;
	}

	return (rand() % reference_frames) == 0;
}

void SetAction(OBJECT* object, int action, bool reset)
{
	if (object == NULL)
	{
		return;
	}

	if (reset || object->CurrentAction != action)
	{
		object->PriorAction = object->CurrentAction;
		object->CurrentAction = static_cast<unsigned short>(action);
		object->PriorAnimationFrame = object->AnimationFrame;
		object->AnimationFrame = 0.0f;
	}
}

CCharacterManager gCharacterManager;

CCharacterManager::CCharacterManager()
{
}

CCharacterManager::~CCharacterManager()
{
}

int CCharacterManager::GetCharacterIndex(CHARACTER* pChar)
{
	if (pChar == NULL || CharactersClient == NULL)
	{
		return -1;
	}

	return static_cast<int>(pChar - &CharactersClient[0]);
}

BYTE CCharacterManager::ChangeServerClassTypeToClientClassType(const BYTE byServerClassType)
{
	return static_cast<BYTE>(
		(((byServerClassType >> 4) & 0x01) << 3) |
		(byServerClassType >> 5) |
		(((byServerClassType >> 3) & 0x01) << 4));
}

BYTE CCharacterManager::GetCharacterClass(const BYTE byClass)
{
	const BYTE byFirstClass = byClass & 0x7;
	const BYTE bySecondClass = (byClass >> 3) & 0x01;
	const BYTE byThirdClass = (byClass >> 4) & 0x01;

	switch (byFirstClass)
	{
	case 0: return byThirdClass ? CLASS_GRANDMASTER : (bySecondClass ? CLASS_SOULMASTER : CLASS_WIZARD);
	case 1: return byThirdClass ? CLASS_BLADEMASTER : (bySecondClass ? CLASS_BLADEKNIGHT : CLASS_KNIGHT);
	case 2: return byThirdClass ? CLASS_HIGHELF : (bySecondClass ? CLASS_MUSEELF : CLASS_ELF);
	case 3: return byThirdClass ? CLASS_DUELMASTER : CLASS_DARK;
	case 4: return byThirdClass ? CLASS_LORDEMPEROR : CLASS_DARK_LORD;
	case 5: return byThirdClass ? CLASS_DIMENSIONMASTER : (bySecondClass ? CLASS_BLOODYSUMMONER : CLASS_SUMMONER);
#ifdef PBG_ADD_NEWCHAR_MONK
	case 6: return byThirdClass ? CLASS_TEMPLENIGHT : CLASS_RAGEFIGHTER;
#endif
	default: return byFirstClass;
	}
}

bool CCharacterManager::IsSecondClass(const BYTE byClass)
{
	return (((signed int)byClass >> 3) & 1) != 0;
}

bool CCharacterManager::IsThirdClass(const BYTE byClass)
{
	return (((signed int)byClass >> 4) & 1) != 0;
}

bool CCharacterManager::IsMasterLevel(const BYTE byClass)
{
	return CharacterAttribute != NULL && CharacterAttribute->Level >= 401 && IsThirdClass(byClass);
}

bool CCharacterManager::IsMasterLevelExpCheck(const BYTE byClass)
{
	return IsMasterLevel(byClass);
}

const char* CCharacterManager::GetCharacterClassText(const BYTE byClass)
{
	(void)byClass;
	return "";
}

BYTE CCharacterManager::GetSkinModelIndex(const BYTE byClass)
{
	BYTE bySkinIndex = 0;
	BYTE byFirstClass = byClass & 0x7;
	BYTE bySecondClass = (byClass >> 3) & 0x01;
	BYTE byThirdClass = (byClass >> 4) & 0x01;

	if (byFirstClass == CLASS_WIZARD || byFirstClass == CLASS_KNIGHT || byFirstClass == CLASS_ELF || byFirstClass == CLASS_SUMMONER)
	{
		bySkinIndex = byFirstClass + (bySecondClass + byThirdClass) * MAX_CLASS;
	}
	else
	{
		bySkinIndex = byFirstClass + (byThirdClass * 2) * MAX_CLASS;
	}

	return bySkinIndex;
}

BYTE CCharacterManager::GetStepClass(const BYTE byClass)
{
	if (IsThirdClass(byClass))
	{
		return 3;
	}
	else if (IsSecondClass(byClass))
	{
		return 2;
	}

	return 1;
}

int CCharacterManager::GetEquipedBowType(CHARACTER* pChar)
{
	if (pChar == NULL)
	{
		return BOWTYPE_NONE;
	}

	if ((pChar->Weapon[1].Type >= MODEL_BOW && pChar->Weapon[1].Type < MODEL_BOW + MAX_ITEM_INDEX) &&
		pChar->Weapon[1].Type != MODEL_BOW + 7)
	{
		return BOWTYPE_BOW;
	}

	if ((pChar->Weapon[0].Type >= MODEL_BOW + 8 && pChar->Weapon[0].Type < MODEL_BOW + MAX_ITEM_INDEX) &&
		pChar->Weapon[0].Type != MODEL_BOW + 15)
	{
		return BOWTYPE_CROSSBOW;
	}

	return BOWTYPE_NONE;
}

int CCharacterManager::GetEquipedBowType()
{
	if (CharacterMachine == NULL)
	{
		return BOWTYPE_NONE;
	}

	return GetEquipedBowType(&Hero[0]);
}

int CCharacterManager::GetEquipedBowType(ITEM* pItem)
{
	if (pItem == NULL)
	{
		return BOWTYPE_NONE;
	}

	if (((pItem->Type >= ITEM_BOW) && (pItem->Type <= ITEM_BOW + 6)) ||
		(pItem->Type == ITEM_BOW + 17) ||
		((pItem->Type >= ITEM_BOW + 20) && (pItem->Type <= ITEM_BOW + 24)))
	{
		return BOWTYPE_BOW;
	}

	if (((pItem->Type >= ITEM_BOW + 8) && (pItem->Type <= ITEM_BOW + 19)))
	{
		return BOWTYPE_CROSSBOW;
	}

	return BOWTYPE_NONE;
}

int CCharacterManager::GetEquipedBowType_Skill()
{
	return GetEquipedBowType();
}

bool CCharacterManager::IsEquipedWing()
{
	return CharacterMachine != NULL && CharacterMachine->Equipment[EQUIPMENT_WING].Type >= 0;
}

void CCharacterManager::GetMagicSkillDamage(int iType, int* piMinDamage, int* piMaxDamage)
{
	(void)iType;
	if (piMinDamage != NULL) *piMinDamage = 0;
	if (piMaxDamage != NULL) *piMaxDamage = 0;
}

void CCharacterManager::GetCurseSkillDamage(int iType, int* piMinDamage, int* piMaxDamage)
{
	GetMagicSkillDamage(iType, piMinDamage, piMaxDamage);
}

void CCharacterManager::GetSkillDamage(int iType, int* piMinDamage, int* piMaxDamage)
{
	GetMagicSkillDamage(iType, piMinDamage, piMaxDamage);
}

CDarkSpirit gDarkSpirit;

CDarkSpirit::CDarkSpirit()
{
}

CDarkSpirit::~CDarkSpirit()
{
}

void CDarkSpirit::Init()
{
	m_DarkSpirit.clear();
	m_DarkSpiritInfo.clear();
	m_DarkSpiritModels.clear();
}

void CDarkSpirit::CreateDarkSpirit(CHARACTER* m_Owner, int ModelIndex)
{
	(void)m_Owner;
	(void)ModelIndex;
}

bool CDarkSpirit::CheckExistDarkSpirit(CHARACTER* Owner)
{
	(void)Owner;
	return false;
}

int CDarkSpirit::GetItemDarkSpirit(CHARACTER* Owner)
{
	(void)Owner;
	return -1;
}

void CDarkSpirit::Register(BoostSmart_Ptr(DARK_SPIRIT_HELPER_VIEW) info)
{
	(void)info;
}

void CDarkSpirit::DeleteDarkSpirit(CHARACTER* Owner, int itemType, bool allDelete)
{
	(void)Owner;
	(void)itemType;
	(void)allDelete;
}

void CDarkSpirit::UnRegister(CHARACTER* Owner, int itemType, bool isUnregistAll)
{
	(void)Owner;
	(void)itemType;
	(void)isUnregistAll;
}

DARK_SPIRIT_INFO* CDarkSpirit::getDarkSpirit(int ItemIndex)
{
	std::map<int, DARK_SPIRIT_INFO>::iterator iter = m_DarkSpiritInfo.find(ItemIndex);
	return iter != m_DarkSpiritInfo.end() ? &iter->second : NULL;
}

bool CDarkSpirit::checkIsDarkSpirit(int ItemIndex)
{
	return m_DarkSpiritInfo.find(ItemIndex) != m_DarkSpiritInfo.end();
}

int CDarkSpirit::GetItemModelDarkSpirit(int ItemIndex)
{
	DARK_SPIRIT_INFO* info = getDarkSpirit(ItemIndex);
	return info != NULL ? info->ItemModel : -1;
}

void CDarkSpirit::RenderModel(DWORD BMDStruct, DWORD ObjectStruct, DWORD ItemIndex)
{
	(void)BMDStruct;
	(void)ObjectStruct;
	(void)ItemIndex;
}

int CDarkSpirit::CheckNewRaven(CHARACTER* Struct)
{
	(void)Struct;
	return 0;
}

int CDarkSpirit::CheckCreateDarkSpiritNow(int Index)
{
	(void)Index;
	return 0;
}

namespace SEASON3B
{
	bool GMNewTown::m_bCharacterSceneCheckMouse = false;

	GMNewTown::GMNewTown()
	{
	}

	GMNewTown::~GMNewTown()
	{
	}

	bool GMNewTown::IsCurrentMap() { return false; }
	void GMNewTown::CreateObject(OBJECT* pObject) { (void)pObject; }
	bool GMNewTown::MoveObject(OBJECT* pObject) { (void)pObject; return false; }
	bool GMNewTown::RenderObjectVisual(OBJECT* pObject, BMD* pModel) { (void)pObject; (void)pModel; return false; }
	bool GMNewTown::RenderObject(OBJECT* pObject, BMD* pModel, bool ExtraMon) { (void)pObject; (void)pModel; (void)ExtraMon; return false; }
	void GMNewTown::RenderObjectAfterCharacter(OBJECT* pObject, BMD* pModel, bool ExtraMon) { (void)pObject; (void)pModel; (void)ExtraMon; }
	CHARACTER* GMNewTown::CreateNewTownMonster(int iType, int PosX, int PosY, int Key) { (void)iType; (void)PosX; (void)PosY; (void)Key; return NULL; }
	bool GMNewTown::MoveMonsterVisual(OBJECT* pObject, BMD* pModel) { (void)pObject; (void)pModel; return false; }
	void GMNewTown::MoveBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel) { (void)pCharacter; (void)pObject; (void)pModel; }
	bool GMNewTown::RenderMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel) { (void)pCharacter; (void)pObject; (void)pModel; return false; }
	bool GMNewTown::AttackEffectMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel) { (void)pCharacter; (void)pObject; (void)pModel; return false; }
	bool GMNewTown::SetCurrentActionMonster(CHARACTER* pCharacter, OBJECT* pObject) { (void)pCharacter; (void)pObject; return false; }
	bool GMNewTown::PlayMonsterSound(OBJECT* pObject) { (void)pObject; return false; }
	void GMNewTown::PlayObjectSound(OBJECT* pObject) { (void)pObject; }
	bool GMNewTown::IsCheckMouseIn() { return false; }
	bool GMNewTown::CharacterSceneCheckMouse(OBJECT* pObj) { (void)pObj; return false; }
}

namespace SEASON3C
{
	GMSwampOfQuiet::GMSwampOfQuiet()
	{
	}

	GMSwampOfQuiet::~GMSwampOfQuiet()
	{
	}

	bool GMSwampOfQuiet::IsCurrentMap() { return false; }
	void GMSwampOfQuiet::RenderBaseSmoke() {}
	void GMSwampOfQuiet::CreateObject(OBJECT* pObject) { (void)pObject; }
	bool GMSwampOfQuiet::MoveObject(OBJECT* pObject) { (void)pObject; return false; }
	bool GMSwampOfQuiet::RenderObjectVisual(OBJECT* pObject, BMD* pModel) { (void)pObject; (void)pModel; return false; }
	bool GMSwampOfQuiet::RenderObject(OBJECT* pObject, BMD* pModel, bool ExtraMon) { (void)pObject; (void)pModel; (void)ExtraMon; return false; }
	void GMSwampOfQuiet::RenderObjectAfterCharacter(OBJECT* pObject, BMD* pModel, bool ExtraMon) { (void)pObject; (void)pModel; (void)ExtraMon; }
	CHARACTER* GMSwampOfQuiet::CreateSwampOfQuietMonster(int iType, int PosX, int PosY, int Key) { (void)iType; (void)PosX; (void)PosY; (void)Key; return NULL; }
	bool GMSwampOfQuiet::MoveMonsterVisual(OBJECT* pObject, BMD* pModel) { (void)pObject; (void)pModel; return false; }
	void GMSwampOfQuiet::MoveBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel) { (void)pCharacter; (void)pObject; (void)pModel; }
	bool GMSwampOfQuiet::RenderMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel) { (void)pCharacter; (void)pObject; (void)pModel; return false; }
	bool GMSwampOfQuiet::AttackEffectMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel) { (void)pCharacter; (void)pObject; (void)pModel; return false; }
	bool GMSwampOfQuiet::SetCurrentActionMonster(CHARACTER* pCharacter, OBJECT* pObject) { (void)pCharacter; (void)pObject; return false; }
	bool GMSwampOfQuiet::PlayMonsterSound(OBJECT* pObject) { (void)pObject; return false; }
	void GMSwampOfQuiet::PlayObjectSound(OBJECT* pObject) { (void)pObject; }
}

MapProcessPtr g_MapProcess;

MapProcessPtr MapProcess::Make()
{
	return MapProcessPtr(new MapProcess());
}

MapProcess::MapProcess()
{
	Init();
}

MapProcess::~MapProcess()
{
	Destroy();
}

void MapProcess::Init()
{
}

void MapProcess::Destroy()
{
}

bool MapProcess::LoadMapData() { return false; }
bool MapProcess::CreateObject(OBJECT* o) { (void)o; return false; }
bool MapProcess::MoveObject(OBJECT* o) { (void)o; return false; }
bool MapProcess::RenderObjectVisual(OBJECT* o, BMD* b) { (void)o; (void)b; return false; }
bool MapProcess::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon) { (void)o; (void)b; (void)ExtraMon; return false; }
void MapProcess::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon) { (void)o; (void)b; (void)ExtraMon; }
void MapProcess::RenderFrontSideVisual() {}
CHARACTER* MapProcess::CreateMonster(int iType, int PosX, int PosY, int Key) { (void)iType; (void)PosX; (void)PosY; (void)Key; return NULL; }
bool MapProcess::MoveMonsterVisual(OBJECT* o, BMD* b) { (void)o; (void)b; return false; }
void MapProcess::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; }
bool MapProcess::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; return false; }
bool MapProcess::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; return false; }
bool MapProcess::SetCurrentActionMonster(CHARACTER* c, OBJECT* o) { (void)c; (void)o; return false; }
bool MapProcess::PlayMonsterSound(OBJECT* o) { (void)o; return false; }
bool MapProcess::ReceiveMapMessage(BYTE code, BYTE subcode, BYTE* ReceiveBuffer) { (void)code; (void)subcode; (void)ReceiveBuffer; return false; }
void MapProcess::Register(BoostSmart_Ptr(BaseMap) pMap) { (void)pMap; }
void MapProcess::UnRegister(ENUM_WORLD type) { (void)type; }
BaseMap& MapProcess::GetMap(int type)
{
	static BaseMap g_default_map;
	(void)type;
	return g_default_map;
}

bool MapProcess::FindMap(ENUM_WORLD type)
{
	(void)type;
	return false;
}

BaseMap& MapProcess::FindBaseMap(ENUM_WORLD type)
{
	static BaseMap g_default_map;
	(void)type;
	return g_default_map;
}

MapProcess& TheMapProcess()
{
	if (!g_MapProcess)
	{
		g_MapProcess = MapProcess::Make();
	}

	return *g_MapProcess;
}

float CPhysicsVertex::s_Gravity = 0.0f;
float CPhysicsVertex::s_fMass = 1.0f;
float CPhysicsVertex::s_fInvOfMass = 1.0f;
float CPhysicsManager::s_fWind = 0.0f;
vec3_t CPhysicsManager::s_vWind = { 0.0f, 0.0f, 0.0f };

CPhysicsCloth::CPhysicsCloth()
{
	Clear();
}

CPhysicsCloth::~CPhysicsCloth()
{
	Destroy();
}

void CPhysicsCloth::Clear(void)
{
	m_oOwner = NULL;
	m_iBone = -1;
	m_iTexFront = -1;
	m_iTexBack = -1;
	m_dwType = 0;
	m_fxPos = m_fyPos = m_fzPos = 0.0f;
	m_fWidth = m_fHeight = 0.0f;
	m_iNumHor = m_iNumVer = 0;
	m_iNumVertices = 0;
	m_pVertices = NULL;
	m_iNumLink = 0;
	m_pLink = NULL;
	m_fWind = 0.0f;
	m_byWindMax = 0;
	m_byWindMin = 0;
	m_fUnitWidth = 0.0f;
	m_fUnitHeight = 0.0f;
	m_lstCollision.RemoveAll();
}

BOOL CPhysicsCloth::Create(OBJECT* o, int iBone, float fxPos, float fyPos, float fzPos, int iNumHor, int iNumVer, float fWidth, float fHeight, int iTexFront, int TexBack, DWORD dwType)
{
	m_oOwner = o;
	m_iBone = iBone;
	m_fxPos = fxPos;
	m_fyPos = fyPos;
	m_fzPos = fzPos;
	m_iNumHor = iNumHor;
	m_iNumVer = iNumVer;
	m_fWidth = fWidth;
	m_fHeight = fHeight;
	m_iTexFront = iTexFront;
	m_iTexBack = TexBack;
	m_dwType = dwType;
	return FALSE;
}

void CPhysicsCloth::Destroy(void)
{
	delete[] m_pVertices;
	m_pVertices = NULL;
	delete[] m_pLink;
	m_pLink = NULL;
	m_iNumVertices = 0;
	m_iNumLink = 0;
}

void CPhysicsCloth::SetFixedVertices(float Matrix[3][4]) { (void)Matrix; }
void CPhysicsCloth::SetLink(int iLink, int iVertex1, int iVertex2, float fDistanceSmall, float fDistanceLarge, BYTE byStyle) { (void)iLink; (void)iVertex1; (void)iVertex2; (void)fDistanceSmall; (void)fDistanceLarge; (void)byStyle; }
BOOL CPhysicsCloth::Move2(float fTime, int iCount) { (void)fTime; (void)iCount; return FALSE; }
BOOL CPhysicsCloth::Move(float fTime) { (void)fTime; return FALSE; }
void CPhysicsCloth::GetPosition(int index, vec3_t* pPos) { (void)index; if (pPos != NULL) Vector(0.0f, 0.0f, 0.0f, *pPos); }
void CPhysicsCloth::InitForces(void) {}
void CPhysicsCloth::MoveVertices(float fTime) { (void)fTime; }
BOOL CPhysicsCloth::PreventFromStretching(void) { return FALSE; }
void CPhysicsCloth::Render(vec3_t* pvColor, int iLevel) { (void)pvColor; (void)iLevel; }
void CPhysicsCloth::RenderFace(BOOL bFront, int iTexture, vec3_t* pvRenderPos) { (void)bFront; (void)iTexture; (void)pvRenderPos; }
void CPhysicsCloth::RenderCollisions(void) {}
void CPhysicsCloth::AddCollisionSphere(float fXPos, float fYPos, float fZPos, float fRadius, int iBone) { (void)fXPos; (void)fYPos; (void)fZPos; (void)fRadius; (void)iBone; }
void CPhysicsCloth::ProcessCollision(void) {}

CPhysicsClothMesh::CPhysicsClothMesh()
{
	Clear();
}

CPhysicsClothMesh::~CPhysicsClothMesh()
{
	Destroy();
}

void CPhysicsClothMesh::Clear(void)
{
	CPhysicsCloth::Clear();
	m_iMesh = -1;
	m_iBoneForFixed = -1;
	m_iBMDType = -1;
}

BOOL CPhysicsClothMesh::Create(OBJECT* o, int iMesh, int iBone, DWORD dwType, int iBMDType)
{
	m_oOwner = o;
	m_iMesh = iMesh;
	m_iBone = iBone;
	m_dwType = dwType;
	m_iBMDType = iBMDType;
	return FALSE;
}

BOOL CPhysicsClothMesh::Create(OBJECT* o, int iMesh, int iBone, float fxPos, float fyPos, float fzPos, int iNumHor, int iNumVer, float fWidth, float fHeight, int iTexFront, int TexBack, DWORD dwType, int iBMDType)
{
	CPhysicsCloth::Create(o, iBone, fxPos, fyPos, fzPos, iNumHor, iNumVer, fWidth, fHeight, iTexFront, TexBack, dwType);
	m_iMesh = iMesh;
	m_iBMDType = iBMDType;
	return FALSE;
}

int CPhysicsClothMesh::FindMatchVertex(Mesh_t* pMesh, int iV1, int iV2, int iV3) { (void)pMesh; (void)iV1; (void)iV2; (void)iV3; return -1; }
BOOL CPhysicsClothMesh::FindInLink(int iCount, int iV1, int iV2) { (void)iCount; (void)iV1; (void)iV2; return FALSE; }
void CPhysicsClothMesh::SetFixedVertices(float Matrix[3][4]) { (void)Matrix; }
void CPhysicsClothMesh::NotifyVertexPos(int iVertex, vec3_t vPos) { (void)iVertex; (void)vPos; }
void CPhysicsClothMesh::InitForces(void) {}
void CPhysicsClothMesh::Render(vec3_t* pvColor, int iLevel) { (void)pvColor; (void)iLevel; }

CPhysicsManager::CPhysicsManager()
{
}

CPhysicsManager::~CPhysicsManager()
{
	RemoveAll();
}

void CPhysicsManager::Clear(void)
{
	RemoveAll();
}

void CPhysicsManager::Move(float fTime)
{
	(void)fTime;
}

void CPhysicsManager::Render(void)
{
}

void CPhysicsManager::Add(CPhysicsCloth* pCloth)
{
	if (pCloth != NULL)
	{
		m_lstCloth.AddTail(pCloth);
	}
}

void CPhysicsManager::Remove(OBJECT* oOwner)
{
	(void)oOwner;
}

void CPhysicsManager::RemoveAll(void)
{
	CNode<CPhysicsCloth*>* position = m_lstCloth.FindHead();
	for (; position != NULL; position = m_lstCloth.GetNext(position))
	{
		CPhysicsCloth* cloth = position->GetData();
		delete cloth;
	}
	m_lstCloth.RemoveAll();
}

#endif

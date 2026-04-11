#include "stdafx.h"

#if defined(__ANDROID__) && !defined(MU_ANDROID_HAS_ZZZSCENE_RUNTIME)

#include <android/log.h>
#include "ChangeRingManager.h"
#include "CKANTURUDirection.h"
#include "CSParts.h"
#include "ZzzBMD.h"
#include "Event.h"
#include "GM3rdChangeUp.h"
#include "GMAida.h"
#include "GMBattleCastle.h"
#include "GMCrywolf1st.h"
#include "GMCryingWolf2nd.h"
#include "GMEmpireGuardian1.h"
#include "GMEmpireGuardian2.h"
#include "GMEmpireGuardian3.h"
#include "GMEmpireGuardian4.h"
#include "GMHellas.h"
#include "GMHuntingGround.h"
#include "GM_Kanturu_2nd.h"
#include "GM_Kanturu_3rd.h"
#include "GM_Kanturu_1st.h"
#include "Lua.h"
#include "LuaBMD.h"
#include "LuaEffects.h"
#include "NewUIMiniMap.h"
#include "NewUISystem.h"
#include "PartyManager.h"
#include "PhysicsManager.h"
#include "Preview.h"
#include "GIPetManager.h"
#include "SkillManager.h"
#include "SummonSystem.h"
#include "MoveCommandData.h"
#include "CSQuest.h"
#include "CSItemOption.h"
#include "GOBoid.h"
#include "QuestMng.h"
#include "SocketSystem.h"
#include "w_PetProcess.h"
#include "w_CursedTemple.h"
#include "BoneManager.h"
#include "CustomCape.h"
#include "CustomJewel.h"
#include "CustomWing.h"
#include "DuelMgr.h"
#include "ElementPet.h"
#include "GuildCache.h"
#include "HelperManager.h"
#include "LuaCloth.h"
#include "LuaLoadImage.h"
#include "MonsterEffect.h"
#include "MonsterName.h"
#include "Monsters.h"
#include "NewUICursedTempleSystem.h"
#include "ZzzAI.h"
#include "ZzzCharacter.h"
#include "ZzzEffect.h"
#include "ZzzInfomation.h"
#include "ZzzInterface.h"
#include "ZzzLodTerrain.h"
#include "ZzzObject.h"
#include "SimpleModulus.h"
#include "wsctlc.h"

#include <cstring>

int WaterTextureNumber = 0;

bool OpenSMDModel(int ID, char* FileName1, int Actions, bool Flip)
{
	(void)ID;
	(void)FileName1;
	(void)Actions;
	(void)Flip;
	return false;
}

bool OpenSMDAnimation(int ID, char* FileName2, bool LockPosition)
{
	(void)ID;
	(void)FileName2;
	(void)LockPosition;
	return false;
}

Lua::Lua()
	: m_luaState(NULL)
{
}

Lua::~Lua()
{
}

void Lua::RegisterLua()
{
}

void Lua::CloseLua()
{
	m_luaState = NULL;
}

lua_State* Lua::GetState()
{
	return m_luaState;
}

void Lua::DoFile(const char* szFileName)
{
	(void)szFileName;
}

bool Lua::Generic_Call(const char* func, const char* sig, ...)
{
	(void)func;
	(void)sig;
	return false;
}

LuaBMD::LuaBMD()
{
}

LuaBMD::~LuaBMD()
{
}

void LuaBMD::RegisterClassBMD(lua_State* lua)
{
	(void)lua;
}

void FuncCreateParticle(DWORD BMDStruct, int Bitmap, int SubType, int Link, float Scale, vec3_t Light, DWORD ObjectStruct)
{
	(void)BMDStruct;
	(void)Bitmap;
	(void)SubType;
	(void)Link;
	(void)Scale;
	(void)Light;
	(void)ObjectStruct;
}

CLuaEffects::CLuaEffects()
	: m_Struct(NULL)
{
}

CLuaEffects::~CLuaEffects()
{
}

void CLuaEffects::RegisterLuaClass(lua_State* lua)
{
	(void)lua;
}

CPhysicsVertex::CPhysicsVertex()
{
	Clear();
}

CPhysicsVertex::~CPhysicsVertex()
{
}

void CPhysicsVertex::Clear(void)
{
	Vector(0.0f, 0.0f, 0.0f, m_vForce);
	Vector(0.0f, 0.0f, 0.0f, m_vVel);
	Vector(0.0f, 0.0f, 0.0f, m_vPos);
	m_byState = 0;
	m_iCountOneTimeMove = 0;
	Vector(0.0f, 0.0f, 0.0f, m_vOneTimeMove);
}

void CPhysicsVertex::Init(float fXPos, float fYPos, float fZPos, BOOL bFixed)
{
	m_vPos[0] = fXPos;
	m_vPos[1] = fYPos;
	m_vPos[2] = fZPos;
	m_byState = bFixed ? PVS_FIXEDPOS : PVS_NORMAL;
}

void CPhysicsVertex::UpdateForce(unsigned int iKey, DWORD dwType, float fWind)
{
	(void)iKey;
	(void)dwType;
	(void)fWind;
}

void CPhysicsVertex::AddToForce(float fXForce, float fYForce, float fZForce)
{
	m_vForce[0] += fXForce;
	m_vForce[1] += fYForce;
	m_vForce[2] += fZForce;
}

void CPhysicsVertex::Move(float fTime)
{
	(void)fTime;
}

void CPhysicsVertex::GetPosition(vec3_t* pPos)
{
	if (pPos != NULL)
	{
		VectorCopy(m_vPos, *pPos);
	}
}

float CPhysicsVertex::GetDistance(CPhysicsVertex* pVertex2, vec3_t* pDistance)
{
	if (pVertex2 == NULL)
	{
		if (pDistance != NULL)
		{
			Vector(0.0f, 0.0f, 0.0f, *pDistance);
		}
		return 0.0f;
	}

	vec3_t vDelta;
	vDelta[0] = pVertex2->m_vPos[0] - m_vPos[0];
	vDelta[1] = pVertex2->m_vPos[1] - m_vPos[1];
	vDelta[2] = pVertex2->m_vPos[2] - m_vPos[2];

	if (pDistance != NULL)
	{
		VectorCopy(vDelta, *pDistance);
	}

	return sqrtf(vDelta[0] * vDelta[0] + vDelta[1] * vDelta[1] + vDelta[2] * vDelta[2]);
}

BOOL CPhysicsVertex::KeepLength(CPhysicsVertex* pVertex2, float* pfLength)
{
	(void)pVertex2;
	(void)pfLength;
	return FALSE;
}

void CPhysicsVertex::AddOneTimeMoveToKeepLength(CPhysicsVertex* pVertex2, float fLength)
{
	(void)pVertex2;
	(void)fLength;
}

void CPhysicsVertex::DoOneTimeMove(void)
{
}

void CPhysicsVertex::AddOneTimeMove(vec3_t vMove)
{
	(void)vMove;
}

CChangeRingManager::CChangeRingManager()
{
}

CChangeRingManager::~CChangeRingManager()
{
}

void CChangeRingManager::LoadItemModel()
{
}

void CChangeRingManager::LoadItemTexture()
{
}

bool CChangeRingManager::CheckDarkLordHair(int iType)
{
	(void)iType;
	return false;
}

bool CChangeRingManager::CheckDarkCloak(int iClass, int iType)
{
	(void)iClass;
	(void)iType;
	return false;
}

bool CChangeRingManager::CheckChangeRing(short RingType)
{
	(void)RingType;
	return false;
}

bool CChangeRingManager::CheckRepair(int iType)
{
	(void)iType;
	return true;
}

bool CChangeRingManager::CheckMoveMap(short sLeftRingType, short sRightRingType)
{
	(void)sLeftRingType;
	(void)sRightRingType;
	return true;
}

bool CChangeRingManager::CheckBanMoveIcarusMap(short sLeftRingType, short sRightRingType)
{
	(void)sLeftRingType;
	(void)sRightRingType;
	return false;
}

CXmasEvent::CXmasEvent(void)
	: m_iEffectID(0)
{
}

CXmasEvent::~CXmasEvent(void)
{
}

void CXmasEvent::LoadXmasEvent()
{
}

void CXmasEvent::LoadXmasEventEffect()
{
}

void CXmasEvent::LoadXmasEventItem()
{
}

void CXmasEvent::LoadXmasEventSound()
{
}

void CXmasEvent::CreateXmasEventEffect(CHARACTER* pCha, OBJECT* pObj, int iType)
{
	(void)pCha;
	(void)pObj;
	(void)iType;
}

void CXmasEvent::GenID()
{
}

CNewYearsDayEvent::CNewYearsDayEvent()
{
}

CNewYearsDayEvent::~CNewYearsDayEvent()
{
}

void CNewYearsDayEvent::LoadModel()
{
}

void CNewYearsDayEvent::LoadSound()
{
}

CHARACTER* CNewYearsDayEvent::CreateMonster(int iType, int iPosX, int iPosY, int iKey)
{
	(void)iType;
	(void)iPosX;
	(void)iPosY;
	(void)iKey;
	return NULL;
}

bool CNewYearsDayEvent::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
	(void)c;
	(void)o;
	(void)b;
	return false;
}

C09SummerEvent::C09SummerEvent()
{
}

C09SummerEvent::~C09SummerEvent()
{
}

void C09SummerEvent::LoadModel()
{
}

void C09SummerEvent::LoadSound()
{
}

CHARACTER* C09SummerEvent::CreateMonster(int iType, int iPosX, int iPosY, int iKey)
{
	(void)iType;
	(void)iPosX;
	(void)iPosY;
	(void)iKey;
	return NULL;
}

bool C09SummerEvent::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
	(void)c;
	(void)o;
	(void)b;
	return false;
}

void SetAction_Fenrir_Walk(CHARACTER* c, OBJECT* o)
{
	(void)c;
	(void)o;
}

void SetAction_Fenrir_Run(CHARACTER* c, OBJECT* o)
{
	(void)c;
	(void)o;
}

void SetAction_Fenrir_Damage(CHARACTER* c, OBJECT* o)
{
	(void)c;
	(void)o;
}

CKanturuDirection::CKanturuDirection()
{
	Init();
}

CKanturuDirection::~CKanturuDirection()
{
}

void CKanturuDirection::Init()
{
	m_iKanturuState = 0;
	m_iMayaState = 0;
	m_iNightmareState = 0;
	m_bKanturuDirection = false;
	m_bMayaDie = false;
	m_bMayaAppear = false;
	m_bDirectionEnd = false;
}

bool CKanturuDirection::IsKanturuDirection()
{
	return m_bKanturuDirection;
}

bool CKanturuDirection::IsKanturu3rdTimer()
{
	return false;
}

bool CKanturuDirection::IsMayaScene()
{
	return false;
}

void CKanturuDirection::GetKanturuAllState(BYTE State, BYTE DetailState)
{
	(void)State;
	(void)DetailState;
}

void CKanturuDirection::KanturuAllDirection()
{
}

bool CKanturuDirection::GetMayaExplotion()
{
	return m_bMayaDie;
}

void CKanturuDirection::SetMayaExplotion(bool MayaDie)
{
	m_bMayaDie = MayaDie;
}

bool CKanturuDirection::GetMayaAppear()
{
	return m_bMayaAppear;
}

void CKanturuDirection::SetMayaAppear(bool MayaDie)
{
	m_bMayaAppear = MayaDie;
}

namespace battleCastle
{
	void SetBuildTimeLocation(OBJECT* o)
	{
		(void)o;
	}

	bool SetCurrentAction_BattleCastleMonster(CHARACTER* c, OBJECT* o)
	{
		(void)c;
		(void)o;
		return false;
	}

	bool AttackEffect_BattleCastleMonster(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}

	bool StopBattleCastleMonster(CHARACTER* c, OBJECT* o)
	{
		(void)c;
		(void)o;
		return false;
	}

	void RenderMonsterHitEffect(OBJECT* o)
	{
		(void)o;
	}

	bool MoveBattleCastleMonsterVisual(OBJECT* o, BMD* b)
	{
		(void)o;
		(void)b;
		return false;
	}
}

namespace M31HuntingGround
{
	bool SetCurrentActionHuntingGroundMonster(CHARACTER* pCharacter, OBJECT* pObject)
	{
		(void)pCharacter;
		(void)pObject;
		return false;
	}

	bool AttackEffectHuntingGroundMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
	{
		(void)pCharacter;
		(void)pObject;
		(void)pModel;
		return false;
	}

	bool MoveHuntingGroundMonsterVisual(OBJECT* pObject, BMD* pModel)
	{
		(void)pObject;
		(void)pModel;
		return false;
	}

	void MoveHuntingGroundBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
	{
		(void)pCharacter;
		(void)pObject;
		(void)pModel;
	}

	bool RenderHuntingGroundMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
	{
		(void)pCharacter;
		(void)pObject;
		(void)pModel;
		return false;
	}
}

namespace M33Aida
{
	bool SetCurrentActionAidaMonster(CHARACTER* pCharacter, OBJECT* pObject)
	{
		(void)pCharacter;
		(void)pObject;
		return false;
	}

	bool AttackEffectAidaMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
	{
		(void)pCharacter;
		(void)pObject;
		(void)pModel;
		return false;
	}

	bool MoveAidaMonsterVisual(OBJECT* pObject, BMD* pModel)
	{
		(void)pObject;
		(void)pModel;
		return false;
	}

	void MoveAidaBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
	{
		(void)pCharacter;
		(void)pObject;
		(void)pModel;
	}

	bool RenderAidaMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
	{
		(void)pCharacter;
		(void)pObject;
		(void)pModel;
		return false;
	}
}

namespace M34CryWolf1st
{
	bool SetCurrentActionCrywolfMonster(CHARACTER* c, OBJECT* o)
	{
		(void)c;
		(void)o;
		return false;
	}

	bool AttackEffectCryWolf1stMonster(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}

	bool MoveCryWolf1stMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}

	void MoveCryWolf1stBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
	}

	bool RenderCryWolf1stMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}
}

namespace M34CryingWolf2nd
{
	bool AttackEffectCryingWolf2ndMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
	{
		(void)pCharacter;
		(void)pObject;
		(void)pModel;
		return false;
	}

	bool MoveCryingWolf2ndMonsterVisual(OBJECT* pObject, BMD* pModel)
	{
		(void)pObject;
		(void)pModel;
		return false;
	}

	void MoveCryingWolf2ndBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
	{
		(void)pCharacter;
		(void)pObject;
		(void)pModel;
	}

	bool RenderCryingWolf2ndMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
	{
		(void)pCharacter;
		(void)pObject;
		(void)pModel;
		return false;
	}
}

namespace M37Kanturu1st
{
	bool SetCurrentActionKanturu1stMonster(CHARACTER* c, OBJECT* o)
	{
		(void)c;
		(void)o;
		return false;
	}

	bool AttackEffectKanturu1stMonster(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}

	bool MoveKanturu1stMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}

	void MoveKanturu1stBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
	}

	bool RenderKanturu1stMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}
}

namespace M38Kanturu2nd
{
	bool Set_CurrentAction_Kanturu2nd_Monster(CHARACTER* c, OBJECT* o)
	{
		(void)c;
		(void)o;
		return false;
	}

	bool AttackEffect_Kanturu2nd_Monster(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}

	bool Move_Kanturu2nd_MonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}

	void Move_Kanturu2nd_BlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
	}

	bool Render_Kanturu2nd_MonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}
}

namespace M39Kanturu3rd
{
	bool SetCurrentActionKanturu3rdMonster(CHARACTER* c, OBJECT* o)
	{
		(void)c;
		(void)o;
		return false;
	}

	bool AttackEffectKanturu3rdMonster(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}

	bool MoveKanturu3rdMonsterVisual(OBJECT* o, BMD* b)
	{
		(void)o;
		(void)b;
		return false;
	}

	void MayaSceneMayaAction(BYTE Skill)
	{
		(void)Skill;
	}

	void MoveKanturu3rdBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
	}

	bool RenderKanturu3rdMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}
}

namespace SEASON3A
{
	CGM3rdChangeUp::CGM3rdChangeUp()
		: m_nDarkElfAppearance(false)
	{
	}

	CGM3rdChangeUp::~CGM3rdChangeUp()
	{
	}

	CGM3rdChangeUp& CGM3rdChangeUp::Instance()
	{
		static CGM3rdChangeUp s_instance;
		return s_instance;
	}

	bool CGM3rdChangeUp::IsBalgasBarrackMap()
	{
		return false;
	}

	bool CGM3rdChangeUp::IsBalgasRefugeMap()
	{
		return false;
	}

	bool CGM3rdChangeUp::CreateBalgasBarrackObject(OBJECT* pObject)
	{
		(void)pObject;
		return false;
	}

	bool CGM3rdChangeUp::CreateBalgasRefugeObject(OBJECT* pObject)
	{
		(void)pObject;
		return false;
	}

	bool CGM3rdChangeUp::MoveObject(OBJECT* pObject)
	{
		(void)pObject;
		return false;
	}

	bool CGM3rdChangeUp::RenderObjectVisual(OBJECT* pObject, BMD* pModel)
	{
		(void)pObject;
		(void)pModel;
		return false;
	}

	bool CGM3rdChangeUp::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
	{
		(void)o;
		(void)b;
		(void)ExtraMon;
		return false;
	}

	void CGM3rdChangeUp::RenderAfterObjectMesh(OBJECT* o, BMD* b)
	{
		(void)o;
		(void)b;
	}

	bool CGM3rdChangeUp::CreateFireSnuff(PARTICLE* o)
	{
		(void)o;
		return false;
	}

	void CGM3rdChangeUp::PlayEffectSound(OBJECT* o)
	{
		(void)o;
	}

	void CGM3rdChangeUp::PlayBGM()
	{
	}

	CHARACTER* CGM3rdChangeUp::CreateBalgasBarrackMonster(int iType, int PosX, int PosY, int Key)
	{
		(void)iType;
		(void)PosX;
		(void)PosY;
		(void)Key;
		return NULL;
	}

	bool CGM3rdChangeUp::SetCurrentActionBalgasBarrackMonster(CHARACTER* c, OBJECT* o)
	{
		(void)c;
		(void)o;
		return false;
	}

	bool CGM3rdChangeUp::AttackEffectBalgasBarrackMonster(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}

	bool CGM3rdChangeUp::MoveBalgasBarrackMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}

	void CGM3rdChangeUp::MoveBalgasBarrackBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
	}

	bool CGM3rdChangeUp::RenderMonsterObjectMesh(OBJECT* o, BMD* b, int ExtraMon)
	{
		(void)o;
		(void)b;
		(void)ExtraMon;
		return false;
	}

	bool CGM3rdChangeUp::RenderBalgasBarrackMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}

	CursedTemple::CursedTemple()
	{
		Initialize();
	}

	CursedTemple::~CursedTemple()
	{
		Destroy();
	}

	void CursedTemple::Initialize()
	{
		m_IsTalkEnterNpc = false;
		m_InterfaceState = false;
		m_HolyItemPlayerIndex = 0;
		m_CursedTempleState = eCursedTempleState_None;
		m_AlliedPoint = 0;
		m_IllusionPoint = 0;
		m_ShowAlliedPointEffect = false;
		m_ShowIllusionPointEffect = false;
		m_bGaugebarEnabled = false;
		m_fGaugebarCloseTimer = 0.0f;
	}

	void CursedTemple::Destroy()
	{
	}

	CursedTemple* CursedTemple::GetInstance()
	{
		static CursedTemple s_instance;
		return &s_instance;
	}

	void CursedTemple::Process()
	{
	}

	void CursedTemple::Draw()
	{
	}

	bool CursedTemple::GetInterfaceState(int type, int subtype)
	{
		(void)type;
		(void)subtype;
		return m_InterfaceState;
	}

	void CursedTemple::SetInterfaceState(bool state, int subtype)
	{
		(void)subtype;
		m_InterfaceState = state;
	}

	bool CursedTemple::IsHolyItemPickState()
	{
		return false;
	}

	bool CursedTemple::IsPartyMember(DWORD selectcharacterindex)
	{
		(void)selectcharacterindex;
		return false;
	}

	void CursedTemple::ReceiveCursedTempleInfo(BYTE* ReceiveBuffer)
	{
		(void)ReceiveBuffer;
	}

	void CursedTemple::ReceiveCursedTempleState(const eCursedTempleState state)
	{
		m_CursedTempleState = state;
	}

	bool CursedTemple::CreateObject(OBJECT* o)
	{
		(void)o;
		return false;
	}

	CHARACTER* CursedTemple::CreateCharacters(int iType, int iPosX, int iPosY, int iKey)
	{
		(void)iType;
		(void)iPosX;
		(void)iPosY;
		(void)iKey;
		return NULL;
	}

	bool CursedTemple::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
	{
		(void)c;
		(void)o;
		return false;
	}

	bool CursedTemple::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}

	void CursedTemple::PlayBGM()
	{
	}

	void CursedTemple::ResetCursedTemple()
	{
		Initialize();
	}

	bool CursedTemple::MoveObject(OBJECT* o)
	{
		(void)o;
		return false;
	}

	void CursedTemple::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
	}

	bool CursedTemple::MoveMonsterVisual(OBJECT* o, BMD* b)
	{
		(void)o;
		(void)b;
		return false;
	}

	void CursedTemple::MoveMonsterSoundVisual(OBJECT* o, BMD* b)
	{
		(void)o;
		(void)b;
	}

	bool CursedTemple::RenderObject_AfterCharacter(OBJECT* o, BMD* b)
	{
		(void)o;
		(void)b;
		return false;
	}

	bool CursedTemple::RenderObjectVisual(OBJECT* o, BMD* b)
	{
		(void)o;
		(void)b;
		return false;
	}

	bool CursedTemple::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}

	bool CursedTemple::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
	{
		(void)o;
		(void)b;
		(void)ExtraMon;
		return false;
	}

	void CursedTemple::UpdateTempleSystemMsg(int _Value)
	{
		(void)_Value;
	}

	void CursedTemple::SetGaugebarEnabled(bool bFlag)
	{
		m_bGaugebarEnabled = bFlag;
	}

	void CursedTemple::SetGaugebarCloseTimer()
	{
		m_fGaugebarCloseTimer = 0.0f;
	}

	bool CursedTemple::IsGaugebarEnabled()
	{
		return m_bGaugebarEnabled;
	}
}

bool AttackEffect_HellasMonster(CHARACTER* c, CHARACTER* tc, OBJECT* o, OBJECT* to, BMD* b)
{
	(void)c;
	(void)tc;
	(void)o;
	(void)to;
	(void)b;
	return false;
}

int CreateParticle(int Type, vec3_t Position, vec3_t Angle, vec3_t Light, int SubType, float Scale, OBJECT* Owner)
{
	(void)Type;
	(void)Position;
	(void)Angle;
	(void)Light;
	(void)SubType;
	(void)Scale;
	(void)Owner;
	return 0;
}

int CreateParticleFpsChecked(int Type, vec3_t Position, vec3_t Angle, vec3_t Light, int SubType, float Scale, OBJECT* Owner)
{
	return CreateParticle(Type, Position, Angle, Light, SubType, Scale, Owner);
}

void CreateInferno(vec3_t Position, int SubType)
{
	(void)Position;
	(void)SubType;
}

void CreateJoint(int Type, vec3_t Position, vec3_t TargetPosition, vec3_t Angle, int SubType, OBJECT* Target, float Scale,
	short PKKey, WORD SkillIndex, WORD SkillSerialNum, int iChaIndex, const float* vColor, short int sTargetIndex)
{
	(void)Type;
	(void)Position;
	(void)TargetPosition;
	(void)Angle;
	(void)SubType;
	(void)Target;
	(void)Scale;
	(void)PKKey;
	(void)SkillIndex;
	(void)SkillSerialNum;
	(void)iChaIndex;
	(void)vColor;
	(void)sTargetIndex;
}

void CreateArrows(CHARACTER* c, OBJECT* o, OBJECT* to, WORD SkillIndex, WORD Skill, WORD SKKey)
{
	(void)c;
	(void)o;
	(void)to;
	(void)SkillIndex;
	(void)Skill;
	(void)SKKey;
}

void CreateBomb(vec3_t p, bool Exp, int SubType)
{
	(void)p;
	(void)Exp;
	(void)SubType;
}

void MoveParticle(OBJECT* o, int Turn)
{
	(void)o;
	(void)Turn;
}

void MoveParticle(OBJECT* o, vec3_t angle)
{
	(void)o;
	(void)angle;
}

void CreateBlood(OBJECT* o)
{
	(void)o;
}

void CreateBlur(CHARACTER* Owner, vec3_t p1, vec3_t p2, vec3_t Light, int Type, bool Short, int SubType)
{
	(void)Owner;
	(void)p1;
	(void)p2;
	(void)Light;
	(void)Type;
	(void)Short;
	(void)SubType;
}

void DeleteJoint(int Type, OBJECT* Target, int SubType)
{
	(void)Type;
	(void)Target;
	(void)SubType;
}

void CreateSpark(int Type, CHARACTER* tc, vec3_t Position, vec3_t Angle)
{
	(void)Type;
	(void)tc;
	(void)Position;
	(void)Angle;
}

float CreateAngle2D(const vec3_t from, const vec2_t to)
{
	(void)from;
	(void)to;
	return 0.0f;
}

bool SearchJoint(int Type, OBJECT* Target, int SubType)
{
	(void)Type;
	(void)Target;
	(void)SubType;
	return false;
}

void CreateFire(int Type, OBJECT* o, float x, float y, float z)
{
	(void)Type;
	(void)o;
	(void)x;
	(void)y;
	(void)z;
}

float TurnAngle2(float angle, float a, float d)
{
	(void)a;
	(void)d;
	return angle;
}

float FarAngle(float angle1, float angle2, bool abs)
{
	(void)angle2;
	(void)abs;
	return angle1;
}

float MoveHumming(vec3_t Position, vec3_t Angle, vec3_t TargetPosition, float Turn)
{
	(void)Position;
	(void)Angle;
	(void)TargetPosition;
	(void)Turn;
	return 0.0f;
}

void MoveHead(CHARACTER* c)
{
	(void)c;
}

void CreateForce(OBJECT* o, vec3_t Pos)
{
	(void)o;
	(void)Pos;
}

void CreatePointer(int Type, vec3_t Position, float Angle, vec3_t Light, float Scale)
{
	(void)Type;
	(void)Position;
	(void)Angle;
	(void)Light;
	(void)Scale;
}

int CreateSprite(int Type, vec3_t Position, float Scale, vec3_t Light, OBJECT* Owner, float Rotation, int SubType)
{
	(void)Type;
	(void)Position;
	(void)Scale;
	(void)Light;
	(void)Owner;
	(void)Rotation;
	(void)SubType;
	return 0;
}

namespace SEASON3B
{
	CNewUISystem::CNewUISystem()
	{
		std::memset(this, 0, sizeof(*this));
	}

	CNewUISystem::~CNewUISystem()
	{
	}

	CNewUISystem* CNewUISystem::GetInstance()
	{
		static CNewUISystem s_Instance;
		return &s_Instance;
	}

	CNewUIMiniMap* CNewUISystem::GetUI_pNewUIMiniMap() const
	{
		return m_pNewMiniMap;
	}

	CNewUICursedTempleEnter* CNewUISystem::GetUI_NewCursedTempleEnterWindow() const
	{
		return m_pNewCursedTempleEnterWindow;
	}

	CNewUICursedTempleSystem* CNewUISystem::GetUI_NewCursedTempleWindow() const
	{
		return m_pNewCursedTempleWindow;
	}

	CNewUICursedTempleResult* CNewUISystem::GetUI_NewCursedTempleResultWindow() const
	{
		return m_pNewCursedTempleResultWindow;
	}

	bool CNewUICursedTempleSystem::CheckInventoryHolyItem(CHARACTER* c)
	{
		(void)c;
		return false;
	}
}

void SEASON3B::CNewUIMiniMap::SetBtnPos(int Num, float x, float y, float nx, float ny)
{
	(void)Num;
	(void)x;
	(void)y;
	(void)nx;
	(void)ny;
}

void CreateEffect(int Type, vec3_t Position, vec3_t Angle, vec3_t Light, int SubType, OBJECT* Target, short PKKey,
	WORD SkillIndex, WORD Skill, WORD SkillSerialNum, float Scale, short int sTargetIndex)
{
	(void)Type;
	(void)Position;
	(void)Angle;
	(void)Light;
	(void)SubType;
	(void)Target;
	(void)PKKey;
	(void)SkillIndex;
	(void)Skill;
	(void)SkillSerialNum;
	(void)Scale;
	(void)sTargetIndex;
}

void CreateEffectFpsChecked(int Type, vec3_t Position, vec3_t Angle, vec3_t Light, int SubType, OBJECT* Target, short PKKey,
	WORD SkillIndex, WORD Skill, WORD SkillSerialNum, float Scale, short int sTargetIndex)
{
	CreateEffect(Type, Position, Angle, Light, SubType, Target, PKKey, SkillIndex, Skill, SkillSerialNum, Scale, sTargetIndex);
}

bool SetCurrentAction_HellasMonster(CHARACTER* c, OBJECT* o)
{
	(void)c;
	(void)o;
	return false;
}

namespace SEASON3B
{
	CPartyManager* CPartyManager::m_pPartyManager = NULL;

	CPartyManager::CPartyManager()
	{
	}

	CPartyManager::~CPartyManager()
	{
	}

	bool CPartyManager::Create()
	{
		return true;
	}

	void CPartyManager::Release()
	{
	}

	bool CPartyManager::Update()
	{
		return true;
	}

	bool CPartyManager::Render()
	{
		return true;
	}

	void CPartyManager::SearchPartyMember()
	{
	}

	bool CPartyManager::IsPartyMember(int index)
	{
		(void)index;
		return false;
	}

	bool CPartyManager::IsPartyMemberChar(CHARACTER* c)
	{
		(void)c;
		return false;
	}

	CPartyManager* CPartyManager::GetInstance()
	{
		static CPartyManager s_partyManager;
		return &s_partyManager;
	}
}

int getTargetCharacterKey(CHARACTER* c, int selected)
{
	(void)c;
	return selected;
}

int FindHotKey(int Skill)
{
	return Skill;
}

void AddObjectDescription(char* Text, vec3_t position)
{
	(void)Text;
	(void)position;
}

void Alpha(OBJECT* o)
{
	(void)o;
}

void AddWaterWave(int x, int y, int Range, int Height)
{
	(void)x;
	(void)y;
	(void)Range;
	(void)Height;
}

#if !defined(MU_ANDROID_HAS_DSPLAYSOUND_RUNTIME)
void StopBuffer(int Buffer, BOOL bResetPosition)
{
	(void)Buffer;
	(void)bResetPosition;
}
#endif

bool MoveHellasMonsterVisual(OBJECT* o, BMD* b)
{
	(void)o;
	(void)b;
	return false;
}

void BoneManager::RegisterBone(CHARACTER* pCharacter, const std::string& name, int nBone)
{
	(void)pCharacter;
	(void)name;
	(void)nBone;
}

void BoneManager::UnregisterBone(CHARACTER* pCharacter, const std::string& name)
{
	(void)pCharacter;
	(void)name;
}

void BoneManager::UnregisterBone(CHARACTER* pCharacter)
{
	(void)pCharacter;
}

void BoneManager::UnregisterAll()
{
}

CHARACTER* BoneManager::GetOwnCharacter(OBJECT* pObject, const std::string& name)
{
	(void)pObject;
	(void)name;
	return NULL;
}

int BoneManager::GetBoneNumber(OBJECT* pObject, const std::string& name)
{
	(void)pObject;
	(void)name;
	return -1;
}

bool BoneManager::GetBonePosition(OBJECT* pObject, const std::string& name, OUT vec3_t Position)
{
	(void)pObject;
	(void)name;
	if (Position != NULL)
	{
		Vector(0.0f, 0.0f, 0.0f, Position);
	}
	return false;
}

bool BoneManager::GetBonePosition(OBJECT* pObject, const std::string& name, IN vec3_t Relative, OUT vec3_t Position)
{
	(void)pObject;
	(void)name;
	(void)Relative;
	if (Position != NULL)
	{
		Vector(0.0f, 0.0f, 0.0f, Position);
	}
	return false;
}

void CHARACTER_MACHINE::CalculateDamage()
{
}

void CHARACTER_MACHINE::CalculateMagicDamage()
{
}

void CHARACTER_MACHINE::CalculateCurseDamage()
{
}

void CHARACTER_MACHINE::CalculateAttackSpeed()
{
}

int CurrentSkill = 0;
int HeroTile = 0;

CSkillManager gSkillManager;

CSkillManager::CSkillManager()
{
}

CSkillManager::~CSkillManager()
{
}

bool CSkillManager::FindHeroSkill(ActionSkillType eSkillType)
{
	(void)eSkillType;
	return false;
}

void CSkillManager::GetSkillInformation(int iType, int iLevel, char* lpszName, int* piMana, int* piDistance, int* piSkillMana)
{
	(void)iType;
	(void)iLevel;
	if (lpszName != NULL)
	{
		lpszName[0] = '\0';
	}
	if (piMana != NULL)
	{
		*piMana = 0;
	}
	if (piDistance != NULL)
	{
		*piDistance = 0;
	}
	if (piSkillMana != NULL)
	{
		*piSkillMana = 0;
	}
}

void CSkillManager::GetSkillInformation_Energy(int iType, int* piEnergy)
{
	(void)iType;
	if (piEnergy != NULL)
	{
		*piEnergy = 0;
	}
}

void CSkillManager::GetSkillInformation_Charisma(int iType, int* piCharisma)
{
	(void)iType;
	if (piCharisma != NULL)
	{
		*piCharisma = 0;
	}
}

float CSkillManager::GetSkillDistance(int Index, CHARACTER* c)
{
	(void)Index;
	(void)c;
	return 0.0f;
}

void CSkillManager::GetSkillInformation_Damage(int iType, int* piDamage)
{
	(void)iType;
	if (piDamage != NULL)
	{
		*piDamage = 0;
	}
}

bool CSkillManager::CheckSkillDelay(int SkillIndex)
{
	(void)SkillIndex;
	return true;
}

void CSkillManager::CalcSkillDelay(int time)
{
	(void)time;
}

BYTE CSkillManager::GetSkillMasteryType(int iType)
{
	(void)iType;
	return 0;
}

int CSkillManager::MasterSkillToBaseSkillIndex(int iMasterSkillIndex)
{
	return iMasterSkillIndex;
}

bool CSkillManager::skillVScharactorCheck(const DemendConditionInfo& basicInfo, const DemendConditionInfo& heroInfo)
{
	(void)basicInfo;
	(void)heroInfo;
	return true;
}

bool CSkillManager::DemendConditionCheckSkill(WORD skilltype)
{
	(void)skilltype;
	return true;
}

bool CheckAttack_Fenrir(CHARACTER* c)
{
	(void)c;
	return false;
}

bool PathFinding2(int sx, int sy, int tx, int ty, PATH_t* a, float fDistance, int iDefaultWall)
{
	(void)sx;
	(void)sy;
	(void)tx;
	(void)ty;
	(void)a;
	(void)fDistance;
	(void)iDefaultWall;
	return false;
}

bool MovePath(CHARACTER* c, bool Turn)
{
	(void)c;
	(void)Turn;
	return false;
}

void MoveBlurs()
{
}

bool battleCastle::MoveBattleCastleMonster(CHARACTER* c, OBJECT* o)
{
	(void)c;
	(void)o;
	return false;
}

void ItemObjectAttribute(OBJECT* o)
{
	(void)o;
}

void BodyLight(OBJECT* o, BMD* b)
{
	b->LightEnable = o->LightEnable;
	vec3_t Light;
	RequestTerrainLight(o->Position[0], o->Position[1], Light);
	if (o->LightEnable)
	{
		VectorAdd(Light, o->Light, b->BodyLight);
	}
	else
	{
		VectorScale(Light, 0.1f, Light);
		VectorAdd(Light, o->Light, b->BodyLight);
	}
}

void RenderPartObjectBody(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
	(void)Type;
	(void)RenderType;
	{
		static int diag = 0;
		if (diag < 10)
		{
			++diag;
			__android_log_print(ANDROID_LOG_INFO, "mu_char_render",
				"RenderPartObjectBody: Type=%d NumMeshs=%d Alpha=%.2f BlendMesh=%d HiddenMesh=%d",
				Type, b->NumMeshs, o->Alpha, o->BlendMesh, o->HiddenMesh);
		}
	}
	b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
		o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
}

void RenderPartObjectEffect(OBJECT* o, int Type, vec3_t Light, float Alpha, int Level, int Option1, int ExtOption, int Select, int RenderType)
{
	(void)Level;
	(void)Option1;
	(void)ExtOption;
	(void)Select;

	BMD* b = &Models[Type];

	if (o->EnableShadow)
	{
		return;
	}

	VectorCopy(Light, b->BodyLight);
	RenderPartObjectBody(b, o, Type, Alpha, RenderType);
}

void RenderPartObject(OBJECT* o, int Type, void* p2, vec3_t Light, float Alpha, int Level, int Option1, int ExtOption, bool GlobalTransform, bool HideSkin, bool Translate, int Select, int RenderType)
{
	(void)p2;
	(void)Select;

	if (Alpha <= 0.01f)
	{
		return;
	}

	BMD* b = &Models[Type];
	{
		static int diag = 0;
		if (diag < 20)
		{
			++diag;
			__android_log_print(ANDROID_LOG_INFO, "mu_char_render",
				"RenderPartObject: Type=%d NumActions=%d NumMeshs=%d NumBones=%d Alpha=%.2f GlobalTrans=%d Scale=%.2f",
				Type, b->NumActions, b->NumMeshs, b->NumBones, Alpha, GlobalTransform, o->Scale);
		}
	}
	b->HideSkin = HideSkin;
	b->BodyScale = o->Scale;
	b->ContrastEnable = o->ContrastEnable;
	b->LightEnable = o->LightEnable;
	VectorCopy(o->Position, b->BodyOrigin);

	BodyLight(o, b);

	if (GlobalTransform)
	{
		b->Transform(BoneTransform, o->BoundingBoxMin, o->BoundingBoxMax, &o->OBB, Translate);
	}
	else
	{
		b->Transform(o->BoneTransform, o->BoundingBoxMin, o->BoundingBoxMax, &o->OBB, Translate);
	}

	RenderPartObjectEffect(o, Type, Light, Alpha, Level, Option1, ExtOption, Select, RenderType);
}

bool Calc_RenderObject(OBJECT* o, bool Translate, int Select, int ExtraMon)
{
	(void)ExtraMon;

	if (o->Alpha < 0.01f)
	{
		return false;
	}

	BMD* b = &Models[o->Type];
	b->BodyHeight = 0.f;
	b->ContrastEnable = o->ContrastEnable;
	BodyLight(o, b);
	b->BodyScale = o->Scale;
	b->CurrentAction = o->CurrentAction;
	VectorCopy(o->Position, b->BodyOrigin);

	if (o->EnableBoneMatrix)
	{
		b->Animation(o->BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, !Translate);
	}
	else
	{
		b->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, !Translate);
	}

	b->Transform(o->BoneTransform, o->BoundingBoxMin, o->BoundingBoxMax, &o->OBB, Translate);
	return true;
}

bool Calc_ObjectAnimation(OBJECT* o, bool Translate, int Select)
{
	(void)Select;

	if (o->Alpha < 0.01f)
	{
		return false;
	}

	BMD* b = &Models[o->Type];
	{
		static int diag = 0;
		if (diag < 5)
		{
			++diag;
			__android_log_print(ANDROID_LOG_INFO, "mu_char_render",
				"Calc_ObjectAnimation: Type=%d NumActions=%d CurrentAction=%d AnimFrame=%.2f Scale=%.2f EnableBone=%d Pos=(%.1f,%.1f,%.1f)",
				o->Type, b->NumActions, o->CurrentAction, o->AnimationFrame, o->Scale,
				o->EnableBoneMatrix, o->Position[0], o->Position[1], o->Position[2]);
		}
	}
	b->BodyHeight = 0.f;
	b->ContrastEnable = o->ContrastEnable;
	BodyLight(o, b);
	b->BodyScale = o->Scale;
	b->CurrentAction = o->CurrentAction;
	VectorCopy(o->Position, b->BodyOrigin);

	if (o->EnableBoneMatrix)
	{
		b->Animation(o->BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, !Translate);
	}
	else
	{
		b->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, !Translate);
	}
	return true;
}

void Draw_RenderObject(OBJECT* o, bool Translate, int Select, int ExtraMon)
{
	(void)ExtraMon;

	BMD* b = &Models[o->Type];
	b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
		o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
}

void RenderObject(OBJECT* o, bool Translate, int Select, int ExtraMon)
{
	if (!Calc_RenderObject(o, Translate, Select, ExtraMon))
	{
		return;
	}
	Draw_RenderObject(o, Translate, Select, ExtraMon);
}

void RenderObject_AfterImage(OBJECT* o, bool Translate, int Select, int ExtraMon)
{
	(void)o;
	(void)Translate;
	(void)Select;
	(void)ExtraMon;
}

void RenderTerrainAlphaBitmap(int Texture, float xf, float yf, float SizeX, float SizeY, vec3_t Light, float Rotation, float Alpha, float Height)
{
	(void)Texture;
	(void)xf;
	(void)yf;
	(void)SizeX;
	(void)SizeY;
	(void)Light;
	(void)Rotation;
	(void)Alpha;
	(void)Height;
}

OBJECT Butterfles[MAX_BUTTERFLES];

namespace giPetManager
{
	void InitPetManager()
	{
	}

	void CreatePetDarkSpirit(CHARACTER* c)
	{
		(void)c;
	}

	void CreatePetDarkSpirit_Now(CHARACTER* c)
	{
		(void)c;
	}

	void MovePet(CHARACTER* c)
	{
		(void)c;
	}

	void RenderPet(CHARACTER* c)
	{
		(void)c;
	}

	void DeletePet(CHARACTER* c)
	{
		(void)c;
	}

	void InitItemBackup()
	{
	}

	void SetPetInfo(BYTE InvType, BYTE InvPos, PET_INFO* pPetinfo)
	{
		(void)InvType;
		(void)InvPos;
		(void)pPetinfo;
	}

	PET_INFO* GetPetInfo(ITEM* pItem)
	{
		(void)pItem;
		return &gs_PetInfo;
	}

	void CalcPetInfo(PET_INFO* pPetInfo)
	{
		(void)pPetInfo;
	}

	void SetPetItemConvert(ITEM* ip, PET_INFO* pPetInfo)
	{
		(void)ip;
		(void)pPetInfo;
	}

	DWORD GetPetItemValue(PET_INFO* pPetInfo)
	{
		(void)pPetInfo;
		return 0;
	}

	bool RequestPetInfo(int sx, int sy, ITEM* pItem)
	{
		(void)sx;
		(void)sy;
		(void)pItem;
		return false;
	}

	bool RenderPetItemInfo(int sx, int sy, ITEM* pItem, int iInvenType)
	{
		(void)sx;
		(void)sy;
		(void)pItem;
		(void)iInvenType;
		return false;
	}

	bool SelectPetCommand()
	{
		return false;
	}

	void MovePetCommand(CHARACTER* c)
	{
		(void)c;
	}

	bool SendPetCommand(CHARACTER* c, int Index)
	{
		(void)c;
		(void)Index;
		return false;
	}

	void SetPetCommand(CHARACTER* c, int Key, BYTE Cmd)
	{
		(void)c;
		(void)Key;
		(void)Cmd;
	}

	void SetAttack(CHARACTER* c, int Key, int attackType)
	{
		(void)c;
		(void)Key;
		(void)attackType;
	}

	bool RenderPetCmdInfo(int sx, int sy, int Type)
	{
		(void)sx;
		(void)sy;
		(void)Type;
		return false;
	}
}

bool IsIceCity()
{
	return false;
}

CCustomWing gCustomWing;
CCustomCape gCustomCape;
CMonsterEffect gMonsterEffect;

CCustomWing::CCustomWing()
	: m_Lua()
{
}

CCustomWing::~CCustomWing()
{
}

void CCustomWing::Init()
{
	m_CustomWingInfo.clear();
}

CUSTOM_WING_INFO* CCustomWing::GetInfoByItem(int ItemIndex)
{
	std::map<int, CUSTOM_WING_INFO>::iterator it = m_CustomWingInfo.find(ItemIndex);
	return it != m_CustomWingInfo.end() ? &it->second : NULL;
}

BOOL CCustomWing::CheckCustomWingByItem(int ItemIndex)
{
	return GetInfoByItem(ItemIndex) != NULL;
}

BOOL CCustomWing::CheckCustomWingByModelType(int ItemIndex, int ModelType)
{
	CUSTOM_WING_INFO* info = GetInfoByItem(ItemIndex);
	return info != NULL && info->ModelType == ModelType;
}

int CCustomWing::GetCustomWingDefense(int ItemIndex, int ItemLevel)
{
	(void)ItemIndex;
	(void)ItemLevel;
	return 0;
}

int CCustomWing::GetCustomWingIncDamage(int ItemIndex, int ItemLevel)
{
	(void)ItemIndex;
	(void)ItemLevel;
	return 0;
}

int CCustomWing::GetCustomWingDecDamage(int ItemIndex, int ItemLevel)
{
	(void)ItemIndex;
	(void)ItemLevel;
	return 0;
}

int CCustomWing::GetCustomWingOptionIndex(int ItemIndex, int OptionNumber)
{
	(void)ItemIndex;
	(void)OptionNumber;
	return 0;
}

int CCustomWing::GetCustomWingOptionValue(int ItemIndex, int OptionNumber)
{
	(void)ItemIndex;
	(void)OptionNumber;
	return 0;
}

int CCustomWing::GetCustomWingNewOptionIndex(int ItemIndex, int OptionNumber)
{
	(void)ItemIndex;
	(void)OptionNumber;
	return 0;
}

int CCustomWing::GetCustomWingNewOptionValue(int ItemIndex, int OptionNumber)
{
	(void)ItemIndex;
	(void)OptionNumber;
	return 0;
}

int CCustomWing::CheckCustomWingIsCape(int ItemIndex)
{
	CUSTOM_WING_INFO* info = GetInfoByItem(ItemIndex);
	return info != NULL ? info->IsCape : 0;
}

CCustomCape::CCustomCape()
	: m_Lua()
	, m_LuaLoadImage()
	, m_LuaBMD()
	, m_LuaCloth()
{
}

CCustomCape::~CCustomCape()
{
}

void CCustomCape::Init()
{
}

void CCustomCape::RestartLua()
{
}

void CCustomCape::LoadImageCape()
{
}

void CCustomCape::CreateCape(CPhysicsCloth* pCloth, CHARACTER* character, WORD ItemIndex, BYTE Class)
{
	(void)pCloth;
	(void)character;
	(void)ItemIndex;
	(void)Class;
}

void CCustomCape::RenderModel(BMD* BMDStruct, OBJECT* Object, DWORD ItemIndex)
{
	(void)BMDStruct;
	(void)Object;
	(void)ItemIndex;
}

void CCustomCape::CapeModelPosition(DWORD ItemIndex, float& posX, float& posY, float& posZ, float& matrixX, float& matrixY, float& matrixZ)
{
	(void)ItemIndex;
	posX = 0.0f;
	posY = 0.0f;
	posZ = 0.0f;
	matrixX = 0.0f;
	matrixY = 0.0f;
	matrixZ = 0.0f;
}

CMonsterEffect::CMonsterEffect()
	: m_Lua()
{
}

CMonsterEffect::~CMonsterEffect()
{
}

void CMonsterEffect::Init()
{
	m_MonsterEffect.clear();
}

CUSTOM_MONSTER_EFFECT* CMonsterEffect::getMonsterEffect(int Type)
{
	std::map<int, CUSTOM_MONSTER_EFFECT>::iterator it = m_MonsterEffect.find(Type);
	return it != m_MonsterEffect.end() ? &it->second : NULL;
}

void CMonsterEffect::CreateEffectMonster(OBJECT* ObjectStruct, BMD* pModel)
{
	(void)ObjectStruct;
	(void)pModel;
}

bool IsSantaTown()
{
	return false;
}

void DeleteParts(CHARACTER* c)
{
	(void)c;
}

void CreateChat(char* ID, const char* Text, CHARACTER* c, int Flag, int SetColor)
{
	(void)ID;
	(void)Text;
	(void)c;
	(void)Flag;
	(void)SetColor;
}

int CreateChat(char* ID, const char* Text, OBJECT* Owner, int Flag, int SetColor)
{
	(void)ID;
	(void)Text;
	(void)Owner;
	(void)Flag;
	(void)SetColor;
	return 0;
}

GMEmpireGuardian1::GMEmpireGuardian1()
	: m_iWeather(0)
	, m_bCurrentIsRage_Raymond(false)
	, m_bCurrentIsRage_Ercanne(false)
	, m_bCurrentIsRage_Daesuler(false)
	, m_bCurrentIsRage_Gallia(false)
{
}

GMEmpireGuardian1::~GMEmpireGuardian1()
{
}

GMEmpireGuardian1Ptr GMEmpireGuardian1::Make()
{
	return GMEmpireGuardian1Ptr(new GMEmpireGuardian1());
}

bool GMEmpireGuardian1::CreateObject(OBJECT* o) { (void)o; return false; }
bool GMEmpireGuardian1::MoveObject(OBJECT* o) { (void)o; return false; }
bool GMEmpireGuardian1::RenderObjectVisual(OBJECT* o, BMD* b) { (void)o; (void)b; return false; }
bool GMEmpireGuardian1::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon) { (void)o; (void)b; (void)ExtraMon; return false; }
void GMEmpireGuardian1::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon) { (void)o; (void)b; (void)ExtraMon; }
void GMEmpireGuardian1::RenderFrontSideVisual() {}
bool GMEmpireGuardian1::RenderMonster(OBJECT* o, BMD* b, bool ExtraMon) { (void)o; (void)b; (void)ExtraMon; return false; }
CHARACTER* GMEmpireGuardian1::CreateMonster(int iType, int PosX, int PosY, int Key) { (void)iType; (void)PosX; (void)PosY; (void)Key; return NULL; }
bool GMEmpireGuardian1::MoveMonsterVisual(OBJECT* o, BMD* b) { (void)o; (void)b; return false; }
void GMEmpireGuardian1::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; }
bool GMEmpireGuardian1::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; return false; }
bool GMEmpireGuardian1::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; return false; }
bool GMEmpireGuardian1::SetCurrentActionMonster(CHARACTER* c, OBJECT* o) { (void)c; (void)o; return false; }
bool GMEmpireGuardian1::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; return false; }
bool GMEmpireGuardian1::PlayMonsterSound(OBJECT* o) { (void)o; return false; }
void GMEmpireGuardian1::PlayObjectSound(OBJECT* o) { (void)o; }
void GMEmpireGuardian1::PlayBGM() {}
void GMEmpireGuardian1::Init() {}
void GMEmpireGuardian1::Destroy() {}
bool GMEmpireGuardian1::CreateRain(PARTICLE* o) { (void)o; return false; }

GMEmpireGuardian2::GMEmpireGuardian2()
	: m_bCurrentIsRage_Bermont(false)
{
}

GMEmpireGuardian2::~GMEmpireGuardian2()
{
}

GMEmpireGuardian2Ptr GMEmpireGuardian2::Make()
{
	return GMEmpireGuardian2Ptr(new GMEmpireGuardian2());
}

bool GMEmpireGuardian2::CreateObject(OBJECT* o) { (void)o; return false; }
bool GMEmpireGuardian2::MoveObject(OBJECT* o) { (void)o; return false; }
bool GMEmpireGuardian2::RenderObjectVisual(OBJECT* o, BMD* b) { (void)o; (void)b; return false; }
bool GMEmpireGuardian2::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon) { (void)o; (void)b; (void)ExtraMon; return false; }
void GMEmpireGuardian2::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon) { (void)o; (void)b; (void)ExtraMon; }
void GMEmpireGuardian2::RenderFrontSideVisual() {}
bool GMEmpireGuardian2::RenderMonster(OBJECT* o, BMD* b, bool ExtraMon) { (void)o; (void)b; (void)ExtraMon; return false; }
CHARACTER* GMEmpireGuardian2::CreateMonster(int iType, int PosX, int PosY, int Key) { (void)iType; (void)PosX; (void)PosY; (void)Key; return NULL; }
bool GMEmpireGuardian2::MoveMonsterVisual(OBJECT* o, BMD* b) { (void)o; (void)b; return false; }
void GMEmpireGuardian2::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; }
bool GMEmpireGuardian2::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; return false; }
bool GMEmpireGuardian2::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; return false; }
bool GMEmpireGuardian2::SetCurrentActionMonster(CHARACTER* c, OBJECT* o) { (void)c; (void)o; return false; }
bool GMEmpireGuardian2::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; return false; }
bool GMEmpireGuardian2::PlayMonsterSound(OBJECT* o) { (void)o; return false; }
void GMEmpireGuardian2::PlayObjectSound(OBJECT* o) { (void)o; }
void GMEmpireGuardian2::PlayBGM() {}
bool GMEmpireGuardian2::CreateRain(PARTICLE* o) { (void)o; return false; }
void GMEmpireGuardian2::SetWeather(int weather) { (void)weather; }
void GMEmpireGuardian2::Init() {}
void GMEmpireGuardian2::Destroy() {}

GMEmpireGuardian3::GMEmpireGuardian3()
	: m_bCurrentIsRage_Kato(false)
{
}

GMEmpireGuardian3::~GMEmpireGuardian3()
{
}

GMEmpireGuardian3Ptr GMEmpireGuardian3::Make()
{
	return GMEmpireGuardian3Ptr(new GMEmpireGuardian3());
}

bool GMEmpireGuardian3::CreateObject(OBJECT* o) { (void)o; return false; }
bool GMEmpireGuardian3::MoveObject(OBJECT* o) { (void)o; return false; }
bool GMEmpireGuardian3::RenderObjectVisual(OBJECT* o, BMD* b) { (void)o; (void)b; return false; }
bool GMEmpireGuardian3::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon) { (void)o; (void)b; (void)ExtraMon; return false; }
void GMEmpireGuardian3::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon) { (void)o; (void)b; (void)ExtraMon; }
void GMEmpireGuardian3::RenderFrontSideVisual() {}
bool GMEmpireGuardian3::RenderMonster(OBJECT* o, BMD* b, bool ExtraMon) { (void)o; (void)b; (void)ExtraMon; return false; }
CHARACTER* GMEmpireGuardian3::CreateMonster(int iType, int PosX, int PosY, int Key) { (void)iType; (void)PosX; (void)PosY; (void)Key; return NULL; }
bool GMEmpireGuardian3::MoveMonsterVisual(OBJECT* o, BMD* b) { (void)o; (void)b; return false; }
void GMEmpireGuardian3::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; }
bool GMEmpireGuardian3::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; return false; }
bool GMEmpireGuardian3::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; return false; }
bool GMEmpireGuardian3::SetCurrentActionMonster(CHARACTER* c, OBJECT* o) { (void)c; (void)o; return false; }
bool GMEmpireGuardian3::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; return false; }
bool GMEmpireGuardian3::PlayMonsterSound(OBJECT* o) { (void)o; return false; }
void GMEmpireGuardian3::PlayObjectSound(OBJECT* o) { (void)o; }
void GMEmpireGuardian3::PlayBGM() {}
void GMEmpireGuardian3::Init() {}
void GMEmpireGuardian3::Destroy() {}
bool GMEmpireGuardian3::CreateRain(PARTICLE* o) { (void)o; return false; }
void GMEmpireGuardian3::SetWeather(int weather) { (void)weather; }

GMEmpireGuardian4::GMEmpireGuardian4()
	: m_bCurrentIsRage_BossGaion(false)
	, m_bCurrentIsRage_Jerint(false)
{
}

GMEmpireGuardian4::~GMEmpireGuardian4()
{
}

GMEmpireGuardian4Ptr GMEmpireGuardian4::Make()
{
	return GMEmpireGuardian4Ptr(new GMEmpireGuardian4());
}

bool GMEmpireGuardian4::CreateObject(OBJECT* o) { (void)o; return false; }
bool GMEmpireGuardian4::MoveObject(OBJECT* o) { (void)o; return false; }
bool GMEmpireGuardian4::RenderObjectVisual(OBJECT* o, BMD* b) { (void)o; (void)b; return false; }
bool GMEmpireGuardian4::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon) { (void)o; (void)b; (void)ExtraMon; return false; }
void GMEmpireGuardian4::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon) { (void)o; (void)b; (void)ExtraMon; }
void GMEmpireGuardian4::RenderFrontSideVisual() {}
bool GMEmpireGuardian4::RenderMonster(OBJECT* o, BMD* b, bool ExtraMon) { (void)o; (void)b; (void)ExtraMon; return false; }
CHARACTER* GMEmpireGuardian4::CreateMonster(int iType, int PosX, int PosY, int Key) { (void)iType; (void)PosX; (void)PosY; (void)Key; return NULL; }
bool GMEmpireGuardian4::MoveMonsterVisual(OBJECT* o, BMD* b) { (void)o; (void)b; return false; }
void GMEmpireGuardian4::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; }
bool GMEmpireGuardian4::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; return false; }
bool GMEmpireGuardian4::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; return false; }
bool GMEmpireGuardian4::SetCurrentActionMonster(CHARACTER* c, OBJECT* o) { (void)c; (void)o; return false; }
bool GMEmpireGuardian4::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b) { (void)c; (void)o; (void)b; return false; }
bool GMEmpireGuardian4::PlayMonsterSound(OBJECT* o) { (void)o; return false; }
void GMEmpireGuardian4::PlayObjectSound(OBJECT* o) { (void)o; }
void GMEmpireGuardian4::PlayBGM() {}
void GMEmpireGuardian4::Init() {}
void GMEmpireGuardian4::Destroy() {}
void GMEmpireGuardian4::SetWeather(int weather) { (void)weather; }

LuaLoadImage gLuaLoadImage;

LuaLoadImage::LuaLoadImage()
	: m_Lua()
{
}

LuaLoadImage::~LuaLoadImage()
{
}

void LuaLoadImage::SetFunctions(lua_State* lua)
{
	(void)lua;
}

void LuaLoadImage::Init()
{
}

LuaCloth::LuaCloth()
{
}

LuaCloth::~LuaCloth()
{
}

void LuaCloth::RegisterClassCloth(lua_State* lua)
{
	(void)lua;
}

CCustomJewel gCustomJewel;
Monsters gMonsters;
MARK_t GuildMark[MAX_MARKS] = {};
CGuildCache g_GuildCache;
bool EnableGuildWar = false;
int GuildWarIndex = -1;
char GuildWarName[8 + 1] = { 0 };
int GuildWarScore[2] = { 0, 0 };
bool EnableSoccer = false;
BYTE HeroSoccerTeam = 0;
int SoccerTime = 0;
char SoccerTeamName[2][8 + 1] = {};
bool SoccerObserver = false;
CDuelMgr g_DuelMgr;

CGuildCache::CGuildCache()
	: m_dwCurrIndex(0)
{
}

CGuildCache::~CGuildCache()
{
}

void CGuildCache::Reset()
{
	m_dwCurrIndex = 0;
	std::memset(GuildMark, 0, sizeof(GuildMark));
	for (int i = 0; i < MAX_MARKS; ++i)
	{
		GuildMark[i].Key = -1;
	}
}

BOOL CGuildCache::IsExistGuildMark(int nGuildKey)
{
	return GetGuildMarkIndex(nGuildKey) >= 0;
}

int CGuildCache::GetGuildMarkIndex(int nGuildKey)
{
	for (int i = 0; i < MAX_MARKS; ++i)
	{
		if (GuildMark[i].Key == nGuildKey)
		{
			return i;
		}
	}

	return -1;
}

int CGuildCache::MakeGuildMarkIndex(int nGuildKey)
{
	int index = static_cast<int>(m_dwCurrIndex % MAX_MARKS);
	GuildMark[index].Key = nGuildKey;
	GuildMark[index].UnionName[0] = 0;
	GuildMark[index].GuildName[0] = 0;
	std::memset(GuildMark[index].Mark, 0, sizeof(GuildMark[index].Mark));
	++m_dwCurrIndex;
	return index;
}

int CGuildCache::SetGuildMark(int nGuildKey, BYTE* UnionName, BYTE* GuildName, BYTE* Mark)
{
	int index = GetGuildMarkIndex(nGuildKey);

	if (index < 0)
	{
		index = MakeGuildMarkIndex(nGuildKey);
	}

	if (UnionName != NULL)
	{
		std::strncpy(GuildMark[index].UnionName, reinterpret_cast<char*>(UnionName), 8);
		GuildMark[index].UnionName[8] = 0;
	}

	if (GuildName != NULL)
	{
		std::strncpy(GuildMark[index].GuildName, reinterpret_cast<char*>(GuildName), 8);
		GuildMark[index].GuildName[8] = 0;
	}

	if (Mark != NULL)
	{
		for (int i = 0; i < 64; ++i)
		{
			GuildMark[index].Mark[i] = ((i % 2) == 0) ? ((Mark[i / 2] >> 4) & 0x0f) : (Mark[i / 2] & 0x0f);
		}
	}

	return index;
}

CCustomJewel::CCustomJewel()
{
	Init();
}

CCustomJewel::~CCustomJewel()
{
}

void CCustomJewel::Init()
{
	for (int i = 0; i < MAX_CUSTOM_JEWEL; ++i)
	{
		m_CustomJewelInfo[i].Index = -1;
	}
}

void CCustomJewel::Load(CUSTOM_JEWEL_INFO* info)
{
	if (info == NULL)
	{
		return;
	}

	for (int i = 0; i < MAX_CUSTOM_JEWEL; ++i)
	{
		SetInfo(info[i]);
	}
}

void CCustomJewel::SetInfo(CUSTOM_JEWEL_INFO info)
{
	if (info.Index < 0 || info.Index >= MAX_CUSTOM_JEWEL)
	{
		return;
	}

	m_CustomJewelInfo[info.Index] = info;
}

CUSTOM_JEWEL_INFO* CCustomJewel::GetInfo(int index)
{
	if (index < 0 || index >= MAX_CUSTOM_JEWEL)
	{
		return NULL;
	}

	if (m_CustomJewelInfo[index].Index != index)
	{
		return NULL;
	}

	return &m_CustomJewelInfo[index];
}

CUSTOM_JEWEL_INFO* CCustomJewel::GetInfoByItem(int ItemIndex)
{
	for (int i = 0; i < MAX_CUSTOM_JEWEL; ++i)
	{
		CUSTOM_JEWEL_INFO* info = GetInfo(i);
		if (info != NULL && info->ItemIndex == ItemIndex)
		{
			return info;
		}
	}

	return NULL;
}

BOOL CCustomJewel::CheckCustomJewel(int index)
{
	return GetInfo(index) != NULL;
}

BOOL CCustomJewel::CheckCustomJewelByItem(int ItemIndex)
{
	return GetInfoByItem(ItemIndex) != NULL;
}

BOOL CCustomJewel::CheckCustomJewelApplyItem(int ItemIndex, int TargetItemIndex)
{
	(void)TargetItemIndex;
	return CheckCustomJewelByItem(ItemIndex);
}

int CCustomJewel::GetCustomJewelSuccessRate(int ItemIndex, int AccountLevel)
{
	CUSTOM_JEWEL_INFO* info = GetInfoByItem(ItemIndex);
	if (info == NULL)
	{
		return 0;
	}

	if (AccountLevel < 0 || AccountLevel >= 4)
	{
		return 0;
	}

	return info->SuccessRate[AccountLevel];
}

int CCustomJewel::GetCustomJewelSalePrice(int ItemIndex)
{
	CUSTOM_JEWEL_INFO* info = GetInfoByItem(ItemIndex);
	return info != NULL ? info->SalePrice : 0;
}

Monsters::Monsters()
	: m_Lua()
{
	m_CustomMonsterInfo.clear();
}

Monsters::~Monsters()
{
	m_CustomMonsterInfo.clear();
}

void Monsters::Init()
{
	m_CustomMonsterInfo.clear();
}

bool Monsters::CheckCustomMonster(int MonsterID)
{
	return m_CustomMonsterInfo.find(MonsterID) != m_CustomMonsterInfo.end();
}

CHARACTER* Monsters::CreateMonster(int Type, int PositionX, int PositionY, int Key)
{
	(void)Type;
	(void)PositionX;
	(void)PositionY;
	(void)Key;
	return NULL;
}

void Monsters::LoadMonsterModel(int ModelID, char* a2, char* a3, int a4)
{
	(void)ModelID;
	(void)a2;
	(void)a3;
	(void)a4;
}

void Monsters::LoadMonsterTexture(int ModelID, char* a2)
{
	(void)ModelID;
	(void)a2;
}

bool Monsters::MonsterType(int MonsterIndex, int Type)
{
	for (std::map<int, CUSTOM_MONSTER_INFO>::const_iterator it = m_CustomMonsterInfo.begin(); it != m_CustomMonsterInfo.end(); ++it)
	{
		if ((it->second.MonsterID - 574) == MonsterIndex && it->second.Type == Type)
		{
			return true;
		}
	}

	return false;
}

#if !defined(MU_ANDROID_HAS_DSPLAYSOUND_RUNTIME)
HRESULT ReleaseBuffer(int Buffer)
{
	(void)Buffer;
	return 0;
}

void LoadWaveFile(int Buffer, TCHAR* strFileName, int BufferChannel, bool Enable3DSound)
{
	(void)Buffer;
	(void)strFileName;
	(void)BufferChannel;
	(void)Enable3DSound;
}
#endif

void RenderPartObjectBodyColor(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, float Bright, int Texture, int iMonsterIndex)
{
	(void)Type;
	(void)Bright;
	(void)Texture;
	(void)iMonsterIndex;
	b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight,
		o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
}

void RenderPartObjectBodyColor2(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, float Bright, int Texture)
{
	(void)Type;
	(void)Bright;
	(void)Texture;
	b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight,
		o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
}

CDuelMgr::CDuelMgr()
	: m_bIsDuelEnabled(FALSE)
	, m_bIsPetDuelEnabled(FALSE)
	, m_iCurrentChannel(-1)
	, m_bRegenerated(FALSE)
{
	std::memset(m_DuelPlayer, 0, sizeof(m_DuelPlayer));
	std::memset(m_DuelChannels, 0, sizeof(m_DuelChannels));
}

CDuelMgr::~CDuelMgr()
{
}

void CDuelMgr::Reset()
{
	m_bIsDuelEnabled = FALSE;
	m_bIsPetDuelEnabled = FALSE;
	m_iCurrentChannel = -1;
	m_bRegenerated = FALSE;
	std::memset(m_DuelPlayer, 0, sizeof(m_DuelPlayer));
	std::memset(m_DuelChannels, 0, sizeof(m_DuelChannels));
	m_DuelWatchUserList.clear();
}

void CDuelMgr::EnableDuel(BOOL bEnable)
{
	m_bIsDuelEnabled = bEnable;
}

BOOL CDuelMgr::IsDuelEnabled()
{
	return m_bIsDuelEnabled;
}

void CDuelMgr::EnablePetDuel(BOOL bEnable)
{
	m_bIsPetDuelEnabled = bEnable;
}

BOOL CDuelMgr::IsPetDuelEnabled()
{
	return m_bIsPetDuelEnabled;
}

void CDuelMgr::SetDuelPlayer(int iPlayerNum, int iIndex, char* pszID)
{
	if (iPlayerNum < 0 || iPlayerNum >= MAX_DUEL_PLAYERS)
	{
		return;
	}

	m_DuelPlayer[iPlayerNum].m_iIndex = iIndex;
	std::memset(m_DuelPlayer[iPlayerNum].m_szID, 0, sizeof(m_DuelPlayer[iPlayerNum].m_szID));
	if (pszID != NULL)
	{
		std::strncpy(m_DuelPlayer[iPlayerNum].m_szID, pszID, sizeof(m_DuelPlayer[iPlayerNum].m_szID) - 1);
	}
}

void CDuelMgr::SetHeroAsDuelPlayer(int iPlayerNum)
{
	SetDuelPlayer(iPlayerNum, Hero ? Hero->Key : -1, Hero ? Hero->ID : NULL);
}

void CDuelMgr::SetScore(int iPlayerNum, int iScore)
{
	if (iPlayerNum >= 0 && iPlayerNum < MAX_DUEL_PLAYERS)
	{
		m_DuelPlayer[iPlayerNum].m_iScore = iScore;
	}
}

void CDuelMgr::SetHP(int iPlayerNum, int iRate)
{
	if (iPlayerNum >= 0 && iPlayerNum < MAX_DUEL_PLAYERS)
	{
		m_DuelPlayer[iPlayerNum].m_fHPRate = static_cast<float>(iRate);
	}
}

void CDuelMgr::SetSD(int iPlayerNum, int iRate)
{
	if (iPlayerNum >= 0 && iPlayerNum < MAX_DUEL_PLAYERS)
	{
		m_DuelPlayer[iPlayerNum].m_fSDRate = static_cast<float>(iRate);
	}
}

char* CDuelMgr::GetDuelPlayerID(int iPlayerNum)
{
	static char emptyId[1] = { 0 };
	if (iPlayerNum < 0 || iPlayerNum >= MAX_DUEL_PLAYERS)
	{
		return emptyId;
	}

	return m_DuelPlayer[iPlayerNum].m_szID;
}

int CDuelMgr::GetScore(int iPlayerNum)
{
	return (iPlayerNum >= 0 && iPlayerNum < MAX_DUEL_PLAYERS) ? m_DuelPlayer[iPlayerNum].m_iScore : 0;
}

float CDuelMgr::GetHP(int iPlayerNum)
{
	return (iPlayerNum >= 0 && iPlayerNum < MAX_DUEL_PLAYERS) ? m_DuelPlayer[iPlayerNum].m_fHPRate : 0.0f;
}

float CDuelMgr::GetSD(int iPlayerNum)
{
	return (iPlayerNum >= 0 && iPlayerNum < MAX_DUEL_PLAYERS) ? m_DuelPlayer[iPlayerNum].m_fSDRate : 0.0f;
}

BOOL CDuelMgr::IsDuelPlayer(CHARACTER* pCharacter, int iPlayerNum, BOOL bIncludeSummon)
{
	(void)bIncludeSummon;
	if (pCharacter == NULL)
	{
		return FALSE;
	}

	return IsDuelPlayer(static_cast<WORD>(pCharacter->Key), iPlayerNum);
}

BOOL CDuelMgr::IsDuelPlayer(WORD wIndex, int iPlayerNum)
{
	if (iPlayerNum < 0 || iPlayerNum >= MAX_DUEL_PLAYERS)
	{
		return FALSE;
	}

	return m_DuelPlayer[iPlayerNum].m_iIndex == static_cast<int>(wIndex);
}

void CDuelMgr::SendDuelRequestAnswer(int iPlayerNum, BOOL bOK)
{
	(void)iPlayerNum;
	(void)bOK;
}

void CDuelMgr::SetDuelChannel(int iChannelIndex, BOOL bEnable, BOOL bJoinable, char* pszID1, char* pszID2)
{
	if (iChannelIndex < 0 || iChannelIndex >= MAX_DUEL_CHANNELS)
	{
		return;
	}

	m_DuelChannels[iChannelIndex].m_bEnable = bEnable;
	m_DuelChannels[iChannelIndex].m_bJoinable = bJoinable;
	std::memset(m_DuelChannels[iChannelIndex].m_szID1, 0, sizeof(m_DuelChannels[iChannelIndex].m_szID1));
	std::memset(m_DuelChannels[iChannelIndex].m_szID2, 0, sizeof(m_DuelChannels[iChannelIndex].m_szID2));
	if (pszID1 != NULL)
	{
		std::strncpy(m_DuelChannels[iChannelIndex].m_szID1, pszID1, sizeof(m_DuelChannels[iChannelIndex].m_szID1) - 1);
	}
	if (pszID2 != NULL)
	{
		std::strncpy(m_DuelChannels[iChannelIndex].m_szID2, pszID2, sizeof(m_DuelChannels[iChannelIndex].m_szID2) - 1);
	}
}

void CDuelMgr::RemoveAllDuelWatchUser()
{
	m_DuelWatchUserList.clear();
}

void CDuelMgr::AddDuelWatchUser(char* pszUserID)
{
	if (pszUserID != NULL)
	{
		m_DuelWatchUserList.push_back(pszUserID);
	}
}

void CDuelMgr::RemoveDuelWatchUser(char* pszUserID)
{
	if (pszUserID == NULL)
	{
		return;
	}

	for (std::list<std::string>::iterator it = m_DuelWatchUserList.begin(); it != m_DuelWatchUserList.end(); ++it)
	{
		if (*it == pszUserID)
		{
			m_DuelWatchUserList.erase(it);
			break;
		}
	}
}

char* CDuelMgr::GetDuelWatchUser(int iIndex)
{
	static char emptyUser[1] = { 0 };
	if (iIndex < 0)
	{
		return emptyUser;
	}

	int index = 0;
	for (std::list<std::string>::iterator it = m_DuelWatchUserList.begin(); it != m_DuelWatchUserList.end(); ++it, ++index)
	{
		if (index == iIndex)
		{
			return const_cast<char*>(it->c_str());
		}
	}

	return emptyUser;
}

void RequestTerrainLight(float xf, float yf, vec3_t Light)
{
	(void)xf;
	(void)yf;
	if (Light != NULL)
	{
		Vector(1.0f, 1.0f, 1.0f, Light);
	}
}

CSParts::CSParts(int Type, int BoneNumber, bool bBillBoard, float x, float y, float z, float ax, float ay, float az)
{
	(void)Type;
	m_iBoneNumber = BoneNumber;
	m_pObj.bBillBoard = bBillBoard;
	m_vOffset[0] = x;
	m_vOffset[1] = y;
	m_vOffset[2] = z;
	m_pObj.Angle[0] = ax;
	m_pObj.Angle[1] = ay;
	m_pObj.Angle[2] = az;
}

void CSParts::IRender(CHARACTER* c)
{
	(void)c;
}

CSAnimationParts::CSAnimationParts(int Type, int BoneNumber, bool bBillBoard, float x, float y, float z, float ax, float ay, float az)
{
	(void)Type;
	m_iBoneNumber = BoneNumber;
	m_pObj.bBillBoard = bBillBoard;
	m_vOffset[0] = x;
	m_vOffset[1] = y;
	m_vOffset[2] = z;
	m_pObj.Angle[0] = ax;
	m_pObj.Angle[1] = ay;
	m_pObj.Angle[2] = az;
}

void CSAnimationParts::Animation(CHARACTER* c)
{
	(void)c;
}

void CSAnimationParts::IRender(CHARACTER* c)
{
	(void)c;
}

CSParts2D::CSParts2D(int Type, int SubType, int BoneNumber, float x, float y, float z)
{
	(void)Type;
	(void)SubType;
	m_iBoneNumber = BoneNumber;
	m_vOffset[0] = x;
	m_vOffset[1] = y;
	m_vOffset[2] = z;
}

void CSParts2D::IRender(CHARACTER* c)
{
	(void)c;
}

void CreatePartsFactory(CHARACTER* c)
{
	(void)c;
}

void RenderParts(CHARACTER* c)
{
	(void)c;
}

namespace battleCastle
{
	void RenderAurora(int Type, int RenderType, float x, float y, float width, float height, float* Light)
	{
		(void)Type;
		(void)RenderType;
		(void)x;
		(void)y;
		(void)width;
		(void)height;
		(void)Light;
	}
}

void CreateGuildMark(int nMarkIndex, bool blend)
{
	(void)nMarkIndex;
	(void)blend;
}

void SaveTerrainLight(char* FileName)
{
	(void)FileName;
}

void SaveTerrainHeight(char* name)
{
	(void)name;
}

bool SaveTerrainMapping(char* FileName, int iMapNumber)
{
	(void)FileName;
	(void)iMapNumber;
	return false;
}

bool SaveTerrainAttribute(char* FileName, int iMapNumber)
{
	(void)FileName;
	(void)iMapNumber;
	return false;
}

bool SaveObjects(char* FileName, int iMapNumber)
{
	(void)FileName;
	(void)iMapNumber;
	return false;
}

void InitPath()
{
}

int PartyNumber = 0;
PARTY_t Party[MAX_PARTYS] = {};

bool SearchEffect(int iType, OBJECT* pOwner, int iSubType)
{
	(void)iType;
	(void)pOwner;
	(void)iSubType;
	return false;
}

bool DeleteEffect(int Type, OBJECT* Owner, int iSubType)
{
	(void)Type;
	(void)Owner;
	(void)iSubType;
	return false;
}

CSummonSystem g_SummonSystem;

CSummonSystem::CSummonSystem()
{
	m_EquipEffectRandom.clear();
}

CSummonSystem::~CSummonSystem()
{
}

void CSummonSystem::MoveEquipEffect(CHARACTER* pCharacter, int iItemType, int iItemLevel, int iItemOption1)
{
	(void)pCharacter;
	(void)iItemType;
	(void)iItemLevel;
	(void)iItemOption1;
}

void CSummonSystem::RemoveEquipEffects(CHARACTER* pCharacter)
{
	(void)pCharacter;
}

void CSummonSystem::RemoveEquipEffect_Summon(CHARACTER* pCharacter)
{
	(void)pCharacter;
}

BOOL CSummonSystem::SendRequestSummonSkill(int iSkill, CHARACTER* pCharacter, OBJECT* pObject)
{
	(void)iSkill;
	(void)pCharacter;
	(void)pObject;
	return FALSE;
}

void CSummonSystem::CastSummonSkill(int iSkill, CHARACTER* pCharacter, OBJECT* pObject, int iTargetPos_X, int iTargetPos_Y)
{
	(void)iSkill;
	(void)pCharacter;
	(void)pObject;
	(void)iTargetPos_X;
	(void)iTargetPos_Y;
}

void CSummonSystem::CreateDamageOfTimeEffect(int iSkill, OBJECT* pObject)
{
	(void)iSkill;
	(void)pObject;
}

void CSummonSystem::RemoveDamageOfTimeEffect(int iSkill, OBJECT* pObject)
{
	(void)iSkill;
	(void)pObject;
}

void CSummonSystem::RemoveAllDamageOfTimeEffect(OBJECT* pObject)
{
	(void)pObject;
}

void CSummonSystem::CreateEquipEffect_WristRing(CHARACTER* pCharacter, int iItemType, int iItemLevel, int iItemOption1)
{
	(void)pCharacter;
	(void)iItemType;
	(void)iItemLevel;
	(void)iItemOption1;
}

void CSummonSystem::RemoveEquipEffect_WristRing(CHARACTER* pCharacter)
{
	(void)pCharacter;
}

void CSummonSystem::CreateEquipEffect_Summon(CHARACTER* pCharacter, int iItemType, int iItemLevel, int iItemOption1)
{
	(void)pCharacter;
	(void)iItemType;
	(void)iItemLevel;
	(void)iItemOption1;
}

void CSummonSystem::CreateCastingEffect(vec3_t vPosition, vec3_t vAngle, int iSubType)
{
	(void)vPosition;
	(void)vAngle;
	(void)iSubType;
}

void CSummonSystem::CreateSummonObject(int iSkill, CHARACTER* pCharacter, OBJECT* pObject, float fTargetPos_X, float fTargetPos_Y)
{
	(void)iSkill;
	(void)pCharacter;
	(void)pObject;
	(void)fTargetPos_X;
	(void)fTargetPos_Y;
}

void CSummonSystem::SetPlayerSummon(CHARACTER* pCharacter, OBJECT* pObject)
{
	(void)pCharacter;
	(void)pObject;
}

char MacroText[10][256] = {};

namespace
{
	bool g_android_battle_castle_started = false;
	alignas(PetProcess) unsigned char g_android_pet_process_storage[sizeof(PetProcess)] = {};
	static CSItemOption g_android_cs_item_option_singleton;
	alignas(SEASON3B::CNewUIMasterLevel) unsigned char g_android_master_level_storage[sizeof(SEASON3B::CNewUIMasterLevel)] = {};
}

int SelectedNpc = -1;
PetProcessPtr g_petProcess;

namespace battleCastle
{
	bool IsBattleCastleStart(void)
	{
		return g_android_battle_castle_started;
	}

	void SetBattleCastleStart(bool bResult)
	{
		g_android_battle_castle_started = bResult;
	}

	void CreateBattleCastleCharacter_Visual(CHARACTER* c, OBJECT* o)
	{
		(void)c;
		(void)o;
	}

	void InitEtcSetting(void)
	{
	}

	bool SettingBattleCastleMonsterLinkBone(CHARACTER* c, int Type)
	{
		(void)c;
		(void)Type;
		return false;
	}

	bool RenderBattleCastleMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
	{
		(void)c;
		(void)o;
		(void)b;
		return false;
	}
}

bool SettingHellasMonsterLinkBone(CHARACTER* c, int Type)
{
	(void)c;
	(void)Type;
	return false;
}

bool IsGMCharacter()
{
	return false;
}

void DeleteBug(OBJECT* Owner)
{
	(void)Owner;
}

void ChangeChaosCastleUnit(CHARACTER* c)
{
	(void)c;
}

PetProcess& ThePetProcess()
{
	return *reinterpret_cast<PetProcess*>(g_android_pet_process_storage);
}

PetProcessPtr PetProcess::Make()
{
	return PetProcessPtr();
}

PetProcess::PetProcess()
{
}

PetProcess::~PetProcess()
{
}

bool PetProcess::LoadData()
{
	return false;
}

bool PetProcess::IsPet(int itemType)
{
	(void)itemType;
	return false;
}

bool PetProcess::CreatePet(int itemType, int modelType, vec3_t Position, CHARACTER* Owner, int SubType, int LinkBone)
{
	(void)itemType;
	(void)modelType;
	(void)Position;
	(void)Owner;
	(void)SubType;
	(void)LinkBone;
	return false;
}

void PetProcess::DeletePet(CHARACTER* Owner, int itemType, bool allDelete)
{
	(void)Owner;
	(void)itemType;
	(void)allDelete;
}

void PetProcess::UpdatePets()
{
}

void PetProcess::RenderPets()
{
}

void PetProcess::SetCommandPet(CHARACTER* Owner, int targetKey, PetObject::ActionType cmdType)
{
	(void)Owner;
	(void)targetKey;
	(void)cmdType;
}

SEASON4A::CSocketItemMgr::CSocketItemMgr()
	: m_iNumEquitSetBonusOptions(0)
{
	memset(&m_StatusBonus, 0, sizeof(m_StatusBonus));
}

SEASON4A::CSocketItemMgr::~CSocketItemMgr()
{
}

BOOL SEASON4A::CSocketItemMgr::IsSocketItem(const ITEM* pItem)
{
	(void)pItem;
	return FALSE;
}

BOOL SEASON4A::CSocketItemMgr::IsSocketItem(const OBJECT* pObject)
{
	(void)pObject;
	return FALSE;
}

int SEASON4A::CSocketItemMgr::GetSeedShpereSeedID(const ITEM* pItem)
{
	(void)pItem;
	return -1;
}

int SEASON4A::CSocketItemMgr::GetSocketCategory(int iSeedID)
{
	(void)iSeedID;
	return 0;
}

int SEASON4A::CSocketItemMgr::AttachToolTipForSocketItem(const ITEM* pItem, int iTextNum)
{
	(void)pItem;
	return iTextNum;
}

int SEASON4A::CSocketItemMgr::AttachToolTipForSeedSphereItem(const ITEM* pItem, int iTextNum)
{
	(void)pItem;
	return iTextNum;
}

void SEASON4A::CSocketItemMgr::CheckSocketSetOption()
{
}

BOOL SEASON4A::CSocketItemMgr::IsSocketSetOptionEnabled()
{
	return FALSE;
}

void SEASON4A::CSocketItemMgr::RenderToolTipForSocketSetOption(int iPos_x, int iPos_y)
{
	(void)iPos_x;
	(void)iPos_y;
}

void SEASON4A::CSocketItemMgr::CreateSocketOptionText(char* pszOptionText, int iSeedID, int iSphereLv)
{
	(void)iSeedID;
	(void)iSphereLv;
	if (pszOptionText != NULL)
	{
		pszOptionText[0] = '\0';
	}
}

int SEASON4A::CSocketItemMgr::CalcSocketBonusItemValue(const ITEM* pItem, int iOrgGold)
{
	(void)pItem;
	return iOrgGold;
}

int SEASON4A::CSocketItemMgr::GetSocketOptionValue(const ITEM* pItem, int iSocketIndex)
{
	(void)pItem;
	(void)iSocketIndex;
	return 0;
}

void SEASON4A::CSocketItemMgr::CalcSocketStatusBonus()
{
	memset(&m_StatusBonus, 0, sizeof(m_StatusBonus));
}

void SEASON4A::CSocketItemMgr::OpenSocketItemScript(const unicode::t_char* szFileName)
{
	(void)szFileName;
}

BOOL SEASON4A::CSocketItemMgr::IsSocketItem(int iItemType)
{
	(void)iItemType;
	return FALSE;
}

void SEASON4A::CSocketItemMgr::CalcSocketOptionValueText(char* pszOptionValueText, int iOptionType, float fOptionValue)
{
	(void)iOptionType;
	(void)fOptionValue;
	if (pszOptionValueText != NULL)
	{
		pszOptionValueText[0] = '\0';
	}
}

int SEASON4A::CSocketItemMgr::CalcSocketOptionValue(int iOptionType, float fOptionValue)
{
	(void)iOptionType;
	(void)fOptionValue;
	return 0;
}

SEASON4A::CSocketItemMgr g_SocketItemMgr;

bool CSItemOption::OpenItemSetScript(bool bTestServer)
{
	(void)bTestServer;
	return true;
}

CQuestMng::CQuestMng()
	: m_nNPCIndex(-1)
{
	m_szNPCName[0] = '\0';
}

CQuestMng::~CQuestMng()
{
}

void CQuestMng::LoadQuestScript()
{
}

CQuestMng g_QuestMng;

void OpenGateScript(char* FileName)
{
	(void)FileName;
}

void OpenFilterFile(char* FileName)
{
	(void)FileName;
}

void OpenNameFilterFile(char* FileName)
{
	(void)FileName;
}

void OpenMonsterSkillScript(char* FileName)
{
	(void)FileName;
}

void CHARACTER_MACHINE::Init()
{
	memset(this, 0, sizeof(*this));
	for (int i = 0; i < MAX_NEW_EQUIPMENT_INDEX; ++i)
	{
		Equipment[i].Type = -1;
	}
}

void CHARACTER_MACHINE::InitAddValue()
{
}

void CHARACTER_MACHINE::SetCharacter(BYTE Class)
{
	Character.Class = Class;
}

void CHARACTER_MACHINE::InputEnemyAttribute(MONSTER* Enemy)
{
	if (Enemy != NULL)
	{
		memcpy(&this->Enemy, Enemy, sizeof(MONSTER));
	}
}

bool CHARACTER_MACHINE::IsZeroDurability()
{
	return false;
}

void CHARACTER_MACHINE::CalculateCriticalDamage() {}
void CHARACTER_MACHINE::CalculateAttackRating() {}
void CHARACTER_MACHINE::CalculateSuccessfulBlocking() {}
void CHARACTER_MACHINE::CalculateAttackRatingPK() {}
void CHARACTER_MACHINE::CalculateSuccessfulBlockingPK() {}
void CHARACTER_MACHINE::CalculateDefense() {}
void CHARACTER_MACHINE::CalculateMagicDefense() {}
void CHARACTER_MACHINE::CalculateWalkSpeed() {}
void CHARACTER_MACHINE::CalculateNextExperince() {}
void CHARACTER_MACHINE::CalulateMasterLevelNextExperience() {}
void CHARACTER_MACHINE::CalculateAll() {}
void CHARACTER_MACHINE::CalculateBasicState() {}

void CHARACTER_MACHINE::getAllAddStateOnlyExValues(int& iAddStrengthExValues, int& iAddDexterityExValues, int& iAddVitalityExValues, int& iAddEnergyExValues, int& iAddCharismaExValues)
{
	iAddStrengthExValues = 0;
	iAddDexterityExValues = 0;
	iAddVitalityExValues = 0;
	iAddEnergyExValues = 0;
	iAddCharismaExValues = 0;
}

bool CreateBugSub(int Type, vec3_t Position, OBJECT* Owner, OBJECT* o, int SubType, int LinkBone)
{
	(void)Type;
	(void)Position;
	(void)Owner;
	(void)o;
	(void)SubType;
	(void)LinkBone;
	return false;
}

void CreateBug(int Type, vec3_t Position, OBJECT* Owner, int SubType, int LinkBone)
{
	(void)Type;
	(void)Position;
	(void)Owner;
	(void)SubType;
	(void)LinkBone;
}

static SEASON3B::CNewUIMasterLevel* GetDummyMasterLevelWindow()
{
	return reinterpret_cast<SEASON3B::CNewUIMasterLevel*>(g_android_master_level_storage);
}

SEASON3B::CNewUIMasterLevel* SEASON3B::CNewUISystem::GetUI_NewMasterLevelInterface() const
{
	return GetDummyMasterLevelWindow();
}

void SEASON3B::CNewUIMasterLevel::OpenMasterLevel(const char* filename)
{
	(void)filename;
}

void ClearItems()
{
}

CSimpleModulus::CSimpleModulus()
{
	Init();
}

CSimpleModulus::~CSimpleModulus()
{
}

void CSimpleModulus::Init(void)
{
	memset(m_dwModulus, 0, sizeof(m_dwModulus));
	memset(m_dwEncryptionKey, 0, sizeof(m_dwEncryptionKey));
	memset(m_dwDecryptionKey, 0, sizeof(m_dwDecryptionKey));
	memset(m_dwXORKey, 0, sizeof(m_dwXORKey));
}

int CSimpleModulus::Encrypt(void* lpTarget, void* lpSource, int iSize)
{
	if (lpTarget != NULL && lpSource != NULL && iSize > 0)
	{
		memcpy(lpTarget, lpSource, static_cast<size_t>(iSize));
	}
	return iSize;
}

int CSimpleModulus::Decrypt(void* lpTarget, void* lpSource, int iSize)
{
	if (lpTarget != NULL && lpSource != NULL && iSize > 0)
	{
		memcpy(lpTarget, lpSource, static_cast<size_t>(iSize));
	}
	return iSize;
}

CSimpleModulus g_SimpleModulusCS;
BYTE g_byPacketSerialSend = 0;

CmuConsoleDebug::CmuConsoleDebug()
	: m_bInit(false)
{
}

CmuConsoleDebug::~CmuConsoleDebug()
{
}

CmuConsoleDebug* CmuConsoleDebug::GetInstance()
{
	static CmuConsoleDebug s_instance;
	return &s_instance;
}

void CmuConsoleDebug::Write(int iType, const char* pStr, ...)
{
	(void)iType;
	(void)pStr;
}

CWsctlc::CWsctlc(void)
	: m_hWnd(NULL)
	, m_bGame(FALSE)
	, m_iMaxSockets(0)
	, m_socket(INVALID_SOCKET)
	, m_nSendBufLen(0)
	, m_nRecvBufLen(0)
	, m_LogPrint(0)
	, m_logfp(NULL)
	, m_pPacketQueue(NULL)
{
	memset(m_SendBuf, 0, sizeof(m_SendBuf));
	memset(m_RecvBuf, 0, sizeof(m_RecvBuf));
}

CWsctlc::~CWsctlc()
{
}

BOOL CWsctlc::Close()
{
	m_socket = INVALID_SOCKET;
	return TRUE;
}

void CWsctlc::LogHexPrintS(BYTE* buf, int size)
{
	(void)buf;
	(void)size;
}

namespace
{
	CWsctlc g_android_dummy_socket_client;
}

CWsctlc* g_pSocketClient = &g_android_dummy_socket_client;

CHelperManager::CHelperManager()
{
}

CHelperManager::~CHelperManager()
{
}

bool CHelperManager::CreateHelper(int Index, int ModelIndex, vec3_t Position, CHARACTER* m_Owner, int SubType)
{
	(void)Index;
	(void)ModelIndex;
	(void)Position;
	(void)m_Owner;
	(void)SubType;
	return false;
}

void CHelperManager::DeleteHelper(CHARACTER* Owner, int itemType, bool allDelete)
{
	(void)Owner;
	(void)itemType;
	(void)allDelete;
}

CHelperManager gHelperManager;

CElementPet::CElementPet()
{
}

CElementPet::~CElementPet()
{
}

bool CElementPet::CreateHelper(int Index, int ModelIndex, vec3_t Position, CHARACTER* m_Owner, int SubType)
{
	(void)Index;
	(void)ModelIndex;
	(void)Position;
	(void)m_Owner;
	(void)SubType;
	return false;
}

void CElementPet::DeleteHelper(CHARACTER* Owner, int itemType, bool allDelete)
{
	(void)Owner;
	(void)itemType;
	(void)allDelete;
}

CElementPet gElementPetFirst;
CElementPet gElementPetSecond;

CCustomPreview::CCustomPreview()
{
	ClearCustomPreviewCharList();
	ClearCustomPreviewList(0);
	ClearCustomPreviewList(1);
}

CCustomPreview::~CCustomPreview()
{
}

void CCustomPreview::ClearCustomPreviewCharList()
{
	for (int i = 0; i < MAX_CUSTOM_PREVIEW_CHAR; ++i)
	{
		m_CustomPreviewCharList[i].Reset();
	}
}

void CCustomPreview::InsertCustomPreviewCharList(int slot, char* name, int pet, int wing)
{
	if (slot < 0 || slot >= MAX_CUSTOM_PREVIEW_CHAR)
	{
		return;
	}

	m_CustomPreviewCharList[slot].Reset();
	if (name != NULL)
	{
		strncpy(m_CustomPreviewCharList[slot].name, name, sizeof(m_CustomPreviewCharList[slot].name) - 1);
	}
	m_CustomPreviewCharList[slot].PetIndex = pet;
	m_CustomPreviewCharList[slot].WingIndex = wing;
	m_CustomPreviewCharList[slot].slot = static_cast<BYTE>(slot);
}

CUSTOM_PET_VIEW_CHAR_LIST* CCustomPreview::GetCustomPreviewCharList(char* name)
{
	if (name == NULL || name[0] == '\0')
	{
		return NULL;
	}

	for (int i = 0; i < MAX_CUSTOM_PREVIEW_CHAR; ++i)
	{
		if (strncmp(m_CustomPreviewCharList[i].name, name, sizeof(m_CustomPreviewCharList[i].name)) == 0)
		{
			return &m_CustomPreviewCharList[i];
		}
	}

	return NULL;
}

void CCustomPreview::ClearCustomPreviewList(int slot)
{
	(void)slot;
	for (int i = 0; i < MAX_CUSTOM_PREVIEW; ++i)
	{
		m_CustomPreviewList[i].Reset();
	}
}

void CCustomPreview::InsertCustomPreviewList(int slot, char* name, int pet, int wing, WORD index, WORD secondpet, WORD Element[2])
{
	if (slot < 0 || slot >= MAX_CUSTOM_PREVIEW)
	{
		return;
	}

	m_CustomPreviewList[slot].Reset();
	if (name != NULL)
	{
		strncpy(m_CustomPreviewList[slot].name, name, sizeof(m_CustomPreviewList[slot].name) - 1);
	}
	m_CustomPreviewList[slot].PetIndex = pet;
	m_CustomPreviewList[slot].SecondPetIndex = secondpet;
	m_CustomPreviewList[slot].WingIndex = wing;
	m_CustomPreviewList[slot].index = index;
	m_CustomPreviewList[slot].slot = static_cast<BYTE>(slot);
	if (Element != NULL)
	{
		m_CustomPreviewList[slot].Element[0] = Element[0];
		m_CustomPreviewList[slot].Element[1] = Element[1];
	}
}

CUSTOM_PET_VIEW_LIST* CCustomPreview::GetCustomPreviewList(int index)
{
	for (int i = 0; i < MAX_CUSTOM_PREVIEW; ++i)
	{
		if (m_CustomPreviewList[i].index == index)
		{
			return &m_CustomPreviewList[i];
		}
	}

	return NULL;
}

void CCustomPreview::DeleteCustomPreview(int index)
{
	for (int i = 0; i < MAX_CUSTOM_PREVIEW; ++i)
	{
		if (m_CustomPreviewList[i].index == index)
		{
			m_CustomPreviewList[i].Reset();
		}
	}
}

CCustomPreview gCustomPreview;
int HeroIndex = 0;

CMonsterName::CMonsterName()
{
}

CMonsterName::~CMonsterName()
{
}

void CMonsterName::Init()
{
	m_CustomMonsterNameInfo.clear();
}

CUSTOM_MONSTER_NAME_INFO* CMonsterName::getMonster(int Class, int Map, int X, int Y)
{
	(void)Class;
	(void)Map;
	(void)X;
	(void)Y;
	return NULL;
}

void CMonsterName::SetMonsterAttribute(CHARACTER* ObjectMonster, int MonsterID, int PositionX, int PositionY)
{
	(void)ObjectMonster;
	(void)MonsterID;
	(void)PositionX;
	(void)PositionY;
}

CMonsterName gMonsterName;

CHARACTER* CreateHellasMonster(int Type, int PositionX, int PositionY, int Key)
{
	(void)Type;
	(void)PositionX;
	(void)PositionY;
	(void)Key;
	return NULL;
}

namespace battleCastle
{
	CHARACTER* CreateBattleCastleMonster(int Type, int PositionX, int PositionY, int Key)
	{
		(void)Type;
		(void)PositionX;
		(void)PositionY;
		(void)Key;
		return NULL;
	}
}

namespace M31HuntingGround
{
	CHARACTER* CreateHuntingGroundMonster(int iType, int PosX, int PosY, int Key)
	{
		(void)iType;
		(void)PosX;
		(void)PosY;
		(void)Key;
		return NULL;
	}
}

namespace M34CryingWolf2nd
{
	CHARACTER* CreateCryingWolf2ndMonster(int iType, int PosX, int PosY, int Key)
	{
		(void)iType;
		(void)PosX;
		(void)PosY;
		(void)Key;
		return NULL;
	}
}

namespace M34CryWolf1st
{
	CHARACTER* CreateCryWolf1stMonster(int iType, int PosX, int PosY, int Key)
	{
		(void)iType;
		(void)PosX;
		(void)PosY;
		(void)Key;
		return NULL;
	}
}

namespace M33Aida
{
	CHARACTER* CreateAidaMonster(int iType, int PosX, int PosY, int Key)
	{
		(void)iType;
		(void)PosX;
		(void)PosY;
		(void)Key;
		return NULL;
	}
}

namespace M37Kanturu1st
{
	CHARACTER* CreateKanturu1stMonster(int iType, int PosX, int PosY, int Key)
	{
		(void)iType;
		(void)PosX;
		(void)PosY;
		(void)Key;
		return NULL;
	}
}

namespace M38Kanturu2nd
{
	CHARACTER* Create_Kanturu2nd_Monster(int iType, int PosX, int PosY, int Key)
	{
		(void)iType;
		(void)PosX;
		(void)PosY;
		(void)Key;
		return NULL;
	}
}

namespace M39Kanturu3rd
{
	CHARACTER* CreateKanturu3rdMonster(int iType, int PosX, int PosY, int Key)
	{
		(void)iType;
		(void)PosX;
		(void)PosY;
		(void)Key;
		return NULL;
	}
}

OBJECT* CreateObject(int Type, vec3_t Position, vec3_t Angle, float Scale)
{
	(void)Type;
	(void)Position;
	(void)Angle;
	(void)Scale;
	return NULL;
}

void UnRegisterBuff(eBuffState buff, OBJECT* o)
{
	(void)buff;
	(void)o;
}

SEASON3B::CNewUIOptionWindow* SEASON3B::CNewUISystem::GetUI_NewOptionWindow() const
{
	return NULL;
}

static SEASON3B::CNewUIOptionWindow* GetDummyOptionWindow()
{
	alignas(SEASON3B::CNewUIOptionWindow) static unsigned char storage[sizeof(SEASON3B::CNewUIOptionWindow)] = {};
	return reinterpret_cast<SEASON3B::CNewUIOptionWindow*>(storage);
}

static SEASON3B::CNewUIMainFrameWindow* GetDummyMainFrameWindow()
{
	alignas(SEASON3B::CNewUIMainFrameWindow) static unsigned char storage[sizeof(SEASON3B::CNewUIMainFrameWindow)] = {};
	return reinterpret_cast<SEASON3B::CNewUIMainFrameWindow*>(storage);
}

static SEASON3B::CNewUIChatLogWindow* GetDummyChatLogWindow()
{
	alignas(SEASON3B::CNewUIChatLogWindow) static unsigned char storage[sizeof(SEASON3B::CNewUIChatLogWindow)] = {};
	return reinterpret_cast<SEASON3B::CNewUIChatLogWindow*>(storage);
}

SEASON3B::CNewUIMainFrameWindow* SEASON3B::CNewUISystem::GetUI_NewMainFrameWindow() const
{
	return GetDummyMainFrameWindow();
}

SEASON3B::CNewUIChatLogWindow* SEASON3B::CNewUISystem::GetUI_NewChatLogWindow() const
{
	return GetDummyChatLogWindow();
}

int SEASON3B::CNewUIMainFrameWindow::GetSkillHotKey(int iHotKey)
{
	(void)iHotKey;
	return 0;
}

int SEASON3B::CNewUIMainFrameWindow::GetItemHotKey(int iHotKey)
{
	(void)iHotKey;
	return 0;
}

int SEASON3B::CNewUIMainFrameWindow::GetItemHotKeyLevel(int iHotKey)
{
	(void)iHotKey;
	return 0;
}

bool SEASON3B::CNewUIOptionWindow::IsAutoAttack()
{
	return false;
}

bool SEASON3B::CNewUIOptionWindow::IsWhisperSound()
{
	return true;
}

bool SEASON3B::CNewUIOptionWindow::IsSlideHelp()
{
	return true;
}

int SEASON3B::CNewUIOptionWindow::GetRenderLevel()
{
	return 4;
}

SEASON3B::MESSAGE_TYPE SEASON3B::CNewUIChatLogWindow::GetCurrentMsgType() const
{
	return SEASON3B::TYPE_ALL_MESSAGE;
}

size_t SEASON3B::CNewUIChatLogWindow::GetNumberOfLines(SEASON3B::MESSAGE_TYPE MsgType)
{
	(void)MsgType;
	return 6;
}

float SEASON3B::CNewUIChatLogWindow::GetBackAlpha() const
{
	return 0.0f;
}

BOOL g_bUseChatListBox = TRUE;

bool RenderHellasMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
	(void)c;
	(void)o;
	(void)b;
	return false;
}

void OpenDialogFile(char* FileName)
{
	(void)FileName;
}

void OpenItemScript(char* FileName)
{
	(void)FileName;
}

void OpenMonsterScript(char* FileName)
{
	(void)FileName;
}

void OpenSkillScript(char* FileName)
{
	(void)FileName;
}

bool SEASON3B::CMoveCommandData::OpenMoveReqScript(const std::string& filename)
{
	(void)filename;
	return false;
}

bool CSQuest::OpenQuestScript(char* filename)
{
	(void)filename;
	return false;
}

#endif

#include "stdafx.h"
#include "Patente.h"
#include "LuaStack.hpp"
#include "LuaBMD.h"
#include "LuaObject.h"
#include "CSParts.h"

using namespace luaaa;

CPatente gPatente;

CPatente::CPatente() : m_Lua()
{
	this->m_PlayerInfo.clear();
	this->m_PatentModelInfo.clear();
	this->m_PatentImageInfo.clear();
}

CPatente::~CPatente()
{
	this->m_PlayerInfo.clear();
	this->m_PatentModelInfo.clear();
	this->m_PatentImageInfo.clear();
}

bool CPatente::CheckImagesModel(int ModelImage)
{
	for (std::map<int, PATENT_IMAGE_INFO>::iterator it = this->m_PatentImageInfo.begin(); it != this->m_PatentImageInfo.end(); ++it)
	{
		if (it->second.PatentTransparency == 1)
		{
			if (it->second.PatentImageIndex == ModelImage)
			{
				return 1;
			}
		}
	}

	return 0;
}

void CPatente::NewCreatePartsFactory(CHARACTER* character)
{
	if (character->Object.Kind != KIND_PLAYER) 
	{
		if (character->m_pParts != 0) 
		{
			DeleteParts(character);
		}
		goto CALLFUNC;
	}

	auto lpInfo = gPatente.GetPlayer(character->Key);

	if (lpInfo != nullptr) 
	{
		if (character->Object.Teleport == 1 || character->Object.Teleport == 2) 
		{
			if (character->m_pParts != 0) 
			{
				DeleteParts(character);
			}

			return;
		}

		if (character->m_pParts != 0 && character->EtcPart != (lpInfo->Patente + 19))
		{
			DeleteParts(character);
		}

		if (character->m_pParts != 0 && character->EtcPart == (lpInfo->Patente + 19))
		{
			return;
		}

		gPatente.m_Lua.Generic_Call("SetPlayerPatente", "iii>", FindCharacterIndex(character->Key), lpInfo->Type, lpInfo->Patente);

		if (character->m_pParts != 0)
		{
			return;
		}
	}

	CALLFUNC:

	CreatePartsFactory(character);
}

void CreatePartsModel(int arrayPlayer, int PatentIndex, int32_t Type, int32_t BoneNumber, int bBillBoard, float x, float y, float z, float ax, float ay, float az) 
{
	auto PreviewStruct = &CharactersClient[arrayPlayer];

	PreviewStruct->EtcPart = (PatentIndex + 19);

	PreviewStruct->m_pParts = (CSIPartsMDL*)new CSParts(Type, BoneNumber, bBillBoard, x, y, z, ax, ay, az);
}

void CreatePartsImage(int arrayPlayer, int PatentIndex, int32_t IndexImage, int BoneNumber, float x, float y, float z)
{
	auto PreviewStruct = &CharactersClient[arrayPlayer];

	PreviewStruct->EtcPart = (PatentIndex + 19);

	PreviewStruct->m_pParts = (CSIPartsMDL*)new CSParts2D(IndexImage, 0, BoneNumber, x, y, z);
}

void SetPatentModel(int PatentIndex, char* Folder, char* ModelName) 
{
	PATENT_MODEL_INFO info;

	info.PatentIndex = PatentIndex;

	strcpy_s(info.ModelFolder, Folder);

	strcpy_s(info.ModelName, ModelName);

	gPatente.m_PatentModelInfo.insert(std::make_pair(info.PatentIndex, info));
}

void SetPatentImage(int PatentIndex, int Transparency, int ImageIndex)
{
	PATENT_IMAGE_INFO info;

	info.PatentIndex = PatentIndex;

	info.PatentTransparency = Transparency;

	info.PatentImageIndex = ImageIndex;

	gPatente.m_PatentImageInfo.insert(std::make_pair(info.PatentIndex, info));
}

PATENT_IMAGE_INFO* CPatente::GetPatentImageInfoByImageIndex(int index)
{
	for (auto& Patente : this->m_PatentImageInfo)
	{
		if (Patente.second.PatentImageIndex == index)
		{
			return &Patente.second;
		}
	}

	return 0;
}
void CPatente::Init()
{
	this->m_LuaLoadImage.SetFunctions(this->m_Lua.GetState());

	this->m_LuaInterface.RegisterInterface(this->m_Lua.GetState());

	this->m_LuaCharacter.RegisterLuaCharacter(this->m_Lua.GetState());

	this->m_LuaBMD.RegisterClassBMD(this->m_Lua.GetState());

	RegisterClassObject(this->m_Lua.GetState());

	LuaModule(this->m_Lua.GetState()).fun("SetPatentModel", &SetPatentModel);

	LuaModule(this->m_Lua.GetState()).fun("SetPatentImage", &SetPatentImage);

	this->m_Lua.DoFile("Data//Configs//Lua//CharacterSystem//PatentSystem.lua");

	this->m_Lua.Generic_Call("StartPatentLoadModel", "");

	LuaModule(this->m_Lua.GetState()).fun("CreatePartsModel", &CreatePartsModel);

	LuaModule(this->m_Lua.GetState()).fun("CreatePartsImage", &CreatePartsImage);
}

void CPatente::Clear()
{
	this->m_PlayerInfo.clear();
}

void CPatente::StartLoadImage()
{
	this->m_Lua.Generic_Call("StartPatentLoadImage", ">");
}

void CPatente::GCPatentePlayerRecv(PMSG_VIEWPORT_RECV * lpMsg)
{
	if (lpMsg->count == 1)
	{
		auto lpInfo = (PMSG_VIEWPORT_PATENT*)(((BYTE*)lpMsg) + sizeof(PMSG_VIEWPORT_RECV) + (sizeof(PMSG_VIEWPORT_PATENT) * 0));

		int aIndex = MAKE_NUMBERW(lpInfo->index[0], lpInfo->index[1]);

		if (this->IsPlayer(aIndex, lpInfo->patent, lpInfo->type) != 0) {
			return;
		}

		PATENTE_PLAYER_INFO info;

		info.aIndex = aIndex;

		info.Patente = lpInfo->patent;

		info.Type = lpInfo->type;

		this->m_PlayerInfo.push_back(info);
	}
	else
	{
		for (int n = 0; n < lpMsg->count; n++)
		{
			auto lpInfo = (PMSG_VIEWPORT_PATENT*)(((BYTE*)lpMsg) + sizeof(PMSG_VIEWPORT_RECV) + (sizeof(PMSG_VIEWPORT_PATENT)*n));

			int aIndex = MAKE_NUMBERW(lpInfo->index[0], lpInfo->index[1]);

			if (this->IsPlayer(aIndex, lpInfo->patent, lpInfo->type) != 0) {
				return;
			}

			PATENTE_PLAYER_INFO info;

			info.aIndex = aIndex;

			info.Patente = lpInfo->patent;

			info.Type = lpInfo->type;

			this->m_PlayerInfo.push_back(info);
		}
	}
}

void CPatente::PatentModelSetIndex(int PatentIndex, int ModelIndex)
{
	this->m_Lua.Generic_Call("PatentModelSetIndex", "ii>", PatentIndex, ModelIndex);
}

PATENTE_PLAYER_INFO* CPatente::GetPlayer(int aIndex)
{
	for (auto &Player : this->m_PlayerInfo) 
	{
		if (Player.aIndex == aIndex) 
		{
			return &Player;
		}
	}

	return 0;
}

bool CPatente::IsPlayer(int aindex, int patent, BYTE type)
{
	for (auto &Player : this->m_PlayerInfo) 
	{
		if (Player.aIndex == aindex) 
		{
			Player.Patente = patent;
			Player.Type = type;
			return 1;
		}
	}

	return 0;
}

void CPatente::DeletePlayer(int aIndex)
{
	std::vector<PATENTE_PLAYER_INFO>::iterator it = this->m_PlayerInfo.begin();

	for (; it != this->m_PlayerInfo.end(); it++) 
	{
		if (it->aIndex == aIndex) 
		{
			this->m_PlayerInfo.erase(it);
			return;
		}
	}
}

int CPatente::PatentModelRender(DWORD BMDStruct, DWORD ObjectStruct, DWORD ItemIndex)
{
	int result = 0;

	if (BMDStruct == 0 || ObjectStruct == 0) 
	{
		return 0;
	}

	this->m_Lua.Generic_Call("PatentModelRender", "jji>i", BMDStruct, ObjectStruct, ItemIndex, &result);

	return result;

}

#pragma once

#include "Protocol.h"
#if !defined(__ANDROID__)
#include "Lua.h"
#include "LuaLoadImage.h"
#include "LuaInterface.h"
#include "LuaCharacter.h"
#include "LuaBMD.h"
#endif

struct PMSG_VIEWPORT_RECV
{
	PSWMSG_HEAD header; // C2:[12:13:1F:20:45:65:68]
	BYTE count;
};

struct PMSG_VIEWPORT_PATENT
{
	BYTE index[2];
	DWORD patent;
	BYTE type;
};

class PATENT_MODEL_INFO 
{
public:
	PATENT_MODEL_INFO() 
	{
		this->PatentIndex = 0;
		memset(this->ModelFolder, 0, sizeof(this->ModelFolder));
		memset(this->ModelName, 0, sizeof(this->ModelName));
	};

	~PATENT_MODEL_INFO() 
	{

	};

	int PatentIndex;
	char ModelFolder[50];
	char ModelName[50];
};

class PATENT_IMAGE_INFO
{
public:
	PATENT_IMAGE_INFO()
	{
		this->PatentIndex = 0;
		this->PatentTransparency = 0;
		this->PatentImageIndex = -1;
	};

	~PATENT_IMAGE_INFO()
	{

	};

	int PatentIndex;
	int PatentTransparency;
	int PatentImageIndex;
};

class PATENTE_PLAYER_INFO 
{
public:
	PATENTE_PLAYER_INFO() 
	{
		this->aIndex = -1;
		this->Patente = 0;
		this->Type = 0;
	};
	~PATENTE_PLAYER_INFO() 
	{

	};

	WORD aIndex;
	DWORD Patente;
	BYTE Type;
};

class CPatente {
public:
	CPatente();
	~CPatente();

	void Init();
	void Clear();
	void StartLoadImage();
	void GCPatentePlayerRecv(PMSG_VIEWPORT_RECV * lpMsg);
	void PatentModelSetIndex(int PatentIndex, int ModelIndex);
	PATENTE_PLAYER_INFO* GetPlayer(int aIndex);
	bool IsPlayer(int aindex, int patent, BYTE type);
	void DeletePlayer(int aIndex);
	int PatentModelRender(DWORD BMDStruct, DWORD ObjectStruct, DWORD ItemIndex);
	void NewCreatePartsFactory(CHARACTER* character);
	bool CheckImagesModel(int ModelImage);
	PATENT_IMAGE_INFO* GetPatentImageInfoByImageIndex(int index);

public:
	std::map<int, PATENT_MODEL_INFO> m_PatentModelInfo;
	std::map<int, PATENT_IMAGE_INFO> m_PatentImageInfo;

private:
	std::vector<PATENTE_PLAYER_INFO> m_PlayerInfo;

private:
#if !defined(__ANDROID__)
	Lua m_Lua;
	LuaLoadImage m_LuaLoadImage;
	LuaInterface m_LuaInterface;
	CLuaCharacter m_LuaCharacter;
	LuaBMD m_LuaBMD;
#endif
};

extern CPatente gPatente;

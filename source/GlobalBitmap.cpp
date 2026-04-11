// GlobalBitmap.cpp: implementation of the CGlobalBitmap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "GlobalBitmap.h"
#include "Platform/RenderBackend.h"
#include "Platform/TextureUploadCompat.h"
#include "./Utilities/Log/ErrorReport.h"
#include "./Utilities/Log/muConsoleDebug.h"

#if defined(__ANDROID__)
#include <android/log.h>
#include <cerrno>
#include <dirent.h>

namespace
{
	// Case-insensitive file open for Android (ext4 is case-sensitive).
	// Tries exact path first, then scans directory for a case-insensitive match.
	FILE* fopen_ci(const std::string& path, const char* mode)
	{
		FILE* f = fopen(path.c_str(), mode);
		if (f) return f;

		// Split into directory and filename
		size_t slash = path.rfind('/');
		if (slash == std::string::npos) return nullptr;

		std::string dir = path.substr(0, slash);
		std::string name = path.substr(slash + 1);

		DIR* d = opendir(dir.c_str());
		if (!d) return nullptr;

		struct dirent* entry;
		while ((entry = readdir(d)) != nullptr)
		{
			if (strcasecmp(entry->d_name, name.c_str()) == 0)
			{
				std::string matched = dir + "/" + entry->d_name;
				closedir(d);
				return fopen(matched.c_str(), mode);
			}
		}
		closedir(d);
		return nullptr;
	}
}
#endif

CBitmapCache::CBitmapCache() 
{
	memset(m_QuickCache, 0, sizeof(QUICK_CACHE)*NUMBER_OF_QUICK_CACHE);
}
CBitmapCache::~CBitmapCache() { Release(); }

bool CBitmapCache::Create()
{
	Release();

	DWORD dwRange = 0;

	dwRange =  BITMAP_MAPTILE_END - BITMAP_MAPTILE_BEGIN;
	m_QuickCache[QUICK_CACHE_MAPTILE].dwBitmapIndexMin = BITMAP_MAPTILE_BEGIN;
	m_QuickCache[QUICK_CACHE_MAPTILE].dwBitmapIndexMax = BITMAP_MAPTILE_END;
	m_QuickCache[QUICK_CACHE_MAPTILE].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_MAPTILE].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_MAPTILE].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	dwRange =  BITMAP_MAPGRASS_END - BITMAP_MAPGRASS_BEGIN;
	m_QuickCache[QUICK_CACHE_MAPGRASS].dwBitmapIndexMin = BITMAP_MAPGRASS_BEGIN;
	m_QuickCache[QUICK_CACHE_MAPGRASS].dwBitmapIndexMax = BITMAP_MAPGRASS_END;
	m_QuickCache[QUICK_CACHE_MAPGRASS].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_MAPGRASS].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_MAPGRASS].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	dwRange =  BITMAP_WATER_END - BITMAP_WATER_BEGIN;
	m_QuickCache[QUICK_CACHE_WATER].dwBitmapIndexMin = BITMAP_WATER_BEGIN;
	m_QuickCache[QUICK_CACHE_WATER].dwBitmapIndexMax = BITMAP_WATER_END;
	m_QuickCache[QUICK_CACHE_WATER].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_WATER].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_WATER].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	dwRange = BITMAP_CURSOR_END - BITMAP_CURSOR_BEGIN;
	m_QuickCache[QUICK_CACHE_CURSOR].dwBitmapIndexMin = BITMAP_CURSOR_BEGIN;
	m_QuickCache[QUICK_CACHE_CURSOR].dwBitmapIndexMax = BITMAP_CURSOR_END;
	m_QuickCache[QUICK_CACHE_CURSOR].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_CURSOR].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_CURSOR].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	dwRange = BITMAP_FONT_END - BITMAP_FONT_BEGIN;
	m_QuickCache[QUICK_CACHE_FONT].dwBitmapIndexMin = BITMAP_FONT_BEGIN;
	m_QuickCache[QUICK_CACHE_FONT].dwBitmapIndexMax = BITMAP_FONT_END;
	m_QuickCache[QUICK_CACHE_FONT].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_FONT].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_FONT].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	dwRange = BITMAP_INTERFACE_NEW_MAINFRAME_END - BITMAP_INTERFACE_NEW_MAINFRAME_BEGIN;
	m_QuickCache[QUICK_CACHE_MAINFRAME].dwBitmapIndexMin = BITMAP_INTERFACE_NEW_MAINFRAME_BEGIN;
	m_QuickCache[QUICK_CACHE_MAINFRAME].dwBitmapIndexMax = BITMAP_INTERFACE_NEW_MAINFRAME_END;
	m_QuickCache[QUICK_CACHE_MAINFRAME].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_MAINFRAME].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_MAINFRAME].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	dwRange = BITMAP_INTERFACE_NEW_SKILLICON_END - BITMAP_INTERFACE_NEW_SKILLICON_BEGIN;
	m_QuickCache[QUICK_CACHE_SKILLICON].dwBitmapIndexMin = BITMAP_INTERFACE_NEW_SKILLICON_BEGIN;
	m_QuickCache[QUICK_CACHE_SKILLICON].dwBitmapIndexMax = BITMAP_INTERFACE_NEW_SKILLICON_END;
	m_QuickCache[QUICK_CACHE_SKILLICON].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_SKILLICON].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_SKILLICON].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));
	
	dwRange = BITMAP_PLAYER_TEXTURE_END - BITMAP_PLAYER_TEXTURE_BEGIN;
	m_QuickCache[QUICK_CACHE_PLAYER].dwBitmapIndexMin = BITMAP_PLAYER_TEXTURE_BEGIN;
	m_QuickCache[QUICK_CACHE_PLAYER].dwBitmapIndexMax = BITMAP_PLAYER_TEXTURE_END;
	m_QuickCache[QUICK_CACHE_PLAYER].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_PLAYER].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_PLAYER].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	m_pNullBitmap = new BITMAP_t;
	memset(m_pNullBitmap, 0, sizeof(BITMAP_t));

	m_ManageTimer.SetTimer(1500);

	return true;
}
void CBitmapCache::Release()
{
	SAFE_DELETE(m_pNullBitmap);

	RemoveAll();

	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		if(m_QuickCache[i].ppBitmap)
			delete [] m_QuickCache[i].ppBitmap;
	}
	memset(m_QuickCache, 0, sizeof(QUICK_CACHE)*NUMBER_OF_QUICK_CACHE);
}

void CBitmapCache::Add(GLuint uiBitmapIndex, BITMAP_t* pBitmap)
{
	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		if(uiBitmapIndex > m_QuickCache[i].dwBitmapIndexMin && uiBitmapIndex < m_QuickCache[i].dwBitmapIndexMax)
		{
			DWORD dwVI = uiBitmapIndex - m_QuickCache[i].dwBitmapIndexMin;
			if(pBitmap)
				m_QuickCache[i].ppBitmap[dwVI] = pBitmap;
			else
				m_QuickCache[i].ppBitmap[dwVI] = m_pNullBitmap;
			return;
		}
	}
	if(pBitmap)
	{
		if(BITMAP_PLAYER_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_PLAYER_TEXTURE_END >= uiBitmapIndex)
			m_mapCachePlayer.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
		else if(BITMAP_INTERFACE_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_INTERFACE_TEXTURE_END >= uiBitmapIndex)
			m_mapCacheInterface.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
		else if(BITMAP_EFFECT_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_EFFECT_TEXTURE_END >= uiBitmapIndex)
			m_mapCacheEffect.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
		else
			m_mapCacheMain.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
	}
}
void CBitmapCache::Remove(GLuint uiBitmapIndex)
{
	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		if(uiBitmapIndex > m_QuickCache[i].dwBitmapIndexMin && uiBitmapIndex < m_QuickCache[i].dwBitmapIndexMax)
		{
			DWORD dwVI = uiBitmapIndex - m_QuickCache[i].dwBitmapIndexMin;
			m_QuickCache[i].ppBitmap[dwVI] = NULL;
			return;
		}
	}

	if(BITMAP_PLAYER_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_PLAYER_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCachePlayer.find(uiBitmapIndex);
		if(mi != m_mapCachePlayer.end())
			m_mapCachePlayer.erase(mi);
	}
	else if(BITMAP_INTERFACE_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_INTERFACE_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCacheInterface.find(uiBitmapIndex);
		if(mi != m_mapCacheInterface.end())
			m_mapCacheInterface.erase(mi);
	}
	else if(BITMAP_EFFECT_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_EFFECT_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCacheEffect.find(uiBitmapIndex);
		if(mi != m_mapCacheEffect.end())
			m_mapCacheEffect.erase(mi);
	}
	else
	{
		type_cache_map::iterator mi = m_mapCacheMain.find(uiBitmapIndex);
		if(mi != m_mapCacheMain.end())
			m_mapCacheMain.erase(mi);
	}
}
void CBitmapCache::RemoveAll()
{
	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		memset(m_QuickCache[i].ppBitmap, 0, m_QuickCache[i].dwRange*sizeof(BITMAP_t*));
	}
	m_mapCacheMain.clear();
	m_mapCachePlayer.clear();
	m_mapCacheInterface.clear();
	m_mapCacheEffect.clear();
}

size_t CBitmapCache::GetCacheSize()
{ 
	return m_mapCacheMain.size() + m_mapCachePlayer.size() + 
		m_mapCacheInterface.size() + m_mapCacheEffect.size(); 
}

void CBitmapCache::Update()
{
	m_ManageTimer.UpdateTime();

	if(m_ManageTimer.IsTime())
	{
		type_cache_map::iterator mi = m_mapCacheMain.begin();
		for(; mi != m_mapCacheMain.end(); )
		{
			BITMAP_t* pBitmap = (*mi).second;
			if(pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				mi++;
			}
			else
			{
				mi = m_mapCacheMain.erase(mi);
			}
		}

		mi = m_mapCachePlayer.begin();
		for(; mi != m_mapCachePlayer.end(); )
		{
			BITMAP_t* pBitmap = (*mi).second;

			if(pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				mi++;
			}
			else
			{
				mi = m_mapCachePlayer.erase(mi);
			}
		}

		mi = m_mapCacheInterface.begin();
		for(; mi != m_mapCacheInterface.end(); )
		{
			BITMAP_t* pBitmap = (*mi).second;
			if(pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				mi++;
			}
			else
			{
				mi = m_mapCacheInterface.erase(mi);
			}
		}

		mi = m_mapCacheEffect.begin();
		for(; mi != m_mapCacheEffect.end(); )
		{
			BITMAP_t* pBitmap = (*mi).second;
			if(pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				mi++;
			}
			else
			{
				mi = m_mapCacheEffect.erase(mi);
			}
		}

#ifdef DEBUG_BITMAP_CACHE
		g_ConsoleDebug->Write(MCD_NORMAL, "M,P,I,E : (%d, %d, %d, %d)", m_mapCacheMain.size(), 
			m_mapCachePlayer.size(), m_mapCacheInterface.size(), m_mapCacheEffect.size());
#endif // DEBUG_BITMAP_CACHE
	}
}

bool CBitmapCache::Find(GLuint uiBitmapIndex, BITMAP_t** ppBitmap)
{
	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		if(uiBitmapIndex > m_QuickCache[i].dwBitmapIndexMin && 
			uiBitmapIndex < m_QuickCache[i].dwBitmapIndexMax)
		{
			DWORD dwVI = uiBitmapIndex - m_QuickCache[i].dwBitmapIndexMin;
			if(m_QuickCache[i].ppBitmap[dwVI])
			{
				if(m_QuickCache[i].ppBitmap[dwVI] == m_pNullBitmap)
					*ppBitmap = NULL;
				else
				{
					*ppBitmap = m_QuickCache[i].ppBitmap[dwVI];
				}
				return true;
			}
			return false;
		}
	}

	if(BITMAP_PLAYER_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_PLAYER_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCachePlayer.find(uiBitmapIndex);
		if(mi != m_mapCachePlayer.end())
		{
			*ppBitmap = (*mi).second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	else if(BITMAP_INTERFACE_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_INTERFACE_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCacheInterface.find(uiBitmapIndex);
		if(mi != m_mapCacheInterface.end())
		{
			*ppBitmap = (*mi).second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	else if(BITMAP_EFFECT_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_EFFECT_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCacheEffect.find(uiBitmapIndex);
		if(mi != m_mapCacheEffect.end())
		{
			*ppBitmap = (*mi).second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	else
	{
		type_cache_map::iterator mi = m_mapCacheMain.find(uiBitmapIndex);
		if(mi != m_mapCacheMain.end())
		{
			*ppBitmap = (*mi).second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	return false;
}

CGlobalBitmap::CGlobalBitmap() 
{ 
	Init();
	m_BitmapCache.Create();

#ifdef DEBUG_BITMAP_CACHE
	m_DebugOutputTimer.SetTimer(5000);
#endif // DEBUG_BITMAP_CACHE
}
CGlobalBitmap::~CGlobalBitmap() 
{ 
	UnloadAllImages();
}
void CGlobalBitmap::Init()
{
	m_uiAlternate = 0;
	m_uiTextureIndexStream = BITMAP_NONAMED_TEXTURES_BEGIN;
	m_dwUsedTextureMemory = 0;
}

GLuint CGlobalBitmap::LoadImage(const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	BITMAP_t* pBitmap = FindTexture(filename);
	if(pBitmap)
	{
		if(pBitmap->Ref > 0)
		{
			if(0 == _stricmp(pBitmap->FileName, filename.c_str()))
			{
				pBitmap->Ref++;

				return pBitmap->BitmapIndex;
			}
		}
	}
	else
	{
		GLuint uiNewTextureIndex = GenerateTextureIndex();
		if(true == LoadImage(uiNewTextureIndex, filename, uiFilter, uiWrapMode))
		{
			m_listNonamedIndex.push_back(uiNewTextureIndex);

			return uiNewTextureIndex;
		}
	}
	return BITMAP_UNKNOWN;
}
bool CGlobalBitmap::LoadImage(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	unsigned int UICLAMP	= GL_CLAMP_TO_EDGE;
	unsigned int UIREPEAT	= GL_REPEAT;

	if( uiWrapMode != UICLAMP && uiWrapMode != UIREPEAT )
	{
#ifdef _DEBUG
		static unsigned int	uiCnt2 = 0;
		int			iBuff;	iBuff = 0;
		
		char		szDebugOutput[256];
		
		iBuff = iBuff + sprintf( iBuff + szDebugOutput, "%d. Call No CLAMP & No REPEAT. \n", uiCnt2++);
		OutputDebugString( szDebugOutput );
#endif
	}

	type_bitmap_map::iterator mi = m_mapBitmap.find(uiBitmapIndex);
	if(mi != m_mapBitmap.end())
	{
		BITMAP_t* pBitmap = (*mi).second;
		if(pBitmap->Ref > 0)
		{
			if(0 == _stricmp(pBitmap->FileName, filename.c_str()))
			{
				pBitmap->Ref++;
				return true;
			}
			else
			{
				g_ErrorReport.Write("File not found %s (%d)->%s\r\n",pBitmap->FileName, uiBitmapIndex, filename.c_str());
				UnloadImage(uiBitmapIndex, true);
			}
		}
	}
	
	std::string ext;
	SplitExt(filename, ext, false);

	if(0 == _stricmp(ext.c_str(), "jpg"))
		return OpenJpeg(uiBitmapIndex, filename, uiFilter, uiWrapMode);
	else if(0 == _stricmp(ext.c_str(), "tga"))
		return OpenTga(uiBitmapIndex, filename, uiFilter, uiWrapMode);

#if defined(__ANDROID__)
	__android_log_print(ANDROID_LOG_WARN, "MUAndroidTexture",
		"LoadImage: unrecognized ext '%s' for %s", ext.c_str(), filename.c_str());
#endif
	return false;
}
void CGlobalBitmap::UnloadImage(GLuint uiBitmapIndex, bool bForce)
{
	type_bitmap_map::iterator mi = m_mapBitmap.find(uiBitmapIndex);
	if(mi != m_mapBitmap.end())
	{
		BITMAP_t* pBitmap = (*mi).second;

		if(--pBitmap->Ref == 0 || bForce)
		{
			glDeleteTextures( 1, &(pBitmap->TextureNumber));

			m_dwUsedTextureMemory -= (DWORD)(pBitmap->Width * pBitmap->Height * pBitmap->Components);

			delete [] pBitmap->Buffer;
			delete pBitmap;
			m_mapBitmap.erase(mi);
			
			if(uiBitmapIndex >= BITMAP_NONAMED_TEXTURES_BEGIN && uiBitmapIndex <= BITMAP_NONAMED_TEXTURES_END)
			{
				m_listNonamedIndex.remove(uiBitmapIndex);
			}
			m_BitmapCache.Remove(uiBitmapIndex);
		}
	}
}
void CGlobalBitmap::UnloadAllImages()
{
#ifdef _DEBUG
	if(!m_mapBitmap.empty())
		g_ErrorReport.Write("Unload Images\r\n");
#endif // _DEBUG
	
	type_bitmap_map::iterator mi = m_mapBitmap.begin();
	for(; mi != m_mapBitmap.end(); mi++)
	{
		BITMAP_t* pBitmap = (*mi).second;

#ifdef _DEBUG
		if(pBitmap->Ref > 1)
		{
			g_ErrorReport.Write("Bitmap %s(RefCount= %d)\r\n", pBitmap->FileName, pBitmap->Ref);
		}
#endif // _DEBUG
		delete [] pBitmap->Buffer;
		delete pBitmap;
	}

	m_mapBitmap.clear();
	m_listNonamedIndex.clear();
	m_BitmapCache.RemoveAll();

	Init();
}

BITMAP_t* CGlobalBitmap::GetTexture(GLuint uiBitmapIndex)
{
	BITMAP_t* pBitmap = NULL;
	if(false == m_BitmapCache.Find(uiBitmapIndex, &pBitmap))
	{
		type_bitmap_map::iterator mi = m_mapBitmap.find(uiBitmapIndex);
		if(mi != m_mapBitmap.end())
			pBitmap = (*mi).second;
		m_BitmapCache.Add(uiBitmapIndex, pBitmap);
	}
	if(NULL == pBitmap)
	{
		static BITMAP_t s_Error;
		memset(&s_Error, 0, sizeof(BITMAP_t));
		strcpy(s_Error.FileName, "CGlobalBitmap::GetTexture Error!!!");
		pBitmap = &s_Error;
	}
	return pBitmap;
}
BITMAP_t* CGlobalBitmap::FindTexture(GLuint uiBitmapIndex)
{
	BITMAP_t* pBitmap = NULL;
	if(false == m_BitmapCache.Find(uiBitmapIndex, &pBitmap))
	{
		type_bitmap_map::iterator mi = m_mapBitmap.find(uiBitmapIndex);
		if(mi != m_mapBitmap.end())
			pBitmap = (*mi).second;
		if(pBitmap != NULL)
			m_BitmapCache.Add(uiBitmapIndex, pBitmap);
	}
	return pBitmap;
}

BITMAP_t* CGlobalBitmap::FindTexture(const std::string& filename)
{
	type_bitmap_map::iterator mi = m_mapBitmap.begin();
	for(; mi != m_mapBitmap.end(); mi++)
	{
		BITMAP_t* pBitmap = (*mi).second;
		if(0 == stricmp(filename.c_str(), pBitmap->FileName)) 
			return pBitmap;
	}
	return NULL;
}

BITMAP_t* CGlobalBitmap::FindTextureByName(const std::string& name)
{
	type_bitmap_map::iterator mi = m_mapBitmap.begin();
	for(; mi != m_mapBitmap.end(); mi++)
	{
		BITMAP_t* pBitmap = (*mi).second;
		std::string texname;
		SplitFileName(pBitmap->FileName, texname, true);
		if(0 == stricmp(texname.c_str(), name.c_str())) 
			return pBitmap;
	}
	return NULL;
}

DWORD CGlobalBitmap::GetUsedTextureMemory() const
{
	return m_dwUsedTextureMemory;
}
size_t CGlobalBitmap::GetNumberOfTexture() const
{
	return m_mapBitmap.size();
}

void CGlobalBitmap::Manage()
{
#ifdef DEBUG_BITMAP_CACHE
	m_DebugOutputTimer.UpdateTime();
	if(m_DebugOutputTimer.IsTime())
	{
		g_ConsoleDebug->Write(MCD_NORMAL, "CacheSize=%d(NumberOfTexture=%d)", m_BitmapCache.GetCacheSize(), GetNumberOfTexture());
	}
#endif // DEBUG_BITMAP_CACHE
	m_BitmapCache.Update();
}

GLuint CGlobalBitmap::GenerateTextureIndex()
{
	GLuint uiAvailableTextureIndex = FindAvailableTextureIndex(m_uiTextureIndexStream);
	if(uiAvailableTextureIndex >= BITMAP_NONAMED_TEXTURES_END)
	{
		m_uiAlternate++;
		m_uiTextureIndexStream = BITMAP_NONAMED_TEXTURES_BEGIN;
		uiAvailableTextureIndex = FindAvailableTextureIndex(m_uiTextureIndexStream);
	}
	return m_uiTextureIndexStream = uiAvailableTextureIndex;
}
GLuint CGlobalBitmap::FindAvailableTextureIndex(GLuint uiSeed)
{
	if(m_uiAlternate > 0)
	{
		type_index_list::iterator li = std::find(m_listNonamedIndex.begin(), m_listNonamedIndex.end(), uiSeed+1);
		if(li != m_listNonamedIndex.end())
			return FindAvailableTextureIndex(uiSeed+1);
	}
	return uiSeed+1;
}

bool CGlobalBitmap::OpenJpeg(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	std::string filename_ozj;
	ExchangeExt(filename, "OZJ", filename_ozj);

#if defined(__ANDROID__)
	FILE* infile = fopen_ci(filename_ozj, "rb");
#else
	FILE* infile = fopen(filename_ozj.c_str(), "rb");
#endif
	if(infile == NULL)
	{
		return false;
	}


	fseek(infile, 0, SEEK_END);
	int DataBytes = ftell(infile);
	fseek(infile, 0, SEEK_SET);

	unsigned char* CData = new unsigned char[DataBytes];
	fread(CData, 1, DataBytes, infile);
	fclose(infile);

	const BYTE gFileProtectHeader[8] = { 0x68, 0xA2, 0xD2, 0x20, 0xA4, 0x43, 0x41, 0xDE };
	const BYTE gFileProtectXorTable[16] = { 0x13, 0xA4, 0x85, 0x1F, 0x1E, 0x19, 0x6F, 0xBE, 0x8A, 0x22, 0x29, 0xDE, 0x03, 0x6E, 0x99, 0xB7 };

	bool hasProtectHeader = (DataBytes >= (int)sizeof(gFileProtectHeader) &&
		memcmp(CData, gFileProtectHeader, sizeof(gFileProtectHeader)) == 0);

	if (hasProtectHeader)
	{
		for (int n = 0; n < ((int)(DataBytes - sizeof(gFileProtectHeader))); n++)
		{
			CData[sizeof(gFileProtectHeader) + n] -= gFileProtectXorTable[gProtect->m_MainInfo.m_PrivateCode[n % strlen(gProtect->m_MainInfo.m_PrivateCode)] % sizeof(gFileProtectXorTable)];
			CData[sizeof(gFileProtectHeader) + n] ^= gFileProtectXorTable[n % sizeof(gFileProtectXorTable)];
		}

		memmove(CData, CData + sizeof(gFileProtectHeader), DataBytes - sizeof(gFileProtectHeader));
		DataBytes -= sizeof(gFileProtectHeader);
	}

	// Find JPEG SOI marker (FF D8).
	// OZJ format: 24-byte header (copy of first 24 bytes of JPEG) + real JPEG.
	// Check byte 24 FIRST (standard OZJ), then byte 0 (raw JPEG / no header).
	int jpegOffset = -1;
	if (DataBytes >= 26 && CData[24] == 0xFF && CData[25] == 0xD8)
	{
		jpegOffset = 24;
	}
	else if (DataBytes >= 2 && CData[0] == 0xFF && CData[1] == 0xD8)
	{
		jpegOffset = 0;
	}
	if (jpegOffset < 0)
	{
#if defined(__ANDROID__)
		__android_log_print(ANDROID_LOG_WARN, "MUAndroidTexture",
			"OpenJpeg: no JPEG SOI found in %s (size=%d first4=%02x%02x%02x%02x)",
			filename.c_str(), DataBytes,
			DataBytes > 0 ? CData[0] : 0, DataBytes > 1 ? CData[1] : 0,
			DataBytes > 2 ? CData[2] : 0, DataBytes > 3 ? CData[3] : 0);
#endif
		delete[] CData;
		return false;
	}

	DataBytes -= jpegOffset;

	unsigned char* Data = new unsigned char[DataBytes];
	memcpy(Data, CData + jpegOffset, DataBytes);
	delete[] CData;

	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	memset(&cinfo, 0, sizeof(cinfo));
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer))
	{
#if defined(__ANDROID__)
		__android_log_print(ANDROID_LOG_WARN, "MUAndroidTexture",
			"OpenJpeg: JPEG decode failed for %s (offset=%d dataSize=%d)",
			filename.c_str(), jpegOffset, DataBytes);
#endif
		jpeg_destroy_decompress(&cinfo);
		delete[] Data;
		return false;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, Data, (unsigned long)DataBytes);
	(void) jpeg_read_header(&cinfo, TRUE);
	(void) jpeg_start_decompress(&cinfo);

	if(cinfo.output_width<=MAX_WIDTH && cinfo.output_height<=MAX_HEIGHT)
	{
		int Width, Height;
		for(int i=1;i<=MAX_WIDTH;i<<=1)
		{
			Width = i;
			if(i >= (int)cinfo.output_width) break;
		}
		for(int i=1;i<=MAX_HEIGHT;i<<=1)
		{
			Height = i;
			if(i >= (int)cinfo.output_height) break;
		}

		BITMAP_t* pNewBitmap = new BITMAP_t;
		memset(pNewBitmap, 0, sizeof(BITMAP_t));

		pNewBitmap->BitmapIndex = uiBitmapIndex;

		{ size_t n = filename.copy(pNewBitmap->FileName, MAX_BITMAP_FILE_NAME - 1); pNewBitmap->FileName[n] = '\0'; }

		pNewBitmap->Width      = (float)Width;
		pNewBitmap->RealWidth = (float)cinfo.output_width;
		pNewBitmap->Height     = (float)Height;
		pNewBitmap->RealHeight = (float)cinfo.output_height;
		pNewBitmap->Components = 3;
#ifdef NEW_MUHELPER_ON
		pNewBitmap->output_width = cinfo.output_width;
		pNewBitmap->output_height = cinfo.output_height;
#endif
		pNewBitmap->Ref = 1;
		
		size_t BufferSize = Width*Height*pNewBitmap->Components;
		pNewBitmap->Buffer = new BYTE[BufferSize];
		m_dwUsedTextureMemory += BufferSize;
		
		int offset=0;
		int row_stride = cinfo.output_width * cinfo.output_components;
		JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
		while (cinfo.output_scanline < cinfo.output_height) 
		{
			if(offset+row_stride > (int)BufferSize)
				break;

			(void) jpeg_read_scanlines(&cinfo, buffer, 1);
			memcpy(pNewBitmap->Buffer+(cinfo.output_scanline-1)*Width*3,buffer[0],row_stride);
			offset += row_stride;
		}

		m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, pNewBitmap));
		
		glGenTextures( 1, &(pNewBitmap->TextureNumber));

		glBindTexture(GL_TEXTURE_2D, pNewBitmap->TextureNumber);

        platform::LegacyCompatibleTexImage2D(GL_TEXTURE_2D, 0, pNewBitmap->Components, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pNewBitmap->Buffer);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, uiFilter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, uiFilter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uiWrapMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, uiWrapMode);
	}
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	delete[] Data;
	return true;
}
bool CGlobalBitmap::OpenTga(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	std::string filename_ozt;
	ExchangeExt(filename, "OZT", filename_ozt);

#if defined(__ANDROID__)
	FILE *fp = fopen_ci(filename_ozt, "rb");
#else
    FILE *fp = fopen(filename_ozt.c_str(), "rb");
#endif
    if(fp == NULL)
	{
		return false;
	}

	fseek(fp,0,SEEK_END);
	int Size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	unsigned char *CPakBuffer = new unsigned char [Size];
	fread(CPakBuffer,1,Size,fp);
	fclose(fp);

	const BYTE gFileProtectHeader[8] = { 0x68, 0xA2, 0xD2, 0x20, 0xA4, 0x43, 0x41, 0xDE };
	const BYTE gFileProtectXorTable[16] = { 0x13, 0xA4, 0x85, 0x1F, 0x1E, 0x19, 0x6F, 0xBE, 0x8A, 0x22, 0x29, 0xDE, 0x03, 0x6E, 0x99, 0xB7 };

	if (Size >= sizeof(gFileProtectHeader) && memcmp(CPakBuffer, gFileProtectHeader, sizeof(gFileProtectHeader)) == 0)
	{
		for (int n = 0; n < ((int)(Size - sizeof(gFileProtectHeader))); n++)
		{
			CPakBuffer[sizeof(gFileProtectHeader) + n] -= gFileProtectXorTable[gProtect->m_MainInfo.m_PrivateCode[n % strlen(gProtect->m_MainInfo.m_PrivateCode)] % sizeof(gFileProtectXorTable)];
			CPakBuffer[sizeof(gFileProtectHeader) + n] ^= gFileProtectXorTable[n % sizeof(gFileProtectXorTable)];
		}

		memmove(CPakBuffer, CPakBuffer + sizeof(gFileProtectHeader), Size - sizeof(gFileProtectHeader));
		Size -= sizeof(gFileProtectHeader);
	}
	unsigned char* PakBuffer = new unsigned char[Size];
	memcpy(PakBuffer, CPakBuffer, Size);

	delete[] CPakBuffer;
	
    int index = 12;
	index += 4;
    short nx = *((short *)(PakBuffer+index));index+=2;
    short ny = *((short *)(PakBuffer+index));index+=2;
    char bit = *((char *)(PakBuffer+index));index+=1;
	index += 1;

    if(bit!=32 || nx>MAX_WIDTH || ny>MAX_HEIGHT)
	{
		SAFE_DELETE_ARRAY(PakBuffer);
		return false;
	}

	int Width = 0, Height = 0;
	for(int i=1;i<=MAX_WIDTH;i<<=1)
	{
		Width = i;
		if(i >= nx) break;
	}
	for(int i=1;i<=MAX_HEIGHT;i<<=1)
	{
		Height = i;
		if(i >= ny) break;
	}

	BITMAP_t* pNewBitmap = new BITMAP_t;
	memset(pNewBitmap, 0, sizeof(BITMAP_t));
	
	pNewBitmap->BitmapIndex = uiBitmapIndex;

	{ size_t n = filename.copy(pNewBitmap->FileName, MAX_BITMAP_FILE_NAME - 1); pNewBitmap->FileName[n] = '\0'; }
	
	pNewBitmap->Width      = (float)Width;
	pNewBitmap->RealWidth = (float)nx;
	pNewBitmap->Height     = (float)Height;
	pNewBitmap->RealHeight = (float)ny;
#ifdef NEW_MUHELPER_ON
	pNewBitmap->output_width = nx;
	pNewBitmap->output_height = ny;
#endif
	pNewBitmap->Components = 4;
	pNewBitmap->Ref = 1;
	
	size_t BufferSize = Width*Height*pNewBitmap->Components;
	pNewBitmap->Buffer     = (unsigned char*)new BYTE[BufferSize];

	m_dwUsedTextureMemory += BufferSize;

    for(int y=0;y<ny;y++)
	{
        unsigned char *src = &PakBuffer[index];
		index += nx * 4;
		unsigned char *dst = &pNewBitmap->Buffer[(ny-1-y)*Width*pNewBitmap->Components];

		for(int x=0;x<nx;x++)
        {
			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
			dst[3] = src[3];
			src += 4;
			dst += pNewBitmap->Components;
        }
	}
	SAFE_DELETE_ARRAY(PakBuffer);

	m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, pNewBitmap));
	
	glGenTextures( 1, &(pNewBitmap->TextureNumber));

	glBindTexture(GL_TEXTURE_2D, pNewBitmap->TextureNumber);

    platform::LegacyCompatibleTexImage2D(GL_TEXTURE_2D, 0, pNewBitmap->Components, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pNewBitmap->Buffer);

    platform::GetRenderBackend().SetTextureCombineMode(platform::TextureCombineMode_Modulate);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, uiFilter);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, uiFilter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uiWrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, uiWrapMode);

	return true;
}

void CGlobalBitmap::SplitFileName(IN const std::string& filepath, OUT std::string& filename, bool bIncludeExt) 
{
	char __fname[_MAX_FNAME] = {0, };
	char __ext[_MAX_EXT] = {0, };
	_splitpath(filepath.c_str(), NULL, NULL, __fname, __ext);
	filename = __fname;
	if(bIncludeExt)
		filename += __ext;
}
void CGlobalBitmap::SplitExt(IN const std::string& filepath, OUT std::string& ext, bool bIncludeDot) 
{
	char __ext[_MAX_EXT] = {0, };
	_splitpath(filepath.c_str(), NULL, NULL, NULL, __ext);
	if(bIncludeDot) {
		ext = __ext;
	}
	else {
		if((__ext[0] == '.') && __ext[1])
			ext = __ext+1;
	}
}
void CGlobalBitmap::ExchangeExt(IN const std::string& in_filepath, IN const std::string& ext, OUT std::string& out_filepath) 
{
	char __drive[_MAX_DRIVE] = {0, };
	char __dir[_MAX_DIR] = {0, };
	char __fname[_MAX_FNAME] = {0, };
	_splitpath(in_filepath.c_str(), __drive, __dir, __fname, NULL);
	
	out_filepath = __drive;
	out_filepath += __dir;
	out_filepath += __fname;
	out_filepath += '.';
	out_filepath += ext;
}

bool CGlobalBitmap::Convert_Format(const unicode::t_string& filename)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	
	::_splitpath(filename.c_str(), drive, dir, fname, ext);
	
	std::string strPath = drive; strPath += dir;
	std::string strName = fname;
	
	if(_stricmp(ext, ".jpg") == 0) 
	{
		unicode::t_string strSaveName = strPath + strName + ".OZJ";
		return Save_Image(filename, strSaveName.c_str(), 24);
	}
	else if(_stricmp(ext, ".tga") == 0) 
	{
		unicode::t_string strSaveName = strPath + strName + ".OZT";
		return Save_Image(filename, strSaveName.c_str(), 4);
	}
	else if(_stricmp(ext, ".bmp") == 0) 
	{
		unicode::t_string strSaveName = strPath + strName + ".OZB";
		return Save_Image(filename, strSaveName.c_str(), 4);
	}
	else
	{
	}

	return false;
}

bool CGlobalBitmap::Save_Image(const unicode::t_string& src, const unicode::t_string& dest, int cDumpHeader)
{
	FILE* fp = fopen(src.c_str(), "rb");
	if(fp == NULL) 
	{
		return false;
	}
	
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	char* pTempBuf = new char[size];
	fread(pTempBuf, 1, size, fp);
	fclose(fp);
	
	fp = fopen(dest.c_str(), "wb");
	if(fp == NULL) 
		return false;
	
	fwrite(pTempBuf, 1, cDumpHeader, fp);
	fwrite(pTempBuf, 1, size, fp);
	fclose(fp);
	
	delete [] pTempBuf;
	
	return true;
}

void CGlobalBitmap::my_error_exit(j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
#if defined(__ANDROID__)
	char buffer[JMSG_LENGTH_MAX] = {0};
	(*cinfo->err->format_message)(cinfo, buffer);
	__android_log_print(ANDROID_LOG_ERROR, "MUAndroidTexture",
		"JPEG error: %s", buffer);
#else
	(*cinfo->err->output_message) (cinfo);
#endif
	longjmp(myerr->setjmp_buffer, 1);
}

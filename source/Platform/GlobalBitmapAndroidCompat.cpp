#include "stdafx.h"

#if defined(__ANDROID__) && !defined(MU_ANDROID_HAS_ZZZTEXTURE_RUNTIME)

#include "GlobalBitmap.h"

#include "Platform/GameAssetPath.h"
#include "Platform/GameBootstrapTexture.h"

#include <algorithm>
#include <string>
#include <sys/stat.h>
#include <vector>

#if !defined(MU_ANDROID_HAS_ZZZTEXTURE_RUNTIME)
CGlobalBitmap Bitmaps;
#endif

namespace
{
	bool FileExists(const std::string& path)
	{
		struct stat file_status = {};
		return !path.empty() && stat(path.c_str(), &file_status) == 0;
	}

	std::string ToLowerAscii(const std::string& value)
	{
		std::string lowered = value;
		for (size_t index = 0; index < lowered.size(); ++index)
		{
			if (lowered[index] >= 'A' && lowered[index] <= 'Z')
			{
				lowered[index] = static_cast<char>(lowered[index] - 'A' + 'a');
			}
		}
		return lowered;
	}

	bool IsAbsolutePath(const std::string& path)
	{
		return !path.empty() && (path[0] == '/' || (path.size() > 1 && path[1] == ':'));
	}

	std::string NormalizeSeparators(const std::string& path)
	{
		std::string normalized = path;
		for (size_t index = 0; index < normalized.size(); ++index)
		{
			if (normalized[index] == '\\')
			{
				normalized[index] = '/';
			}
		}
		return normalized;
	}

	std::string JoinPath(const std::string& left, const std::string& right)
	{
		if (left.empty())
		{
			return right;
		}
		if (right.empty())
		{
			return left;
		}
		if (left[left.size() - 1] == '/' || left[left.size() - 1] == '\\')
		{
			return left + right;
		}
		return left + "/" + right;
	}

	std::string ReplaceExtension(const std::string& path, const char* new_extension)
	{
		const size_t dot = path.find_last_of('.');
		if (dot == std::string::npos)
		{
			return path + (new_extension != NULL ? new_extension : "");
		}
		return path.substr(0, dot) + (new_extension != NULL ? new_extension : "");
	}

	void AppendLegacyBitmapCandidates(const std::string& filename, std::vector<std::string>* candidates)
	{
		if (candidates == NULL || filename.empty())
		{
			return;
		}

		const std::string normalized = NormalizeSeparators(filename);
		const std::string lowered = ToLowerAscii(normalized);
		const bool already_data_root = lowered.rfind("data/", 0) == 0;
		const std::string asset_relative = already_data_root ? normalized : JoinPath("Data", normalized);

		std::vector<std::string> raw_candidates;
		raw_candidates.push_back(asset_relative);

		if (lowered.size() >= 4 && lowered.substr(lowered.size() - 4) == ".tga")
		{
			raw_candidates.push_back(ReplaceExtension(asset_relative, ".OZT"));
			raw_candidates.push_back(ReplaceExtension(asset_relative, ".ozt"));
		}
		else if (lowered.size() >= 4 && lowered.substr(lowered.size() - 4) == ".jpg")
		{
			raw_candidates.push_back(ReplaceExtension(asset_relative, ".OZJ"));
			raw_candidates.push_back(ReplaceExtension(asset_relative, ".ozj"));
		}

		for (size_t index = 0; index < raw_candidates.size(); ++index)
		{
			const std::string candidate =
				IsAbsolutePath(raw_candidates[index]) ? raw_candidates[index] : platform::ResolveGameAssetPath(raw_candidates[index].c_str());
			if (candidate.empty())
			{
				continue;
			}

			if (std::find(candidates->begin(), candidates->end(), candidate) == candidates->end())
			{
				candidates->push_back(candidate);
			}
		}
	}
}

CGlobalBitmap::CGlobalBitmap()
{
	Init();
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
	const GLuint new_index = GenerateTextureIndex();
	if (LoadImage(new_index, filename, uiFilter, uiWrapMode))
	{
		return new_index;
	}
	return BITMAP_UNKNOWN;
}

bool CGlobalBitmap::LoadImage(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	(void)uiFilter;
	(void)uiWrapMode;

	UnloadImage(uiBitmapIndex, true);

	std::vector<std::string> candidates;
	AppendLegacyBitmapCandidates(filename, &candidates);

	for (size_t index = 0; index < candidates.size(); ++index)
	{
		if (!FileExists(candidates[index]))
		{
			continue;
		}

		platform::BootstrapTexture texture = {};
		if (!platform::LoadBootstrapTextureFromFile(candidates[index].c_str(), &texture) || !texture.loaded)
		{
			continue;
		}

		BITMAP_t* bitmap = new BITMAP_t;
		memset(bitmap, 0, sizeof(BITMAP_t));
		bitmap->BitmapIndex = uiBitmapIndex;
		strncpy(bitmap->FileName, candidates[index].c_str(), MAX_BITMAP_FILE_NAME - 1);
		bitmap->FileName[MAX_BITMAP_FILE_NAME - 1] = '\0';
		bitmap->Width = static_cast<float>(texture.width);
		bitmap->Height = static_cast<float>(texture.height);
		bitmap->RealWidth = static_cast<float>(texture.width);
		bitmap->RealHeight = static_cast<float>(texture.height);
		bitmap->Components = 4;
		bitmap->TextureNumber = texture.texture_id;
		bitmap->Ref = 1;
		bitmap->Buffer = NULL;

		m_mapBitmap[uiBitmapIndex] = bitmap;
		m_dwUsedTextureMemory += static_cast<DWORD>(texture.width * texture.height * 4);
		return true;
	}

	return false;
}

void CGlobalBitmap::UnloadImage(GLuint uiBitmapIndex, bool bForce)
{
	(void)bForce;

	type_bitmap_map::iterator found = m_mapBitmap.find(uiBitmapIndex);
	if (found == m_mapBitmap.end())
	{
		return;
	}

	BITMAP_t* bitmap = found->second;
	if (bitmap != NULL)
	{
		if (bitmap->TextureNumber != 0)
		{
			glDeleteTextures(1, &bitmap->TextureNumber);
		}
		delete[] bitmap->Buffer;
		delete bitmap;
	}

	m_mapBitmap.erase(found);
}

void CGlobalBitmap::UnloadAllImages()
{
	type_bitmap_map::iterator iter = m_mapBitmap.begin();
	for (; iter != m_mapBitmap.end(); ++iter)
	{
		BITMAP_t* bitmap = iter->second;
		if (bitmap != NULL)
		{
			if (bitmap->TextureNumber != 0)
			{
				glDeleteTextures(1, &bitmap->TextureNumber);
			}
			delete[] bitmap->Buffer;
			delete bitmap;
		}
	}
	m_mapBitmap.clear();
	m_listNonamedIndex.clear();
	Init();
}

BITMAP_t* CGlobalBitmap::GetTexture(GLuint uiBitmapIndex)
{
	BITMAP_t* bitmap = FindTexture(uiBitmapIndex);
	if (bitmap != NULL)
	{
		return bitmap;
	}

	static BITMAP_t missing_bitmap = {};
	memset(&missing_bitmap, 0, sizeof(BITMAP_t));
	strncpy(missing_bitmap.FileName, "Missing Android bitmap", MAX_BITMAP_FILE_NAME - 1);
	missing_bitmap.Width = 256.0f;
	missing_bitmap.Height = 256.0f;
	missing_bitmap.RealWidth = 256.0f;
	missing_bitmap.RealHeight = 256.0f;
	missing_bitmap.Components = 4;
	return &missing_bitmap;
}

BITMAP_t* CGlobalBitmap::FindTexture(GLuint uiBitmapIndex)
{
	type_bitmap_map::iterator found = m_mapBitmap.find(uiBitmapIndex);
	return found != m_mapBitmap.end() ? found->second : NULL;
}

BITMAP_t* CGlobalBitmap::FindTexture(const std::string& filename)
{
	type_bitmap_map::iterator iter = m_mapBitmap.begin();
	for (; iter != m_mapBitmap.end(); ++iter)
	{
		BITMAP_t* bitmap = iter->second;
		if (bitmap != NULL && strcmp(bitmap->FileName, filename.c_str()) == 0)
		{
			return bitmap;
		}
	}
	return NULL;
}

BITMAP_t* CGlobalBitmap::FindTextureByName(const std::string& name)
{
	type_bitmap_map::iterator iter = m_mapBitmap.begin();
	for (; iter != m_mapBitmap.end(); ++iter)
	{
		BITMAP_t* bitmap = iter->second;
		if (bitmap == NULL)
		{
			continue;
		}

		std::string file_name = bitmap->FileName;
		std::string base_name;
		SplitFileName(file_name, base_name, true);
		if (strcasecmp(base_name.c_str(), name.c_str()) == 0)
		{
			return bitmap;
		}
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

bool CGlobalBitmap::Convert_Format(const unicode::t_string& filename)
{
	(void)filename;
	return false;
}

void CGlobalBitmap::Manage()
{
}

GLuint CGlobalBitmap::GenerateTextureIndex()
{
	return FindAvailableTextureIndex(m_uiTextureIndexStream++);
}

GLuint CGlobalBitmap::FindAvailableTextureIndex(GLuint uiSeed)
{
	GLuint candidate = uiSeed;
	while (m_mapBitmap.find(candidate) != m_mapBitmap.end())
	{
		++candidate;
	}
	return candidate;
}

void CGlobalBitmap::SplitFileName(IN const std::string& filepath, OUT std::string& filename, bool bIncludeExt)
{
	const std::string normalized = NormalizeSeparators(filepath);
	const size_t slash = normalized.find_last_of('/');
	const std::string base_name = (slash == std::string::npos) ? normalized : normalized.substr(slash + 1);
	if (bIncludeExt)
	{
		filename = base_name;
		return;
	}

	const size_t dot = base_name.find_last_of('.');
	filename = dot == std::string::npos ? base_name : base_name.substr(0, dot);
}

void CGlobalBitmap::SplitExt(IN const std::string& filepath, OUT std::string& ext, bool bIncludeDot)
{
	const std::string normalized = NormalizeSeparators(filepath);
	const size_t dot = normalized.find_last_of('.');
	if (dot == std::string::npos)
	{
		ext.clear();
		return;
	}
	ext = bIncludeDot ? normalized.substr(dot) : normalized.substr(dot + 1);
}

void CGlobalBitmap::ExchangeExt(IN const std::string& in_filepath, IN const std::string& ext, OUT std::string& out_filepath)
{
	const std::string replacement = ext.empty() || ext[0] == '.' ? ext : "." + ext;
	out_filepath = ReplaceExtension(in_filepath, replacement.c_str());
}

bool CGlobalBitmap::Save_Image(const unicode::t_string& src, const unicode::t_string& dest, int cDumpHeader)
{
	(void)src;
	(void)dest;
	(void)cDumpHeader;
	return false;
}

void CGlobalBitmap::my_error_exit(j_common_ptr cinfo)
{
	(void)cinfo;
}

#endif

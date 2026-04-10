#if defined(_WIN32)
#include "stdafx.h"
#else
#include <dirent.h>
#include <limits.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "Platform/GameAssetPath.h"

#include <string.h>

#include <string>
#include <vector>

namespace platform
{
	namespace
	{
		std::string g_game_asset_root;
		std::string g_game_asset_root_override;
		bool g_game_asset_root_initialized = false;

	#if defined(_WIN32)
		const char kNativePathSeparator = '\\';
	#else
		const char kNativePathSeparator = '/';
	#endif

		void NormalizePathSeparators(std::string* path)
		{
			for (size_t i = 0; i < path->size(); ++i)
			{
				if ((*path)[i] == '\\' || (*path)[i] == '/')
				{
					(*path)[i] = kNativePathSeparator;
				}
			}
		}

		void TrimTrailingSeparators(std::string* path)
		{
			while (!path->empty() && ((*path)[path->size() - 1] == '\\' || (*path)[path->size() - 1] == '/'))
			{
				path->erase(path->size() - 1);
			}
		}

		bool IsAbsolutePath(const std::string& path)
		{
	#if defined(_WIN32)
			if (path.size() >= 2 && path[1] == ':')
			{
				return true;
			}

			if (path.size() >= 2 && ((path[0] == '\\' && path[1] == '\\') || (path[0] == '/' && path[1] == '/')))
			{
				return true;
			}

			return false;
	#else
			return !path.empty() && path[0] == '/';
	#endif
		}

		bool DirectoryExists(const std::string& path)
		{
	#if defined(_WIN32)
			const DWORD attributes = GetFileAttributesA(path.c_str());
			return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	#else
			struct stat status = {};
			return stat(path.c_str(), &status) == 0 && S_ISDIR(status.st_mode);
	#endif
		}

		bool PathExists(const std::string& path)
		{
	#if defined(_WIN32)
			return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
	#else
			struct stat status = {};
			return stat(path.c_str(), &status) == 0;
	#endif
		}

		int CompareNoCase(const char* left, const char* right)
		{
	#if defined(_WIN32)
			return _stricmp(left, right);
	#else
			return strcasecmp(left, right);
	#endif
		}

		int CompareNoCaseN(const char* left, const char* right, size_t count)
		{
	#if defined(_WIN32)
			return _strnicmp(left, right, count);
	#else
			return strncasecmp(left, right, count);
	#endif
		}

		std::string JoinPath(const std::string& left, const char* right)
		{
			std::string result = left;
			TrimTrailingSeparators(&result);

			if (right != NULL && right[0] != '\0')
			{
				if (!result.empty())
				{
					result += kNativePathSeparator;
				}

				result += right;
			}

			NormalizePathSeparators(&result);
			return result;
		}

		std::string GetCurrentDirectoryString()
		{
	#if defined(_WIN32)
			char buffer[_MAX_PATH] = { 0 };
			GetCurrentDirectoryA(_countof(buffer), buffer);
			std::string path = buffer;
	#else
			char buffer[PATH_MAX] = { 0 };
			if (getcwd(buffer, sizeof(buffer)) == NULL)
			{
				return std::string();
			}
			std::string path = buffer;
	#endif
			NormalizePathSeparators(&path);
			TrimTrailingSeparators(&path);
			return path;
		}

		std::string GetExecutableDirectoryString()
		{
	#if defined(_WIN32)
			char buffer[_MAX_PATH] = { 0 };
			GetModuleFileNameA(NULL, buffer, _countof(buffer));

			std::string path = buffer;
			NormalizePathSeparators(&path);

			const size_t slash_index = path.find_last_of(kNativePathSeparator);
			if (slash_index != std::string::npos)
			{
				path.erase(slash_index);
			}

			TrimTrailingSeparators(&path);
			return path;
	#else
			char buffer[PATH_MAX] = { 0 };
			const ssize_t size = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
			if (size <= 0)
			{
				return std::string();
			}

			buffer[size] = '\0';
			std::string path = buffer;
			NormalizePathSeparators(&path);

			const size_t slash_index = path.find_last_of(kNativePathSeparator);
			if (slash_index != std::string::npos)
			{
				path.erase(slash_index);
			}

			TrimTrailingSeparators(&path);
			return path;
	#endif
		}

		bool EndsWithDataDirectory(const std::string& path)
		{
			const size_t slash_index = path.find_last_of(kNativePathSeparator);
			const char* last_segment = slash_index == std::string::npos ? path.c_str() : path.c_str() + slash_index + 1;
			return CompareNoCase(last_segment, "Data") == 0;
		}

		std::string StripLeadingDataDirectory(const std::string& path)
		{
			if (path.size() > 4 && CompareNoCaseN(path.c_str(), "Data", 4) == 0 && (path[4] == '\\' || path[4] == '/'))
			{
				return path.substr(5);
			}

			return path;
		}

		std::string FindGameAssetRoot()
		{
			if (!g_game_asset_root_override.empty())
			{
				std::string override_root = g_game_asset_root_override;
				NormalizePathSeparators(&override_root);
				TrimTrailingSeparators(&override_root);
				return override_root;
			}

			const std::string current_directory = GetCurrentDirectoryString();
			const std::string executable_directory = GetExecutableDirectoryString();

			std::vector<std::string> candidates;

			if (!current_directory.empty())
			{
				if (EndsWithDataDirectory(current_directory))
				{
					candidates.push_back(current_directory);
				}

				candidates.push_back(JoinPath(current_directory, "Data"));
			#if defined(_WIN32)
				candidates.push_back(JoinPath(current_directory, "..\\..\\Client\\Data"));
			#else
				candidates.push_back(JoinPath(current_directory, "../../Client/Data"));
			#endif
			}

			if (!executable_directory.empty())
			{
				if (EndsWithDataDirectory(executable_directory))
				{
					candidates.push_back(executable_directory);
				}

				candidates.push_back(JoinPath(executable_directory, "Data"));
			#if defined(_WIN32)
				candidates.push_back(JoinPath(executable_directory, "..\\..\\Client\\Data"));
			#else
				candidates.push_back(JoinPath(executable_directory, "../../Client/Data"));
			#endif
			}

			for (size_t i = 0; i < candidates.size(); ++i)
			{
				std::string candidate = candidates[i];
				NormalizePathSeparators(&candidate);
				TrimTrailingSeparators(&candidate);

				if (DirectoryExists(candidate))
				{
					return candidate;
				}
			}

			if (!current_directory.empty())
			{
				return JoinPath(current_directory, "Data");
			}

			return std::string("Data");
		}

#if !defined(_WIN32)
		std::string JoinAbsolutePathComponent(const std::string& base_path, const std::string& component)
		{
			if (base_path.empty() || base_path == "/")
			{
				return std::string("/") + component;
			}

			return base_path + "/" + component;
		}

		std::string ResolveExistingPathCaseInsensitive(const std::string& absolute_path)
		{
			if (absolute_path.empty() || absolute_path[0] != '/' || PathExists(absolute_path))
			{
				return absolute_path;
			}

			std::vector<std::string> components;
			size_t component_begin = 1;
			while (component_begin < absolute_path.size())
			{
				const size_t component_end = absolute_path.find('/', component_begin);
				const std::string component = absolute_path.substr(
					component_begin,
					component_end == std::string::npos ? std::string::npos : component_end - component_begin);
				if (!component.empty())
				{
					components.push_back(component);
				}

				if (component_end == std::string::npos)
				{
					break;
				}

				component_begin = component_end + 1;
			}

			std::string current_path = "/";
			for (size_t index = 0; index < components.size(); ++index)
			{
				const std::string direct_candidate = JoinAbsolutePathComponent(current_path, components[index]);
				if (PathExists(direct_candidate))
				{
					current_path = direct_candidate;
					continue;
				}

				DIR* directory = opendir(current_path.c_str());
				if (directory == NULL)
				{
					return absolute_path;
				}

				bool found_match = false;
				struct dirent* entry = NULL;
				while ((entry = readdir(directory)) != NULL)
				{
					if (CompareNoCase(entry->d_name, components[index].c_str()) == 0)
					{
						current_path = JoinAbsolutePathComponent(current_path, entry->d_name);
						found_match = true;
						break;
					}
				}

				closedir(directory);
				if (!found_match)
				{
					return absolute_path;
				}
			}

			return current_path;
		}
#endif
	}

	void SetGameAssetRootOverride(const char* absolute_root_path)
	{
		g_game_asset_root_override = absolute_root_path != NULL ? absolute_root_path : "";
		NormalizePathSeparators(&g_game_asset_root_override);
		TrimTrailingSeparators(&g_game_asset_root_override);
		g_game_asset_root_initialized = false;
	}

	void ClearGameAssetRootOverride()
	{
		g_game_asset_root_override.clear();
		g_game_asset_root_initialized = false;
	}

	void InitializeGameAssetRoot()
	{
		if (g_game_asset_root_initialized)
		{
			return;
		}

		g_game_asset_root = FindGameAssetRoot();
		g_game_asset_root_initialized = true;
	}

	const char* GetGameAssetRoot()
	{
		InitializeGameAssetRoot();
		return g_game_asset_root.c_str();
	}

	std::string ResolveGameAssetPath(const char* relative_or_data_path)
	{
		InitializeGameAssetRoot();

		if (relative_or_data_path == NULL || relative_or_data_path[0] == '\0')
		{
			return g_game_asset_root;
		}

		std::string path = relative_or_data_path;
		NormalizePathSeparators(&path);

		if (IsAbsolutePath(path))
		{
			return path;
		}

		path = StripLeadingDataDirectory(path);
		path = JoinPath(g_game_asset_root, path.c_str());
#if !defined(_WIN32)
		path = ResolveExistingPathCaseInsensitive(path);
#endif
		return path;
	}

	bool CopyResolvedGameAssetPath(char* destination, size_t destination_size, const char* relative_or_data_path)
	{
		if (destination == NULL || destination_size == 0)
		{
			return false;
		}

		const std::string resolved_path = ResolveGameAssetPath(relative_or_data_path);
		if (resolved_path.size() >= destination_size)
		{
			destination[0] = '\0';
			return false;
		}

		strcpy(destination, resolved_path.c_str());
		return true;
	}
}

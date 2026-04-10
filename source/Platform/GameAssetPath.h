#pragma once

#include <stddef.h>
#include <string>

namespace platform
{
	void SetGameAssetRootOverride(const char* absolute_root_path);
	void ClearGameAssetRootOverride();
	void InitializeGameAssetRoot();
	const char* GetGameAssetRoot();
	std::string ResolveGameAssetPath(const char* relative_or_data_path);
	bool CopyResolvedGameAssetPath(char* destination, size_t destination_size, const char* relative_or_data_path);
}

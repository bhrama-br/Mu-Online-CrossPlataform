#pragma once

#include <string>

namespace platform
{
	struct LanguageAssetBootstrapState
	{
		bool found;
		bool used_fallback;
		std::string requested_language;
		std::string resolved_language;
		std::string folder_path;
		std::string text_path;
		std::string dialog_path;
		std::string item_path;
		std::string quest_path;
		std::string skill_path;
		std::string npc_name_path;
		std::string error_message;
	};

	bool InitializeLanguageAssetBootstrap(const char* requested_language, LanguageAssetBootstrapState* out_state);
}

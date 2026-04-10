#include "Platform/GameLanguageAssetBootstrap.h"

#include "Platform/GameAssetPath.h"

#include <cstdio>
#include <set>
#include <string>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace platform
{
	namespace
	{
		bool FileExists(const std::string& file_path)
		{
			FILE* file = fopen(file_path.c_str(), "rb");
			if (file == NULL)
			{
				return false;
			}

			fclose(file);
			return true;
		}

		bool DirectoryExists(const std::string& directory_path)
		{
#if defined(_WIN32)
			const DWORD attributes = GetFileAttributesA(directory_path.c_str());
			return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
			struct stat status = {};
			return stat(directory_path.c_str(), &status) == 0 && S_ISDIR(status.st_mode);
#endif
		}

		std::string TrimString(const std::string& input)
		{
			size_t begin = 0;
			while (begin < input.size() && (input[begin] == ' ' || input[begin] == '\t' || input[begin] == '\r' || input[begin] == '\n'))
			{
				++begin;
			}

			size_t end = input.size();
			while (end > begin && (input[end - 1] == ' ' || input[end - 1] == '\t' || input[end - 1] == '\r' || input[end - 1] == '\n'))
			{
				--end;
			}

			return input.substr(begin, end - begin);
		}

		std::string ToLowerAscii(const std::string& input)
		{
			std::string output = input;
			for (size_t index = 0; index < output.size(); ++index)
			{
				if (output[index] >= 'A' && output[index] <= 'Z')
				{
					output[index] = static_cast<char>(output[index] - 'A' + 'a');
				}
			}

			return output;
		}

		std::string CanonicalizeLanguage(const std::string& requested_language)
		{
			const std::string normalized = ToLowerAscii(TrimString(requested_language));
			if (normalized.empty() || normalized == "eng" || normalized == "english" || normalized == "en")
			{
				return "Eng";
			}

			if (normalized == "por" || normalized == "pt" || normalized == "ptbr" || normalized == "pt-br" ||
				normalized == "portuguese" || normalized == "portugues")
			{
				return "Por";
			}

			if (normalized == "spn" || normalized == "spa" || normalized == "es" || normalized == "spanish" || normalized == "espanol")
			{
				return "Spn";
			}

			if (normalized.size() >= 3)
			{
				std::string fallback = normalized.substr(0, 3);
				if (fallback[0] >= 'a' && fallback[0] <= 'z')
				{
					fallback[0] = static_cast<char>(fallback[0] - 'a' + 'A');
				}
				return fallback;
			}

			return "Eng";
		}

		void ResetState(LanguageAssetBootstrapState* state)
		{
			if (state == NULL)
			{
				return;
			}

			state->found = false;
			state->used_fallback = false;
			state->requested_language.clear();
			state->resolved_language.clear();
			state->folder_path.clear();
			state->text_path.clear();
			state->dialog_path.clear();
			state->item_path.clear();
			state->quest_path.clear();
			state->skill_path.clear();
			state->npc_name_path.clear();
			state->error_message.clear();
		}

		bool TryLanguage(const std::string& language, LanguageAssetBootstrapState* state)
		{
			if (state == NULL)
			{
				return false;
			}

			const std::string suffix = ToLowerAscii(language);
			const std::string local_folder = std::string("Data/Local/") + language;

			state->folder_path = ResolveGameAssetPath(local_folder.c_str());
			state->text_path = ResolveGameAssetPath((local_folder + "/Text_" + suffix + ".bmd").c_str());
			state->dialog_path = ResolveGameAssetPath((local_folder + "/Dialog_" + suffix + ".bmd").c_str());
			state->item_path = ResolveGameAssetPath((local_folder + "/item_" + suffix + ".bmd").c_str());
			state->quest_path = ResolveGameAssetPath((local_folder + "/Quest_" + suffix + ".bmd").c_str());
			state->skill_path = ResolveGameAssetPath((local_folder + "/skill_" + suffix + ".bmd").c_str());
			state->npc_name_path = ResolveGameAssetPath((local_folder + "/NpcName_" + suffix + ".txt").c_str());

			const bool found =
				DirectoryExists(state->folder_path) &&
				FileExists(state->text_path) &&
				FileExists(state->dialog_path) &&
				FileExists(state->item_path) &&
				FileExists(state->quest_path) &&
				FileExists(state->skill_path) &&
				FileExists(state->npc_name_path);

			if (found)
			{
				state->resolved_language = language;
				state->found = true;
			}

			return found;
		}
	}

	bool InitializeLanguageAssetBootstrap(const char* requested_language, LanguageAssetBootstrapState* out_state)
	{
		ResetState(out_state);
		if (out_state == NULL)
		{
			return false;
		}

		out_state->requested_language = requested_language != NULL ? requested_language : "";

		std::vector<std::string> candidates;
		const std::string requested_canonical = CanonicalizeLanguage(out_state->requested_language);
		candidates.push_back(requested_canonical);

		const char* fallback_languages[] = { "Eng", "Por", "Spn" };
		std::set<std::string> seen;
		for (size_t index = 0; index < candidates.size(); ++index)
		{
			seen.insert(candidates[index]);
		}

		for (size_t index = 0; index < sizeof(fallback_languages) / sizeof(fallback_languages[0]); ++index)
		{
			if (seen.insert(fallback_languages[index]).second)
			{
				candidates.push_back(fallback_languages[index]);
			}
		}

		for (size_t index = 0; index < candidates.size(); ++index)
		{
			if (TryLanguage(candidates[index], out_state))
			{
				out_state->used_fallback = (candidates[index] != requested_canonical);
				return true;
			}
		}

		out_state->resolved_language = requested_canonical;
		out_state->error_message = "Missing required Data/Local language assets";
		return false;
	}
}

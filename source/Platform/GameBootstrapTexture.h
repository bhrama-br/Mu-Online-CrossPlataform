#pragma once

#include <string>

namespace platform
{
	struct BootstrapTexture
	{
		bool loaded;
		unsigned int texture_id;
		int width;
		int height;
		std::string source_path;
		std::string error_message;
	};

	bool LoadBootstrapTextureFromFile(const char* file_path, BootstrapTexture* out_texture);
	bool LoadBootstrapTextureFromTgaLikeFile(const char* file_path, BootstrapTexture* out_texture);
	bool LoadBootstrapTextureFromJpegLikeFile(const char* file_path, BootstrapTexture* out_texture);
	bool LoadBootstrapTextureFromColorAndAlphaFiles(const char* color_file_path, const char* alpha_file_path, BootstrapTexture* out_texture);
	void DestroyBootstrapTexture(BootstrapTexture* texture);
}

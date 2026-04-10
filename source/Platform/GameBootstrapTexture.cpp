#include "Platform/GameBootstrapTexture.h"

#if defined(__ANDROID__)
#include <android/bitmap.h>
#include <android/imagedecoder.h>
#include <GLES2/gl2.h>
#include <dlfcn.h>
#else
#include "stdafx.h"
#include "Jpeglib.h"
#endif

#include "Protect.h"
#include "Platform/TextureUploadCompat.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <setjmp.h>
#include <vector>

namespace platform
{
	namespace
	{
#if !defined(__ANDROID__)
		struct BootstrapJpegErrorManager
		{
			jpeg_error_mgr pub;
			jmp_buf setjmp_buffer;
		};
#endif

#if defined(__ANDROID__)
		struct AndroidImageDecoderApi
		{
			bool attempted;
			bool available;
			void* library_handle;
			int (*create_from_buffer)(const void* buffer, size_t length, AImageDecoder** out_decoder);
			void (*destroy)(AImageDecoder* decoder);
			int (*set_android_bitmap_format)(AImageDecoder* decoder, int32_t format);
			const AImageDecoderHeaderInfo* (*get_header_info)(const AImageDecoder* decoder);
			int32_t (*get_header_width)(const AImageDecoderHeaderInfo* header_info);
			int32_t (*get_header_height)(const AImageDecoderHeaderInfo* header_info);
			size_t (*get_minimum_stride)(AImageDecoder* decoder);
			int (*decode_image)(AImageDecoder* decoder, void* pixels, size_t stride, size_t size);
		};
#endif

		void ResetTexture(BootstrapTexture* texture)
		{
			if (texture == NULL)
			{
				return;
			}

			texture->loaded = false;
			texture->texture_id = 0;
			texture->width = 0;
			texture->height = 0;
			texture->source_path.clear();
			texture->error_message.clear();
		}

		void SetErrorMessage(std::string* error_message, const char* message)
		{
			if (error_message != NULL)
			{
				*error_message = message != NULL ? message : "";
			}
		}

		bool ReadWholeFile(const char* file_path, std::vector<unsigned char>* bytes)
		{
			if (file_path == NULL || file_path[0] == '\0' || bytes == NULL)
			{
				return false;
			}

			bytes->clear();

			FILE* file = fopen(file_path, "rb");
			if (file == NULL)
			{
				return false;
			}

			fseek(file, 0, SEEK_END);
			const long size = ftell(file);
			fseek(file, 0, SEEK_SET);
			if (size <= 0)
			{
				fclose(file);
				return false;
			}

			bytes->resize(static_cast<size_t>(size));
			const size_t read_count = fread(&(*bytes)[0], 1, static_cast<size_t>(size), file);
			fclose(file);
			return read_count == static_cast<size_t>(size);
		}

		bool DecodeProtectedAssetBytes(std::vector<unsigned char>* file_bytes, std::string* error_message)
		{
			if (file_bytes == NULL)
			{
				return false;
			}

			static const unsigned char kFileProtectHeader[8] = { 0x68, 0xA2, 0xD2, 0x20, 0xA4, 0x43, 0x41, 0xDE };
			static const unsigned char kFileProtectXorTable[16] = { 0x13, 0xA4, 0x85, 0x1F, 0x1E, 0x19, 0x6F, 0xBE, 0x8A, 0x22, 0x29, 0xDE, 0x03, 0x6E, 0x99, 0xB7 };
			if (file_bytes->size() < sizeof(kFileProtectHeader) ||
				memcmp(&(*file_bytes)[0], kFileProtectHeader, sizeof(kFileProtectHeader)) != 0)
			{
				return true;
			}

			if (gProtect == NULL)
			{
				SetErrorMessage(error_message, "Protected bootstrap texture requires loaded Configs.xtm");
				return false;
			}

			const size_t private_code_length = strlen(gProtect->m_MainInfo.m_PrivateCode);
			if (private_code_length == 0)
			{
				SetErrorMessage(error_message, "Protected bootstrap texture has empty private code");
				return false;
			}

			for (size_t index = 0; index < (file_bytes->size() - sizeof(kFileProtectHeader)); ++index)
			{
				const unsigned char private_code = static_cast<unsigned char>(gProtect->m_MainInfo.m_PrivateCode[index % private_code_length]);
				(*file_bytes)[sizeof(kFileProtectHeader) + index] -= kFileProtectXorTable[private_code % sizeof(kFileProtectXorTable)];
				(*file_bytes)[sizeof(kFileProtectHeader) + index] ^= kFileProtectXorTable[index % sizeof(kFileProtectXorTable)];
			}

			file_bytes->erase(file_bytes->begin(), file_bytes->begin() + sizeof(kFileProtectHeader));
			return true;
		}

		bool UploadBootstrapTexture(const std::vector<unsigned char>& rgba_pixels, int width, int height, BootstrapTexture* out_texture)
		{
			if (out_texture == NULL || width <= 0 || height <= 0 || rgba_pixels.empty())
			{
				return false;
			}

			glGenTextures(1, &out_texture->texture_id);
			if (out_texture->texture_id == 0)
			{
				out_texture->error_message = "glGenTextures failed";
				return false;
			}

			glBindTexture(GL_TEXTURE_2D, out_texture->texture_id);
			LegacyCompatibleTexImage2D(
				GL_TEXTURE_2D,
				0,
				4,
				width,
				height,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				&rgba_pixels[0]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			out_texture->width = width;
			out_texture->height = height;
			out_texture->loaded = true;
			return true;
		}

		bool DecodeTgaLikeBuffer(const std::vector<unsigned char>& file_bytes, std::vector<unsigned char>* rgba_pixels, int* width, int* height, std::string* error_message)
		{
			if (rgba_pixels == NULL || width == NULL || height == NULL)
			{
				return false;
			}

			rgba_pixels->clear();
			*width = 0;
			*height = 0;

			const size_t kHeaderOffset = 4;
			const size_t kTgaHeaderSize = 18;
			if (file_bytes.size() < (kHeaderOffset + kTgaHeaderSize))
			{
				if (error_message != NULL)
				{
					*error_message = "TGA buffer too small";
				}
				return false;
			}

			const unsigned char* header = &file_bytes[kHeaderOffset];
			const unsigned char image_type = header[2];
			const unsigned short image_width = static_cast<unsigned short>(header[12] | (header[13] << 8));
			const unsigned short image_height = static_cast<unsigned short>(header[14] | (header[15] << 8));
			const unsigned char pixel_depth = header[16];
			const unsigned char image_descriptor = header[17];

			if (image_type != 2 || image_width == 0 || image_height == 0 || pixel_depth != 32)
			{
				if (error_message != NULL)
				{
					*error_message = "Unsupported TGA format in bootstrap texture";
				}
				return false;
			}

			const size_t pixel_data_offset = kHeaderOffset + kTgaHeaderSize;
			const size_t source_stride = static_cast<size_t>(image_width) * 4u;
			const size_t required_size = pixel_data_offset + static_cast<size_t>(image_height) * source_stride;
			if (file_bytes.size() < required_size)
			{
				if (error_message != NULL)
				{
					*error_message = "TGA payload truncated";
				}
				return false;
			}

			rgba_pixels->resize(static_cast<size_t>(image_width) * static_cast<size_t>(image_height) * 4u);
			const bool top_to_bottom = (image_descriptor & 0x20u) != 0;
			for (unsigned int y = 0; y < image_height; ++y)
			{
				const unsigned int source_y = top_to_bottom ? (image_height - 1u - y) : y;
				const unsigned char* source_row = &file_bytes[pixel_data_offset + static_cast<size_t>(source_y) * source_stride];
				unsigned char* destination_row = &(*rgba_pixels)[static_cast<size_t>(y) * source_stride];
				for (unsigned int x = 0; x < image_width; ++x)
				{
					const unsigned char* source_pixel = source_row + static_cast<size_t>(x) * 4u;
					unsigned char* destination_pixel = destination_row + static_cast<size_t>(x) * 4u;
					destination_pixel[0] = source_pixel[2];
					destination_pixel[1] = source_pixel[1];
					destination_pixel[2] = source_pixel[0];
					destination_pixel[3] = source_pixel[3];
				}
			}

			*width = static_cast<int>(image_width);
			*height = static_cast<int>(image_height);
			return true;
		}

		size_t FindJpegPayloadOffset(const std::vector<unsigned char>& file_bytes)
		{
			if (file_bytes.size() >= 26 &&
				file_bytes[24] == 0xFF && file_bytes[25] == 0xD8)
			{
				return 24;
			}

			if (file_bytes.size() >= 2 &&
				file_bytes[0] == 0xFF && file_bytes[1] == 0xD8)
			{
				return 0;
			}

			const size_t scan_limit = file_bytes.size() < 96 ? file_bytes.size() : 96;
			for (size_t index = 0; (index + 2) < scan_limit; ++index)
			{
				if (file_bytes[index] == 0xFF && file_bytes[index + 1] == 0xD8 && file_bytes[index + 2] == 0xFF)
				{
					return index;
				}
			}

			return static_cast<size_t>(-1);
		}

#if !defined(__ANDROID__)
		void BootstrapJpegErrorExit(j_common_ptr cinfo)
		{
			BootstrapJpegErrorManager* error_manager = reinterpret_cast<BootstrapJpegErrorManager*>(cinfo->err);
			longjmp(error_manager->setjmp_buffer, 1);
		}

		bool DecodeJpegBufferDesktop(const unsigned char* jpeg_bytes, size_t jpeg_size, std::vector<unsigned char>* rgba_pixels, int* width, int* height, std::string* error_message)
		{
			if (jpeg_bytes == NULL || jpeg_size == 0 || rgba_pixels == NULL || width == NULL || height == NULL)
			{
				return false;
			}

			rgba_pixels->clear();
			*width = 0;
			*height = 0;

			jpeg_decompress_struct cinfo = {};
			BootstrapJpegErrorManager error_manager = {};
			cinfo.err = jpeg_std_error(&error_manager.pub);
			error_manager.pub.error_exit = BootstrapJpegErrorExit;

			if (setjmp(error_manager.setjmp_buffer))
			{
				jpeg_destroy_decompress(&cinfo);
				SetErrorMessage(error_message, "JPEG decode failed");
				return false;
			}

			jpeg_create_decompress(&cinfo);
			jpeg_mem_src(&cinfo, const_cast<unsigned char*>(jpeg_bytes), jpeg_size);
			jpeg_read_header(&cinfo, TRUE);
			jpeg_start_decompress(&cinfo);

			const int output_width = static_cast<int>(cinfo.output_width);
			const int output_height = static_cast<int>(cinfo.output_height);
			const int output_components = static_cast<int>(cinfo.output_components);
			if (output_width <= 0 || output_height <= 0 || (output_components != 1 && output_components != 3 && output_components != 4))
			{
				jpeg_finish_decompress(&cinfo);
				jpeg_destroy_decompress(&cinfo);
				SetErrorMessage(error_message, "Unsupported bootstrap JPEG format");
				return false;
			}

			rgba_pixels->resize(static_cast<size_t>(output_width) * static_cast<size_t>(output_height) * 4u);
			const int row_stride = output_width * output_components;
			JSAMPARRAY row_buffer = (*cinfo.mem->alloc_sarray)(reinterpret_cast<j_common_ptr>(&cinfo), JPOOL_IMAGE, row_stride, 1);
			while (cinfo.output_scanline < cinfo.output_height)
			{
				jpeg_read_scanlines(&cinfo, row_buffer, 1);
				const unsigned char* source_row = row_buffer[0];
				unsigned char* destination_row = &(*rgba_pixels)[static_cast<size_t>(cinfo.output_scanline - 1) * static_cast<size_t>(output_width) * 4u];
				for (int x = 0; x < output_width; ++x)
				{
					unsigned char* destination_pixel = destination_row + static_cast<size_t>(x) * 4u;
					if (output_components == 1)
					{
						const unsigned char value = source_row[x];
						destination_pixel[0] = value;
						destination_pixel[1] = value;
						destination_pixel[2] = value;
						destination_pixel[3] = 255;
					}
					else
					{
						const unsigned char* source_pixel = source_row + static_cast<size_t>(x) * static_cast<size_t>(output_components);
						destination_pixel[0] = source_pixel[0];
						destination_pixel[1] = source_pixel[1];
						destination_pixel[2] = source_pixel[2];
						destination_pixel[3] = output_components == 4 ? source_pixel[3] : 255;
					}
				}
			}

			jpeg_finish_decompress(&cinfo);
			jpeg_destroy_decompress(&cinfo);

			*width = output_width;
			*height = output_height;
			return true;
		}
#else
		AndroidImageDecoderApi& GetAndroidImageDecoderApi()
		{
			static AndroidImageDecoderApi api = {};
			if (api.attempted)
			{
				return api;
			}

			api.attempted = true;
			api.library_handle = dlopen("libjnigraphics.so", RTLD_NOW | RTLD_LOCAL);
			if (api.library_handle == NULL)
			{
				return api;
			}

			api.create_from_buffer = reinterpret_cast<int (*)(const void*, size_t, AImageDecoder**)>(dlsym(api.library_handle, "AImageDecoder_createFromBuffer"));
			api.destroy = reinterpret_cast<void (*)(AImageDecoder*)>(dlsym(api.library_handle, "AImageDecoder_delete"));
			api.set_android_bitmap_format = reinterpret_cast<int (*)(AImageDecoder*, int32_t)>(dlsym(api.library_handle, "AImageDecoder_setAndroidBitmapFormat"));
			api.get_header_info = reinterpret_cast<const AImageDecoderHeaderInfo* (*)(const AImageDecoder*)>(dlsym(api.library_handle, "AImageDecoder_getHeaderInfo"));
			api.get_header_width = reinterpret_cast<int32_t (*)(const AImageDecoderHeaderInfo*)>(dlsym(api.library_handle, "AImageDecoderHeaderInfo_getWidth"));
			api.get_header_height = reinterpret_cast<int32_t (*)(const AImageDecoderHeaderInfo*)>(dlsym(api.library_handle, "AImageDecoderHeaderInfo_getHeight"));
			api.get_minimum_stride = reinterpret_cast<size_t (*)(AImageDecoder*)>(dlsym(api.library_handle, "AImageDecoder_getMinimumStride"));
			api.decode_image = reinterpret_cast<int (*)(AImageDecoder*, void*, size_t, size_t)>(dlsym(api.library_handle, "AImageDecoder_decodeImage"));

			api.available =
				api.create_from_buffer != NULL &&
				api.destroy != NULL &&
				api.set_android_bitmap_format != NULL &&
				api.get_header_info != NULL &&
				api.get_header_width != NULL &&
				api.get_header_height != NULL &&
				api.get_minimum_stride != NULL &&
				api.decode_image != NULL;
			return api;
		}

		bool DecodeJpegBufferAndroid(const unsigned char* jpeg_bytes, size_t jpeg_size, std::vector<unsigned char>* rgba_pixels, int* width, int* height, std::string* error_message)
		{
			if (jpeg_bytes == NULL || jpeg_size == 0 || rgba_pixels == NULL || width == NULL || height == NULL)
			{
				return false;
			}

			rgba_pixels->clear();
			*width = 0;
			*height = 0;

			AndroidImageDecoderApi& api = GetAndroidImageDecoderApi();
			if (!api.available)
			{
				SetErrorMessage(error_message, "AImageDecoder unavailable on this Android runtime");
				return false;
			}

			AImageDecoder* decoder = NULL;
			const int create_result = api.create_from_buffer(jpeg_bytes, jpeg_size, &decoder);
			if (create_result != ANDROID_IMAGE_DECODER_SUCCESS || decoder == NULL)
			{
				SetErrorMessage(error_message, "AImageDecoder_createFromBuffer failed");
				return false;
			}

			const int format_result = api.set_android_bitmap_format(decoder, ANDROID_BITMAP_FORMAT_RGBA_8888);
			if (format_result != ANDROID_IMAGE_DECODER_SUCCESS)
			{
				api.destroy(decoder);
				SetErrorMessage(error_message, "AImageDecoder_setAndroidBitmapFormat failed");
				return false;
			}

			const AImageDecoderHeaderInfo* header_info = api.get_header_info(decoder);
			const int output_width = header_info != NULL ? static_cast<int>(api.get_header_width(header_info)) : 0;
			const int output_height = header_info != NULL ? static_cast<int>(api.get_header_height(header_info)) : 0;
			if (output_width <= 0 || output_height <= 0)
			{
				api.destroy(decoder);
				SetErrorMessage(error_message, "AImageDecoder returned invalid image size");
				return false;
			}

			const size_t stride = api.get_minimum_stride(decoder);
			if (stride < static_cast<size_t>(output_width) * 4u)
			{
				api.destroy(decoder);
				SetErrorMessage(error_message, "AImageDecoder returned invalid stride");
				return false;
			}

			std::vector<unsigned char> stride_pixels(stride * static_cast<size_t>(output_height));
			const int decode_result = api.decode_image(decoder, &stride_pixels[0], stride, stride_pixels.size());
			api.destroy(decoder);
			if (decode_result != ANDROID_IMAGE_DECODER_SUCCESS)
			{
				SetErrorMessage(error_message, "AImageDecoder_decodeImage failed");
				return false;
			}

			rgba_pixels->resize(static_cast<size_t>(output_width) * static_cast<size_t>(output_height) * 4u);
			for (int y = 0; y < output_height; ++y)
			{
				memcpy(
					&(*rgba_pixels)[static_cast<size_t>(y) * static_cast<size_t>(output_width) * 4u],
					&stride_pixels[static_cast<size_t>(y) * stride],
					static_cast<size_t>(output_width) * 4u);
			}

			*width = output_width;
			*height = output_height;
			return true;
		}
#endif

		bool DecodeJpegLikeBuffer(const std::vector<unsigned char>& file_bytes, std::vector<unsigned char>* rgba_pixels, int* width, int* height, std::string* error_message)
		{
			const size_t jpeg_offset = FindJpegPayloadOffset(file_bytes);
			if (jpeg_offset == static_cast<size_t>(-1) || jpeg_offset >= file_bytes.size())
			{
				SetErrorMessage(error_message, "Unsupported bootstrap JPEG payload");
				return false;
			}

#if defined(__ANDROID__)
			return DecodeJpegBufferAndroid(&file_bytes[jpeg_offset], file_bytes.size() - jpeg_offset, rgba_pixels, width, height, error_message);
#else
			return DecodeJpegBufferDesktop(&file_bytes[jpeg_offset], file_bytes.size() - jpeg_offset, rgba_pixels, width, height, error_message);
#endif
		}

		const char* FindExtension(const char* file_path)
		{
			if (file_path == NULL)
			{
				return NULL;
			}

			const char* extension = strrchr(file_path, '.');
			return extension != NULL ? extension + 1 : NULL;
		}

		bool ExtensionEquals(const char* extension, const char* expected_uppercase)
		{
			if (extension == NULL || expected_uppercase == NULL)
			{
				return false;
			}

			while (*extension != '\0' && *expected_uppercase != '\0')
			{
				if (std::toupper(static_cast<unsigned char>(*extension)) != *expected_uppercase)
				{
					return false;
				}
				++extension;
				++expected_uppercase;
			}

			return *extension == '\0' && *expected_uppercase == '\0';
		}
	}

	bool LoadBootstrapTextureFromFile(const char* file_path, BootstrapTexture* out_texture)
	{
		const char* extension = FindExtension(file_path);
		if (ExtensionEquals(extension, "OZT") || ExtensionEquals(extension, "TGA"))
		{
			return LoadBootstrapTextureFromTgaLikeFile(file_path, out_texture);
		}

		if (ExtensionEquals(extension, "OZJ") || ExtensionEquals(extension, "JPG") || ExtensionEquals(extension, "JPEG"))
		{
			return LoadBootstrapTextureFromJpegLikeFile(file_path, out_texture);
		}

		ResetTexture(out_texture);
		if (out_texture != NULL)
		{
			out_texture->source_path = file_path != NULL ? file_path : "";
			out_texture->error_message = "Unsupported bootstrap texture extension";
		}
		return false;
	}

	bool LoadBootstrapTextureFromTgaLikeFile(const char* file_path, BootstrapTexture* out_texture)
	{
		ResetTexture(out_texture);
		if (out_texture == NULL)
		{
			return false;
		}

		out_texture->source_path = file_path != NULL ? file_path : "";

		std::vector<unsigned char> file_bytes;
		if (!ReadWholeFile(file_path, &file_bytes))
		{
			out_texture->error_message = "Unable to read texture file";
			return false;
		}

		if (!DecodeProtectedAssetBytes(&file_bytes, &out_texture->error_message))
		{
			return false;
		}

		std::vector<unsigned char> rgba_pixels;
		if (!DecodeTgaLikeBuffer(file_bytes, &rgba_pixels, &out_texture->width, &out_texture->height, &out_texture->error_message))
		{
			return false;
		}

		return UploadBootstrapTexture(rgba_pixels, out_texture->width, out_texture->height, out_texture);
	}

	bool LoadBootstrapTextureFromJpegLikeFile(const char* file_path, BootstrapTexture* out_texture)
	{
		ResetTexture(out_texture);
		if (out_texture == NULL)
		{
			return false;
		}

		out_texture->source_path = file_path != NULL ? file_path : "";

		std::vector<unsigned char> file_bytes;
		if (!ReadWholeFile(file_path, &file_bytes))
		{
			out_texture->error_message = "Unable to read texture file";
			return false;
		}

		if (!DecodeProtectedAssetBytes(&file_bytes, &out_texture->error_message))
		{
			return false;
		}

		std::vector<unsigned char> rgba_pixels;
		if (!DecodeJpegLikeBuffer(file_bytes, &rgba_pixels, &out_texture->width, &out_texture->height, &out_texture->error_message))
		{
			return false;
		}

		return UploadBootstrapTexture(rgba_pixels, out_texture->width, out_texture->height, out_texture);
	}

	bool LoadBootstrapTextureFromColorAndAlphaFiles(const char* color_file_path, const char* alpha_file_path, BootstrapTexture* out_texture)
	{
		ResetTexture(out_texture);
		if (out_texture == NULL)
		{
			return false;
		}

		out_texture->source_path = color_file_path != NULL ? color_file_path : "";

		std::vector<unsigned char> color_file_bytes;
		if (!ReadWholeFile(color_file_path, &color_file_bytes))
		{
			out_texture->error_message = "Unable to read color texture file";
			return false;
		}
		if (!DecodeProtectedAssetBytes(&color_file_bytes, &out_texture->error_message))
		{
			return false;
		}

		std::vector<unsigned char> alpha_file_bytes;
		if (!ReadWholeFile(alpha_file_path, &alpha_file_bytes))
		{
			out_texture->error_message = "Unable to read alpha texture file";
			return false;
		}
		if (!DecodeProtectedAssetBytes(&alpha_file_bytes, &out_texture->error_message))
		{
			return false;
		}

		std::vector<unsigned char> color_rgba_pixels;
		int color_width = 0;
		int color_height = 0;
		if (!DecodeJpegLikeBuffer(color_file_bytes, &color_rgba_pixels, &color_width, &color_height, &out_texture->error_message))
		{
			return false;
		}

		std::vector<unsigned char> alpha_rgba_pixels;
		int alpha_width = 0;
		int alpha_height = 0;
		if (!DecodeTgaLikeBuffer(alpha_file_bytes, &alpha_rgba_pixels, &alpha_width, &alpha_height, &out_texture->error_message))
		{
			return false;
		}

		if (color_width <= 0 || color_height <= 0 || alpha_width <= 0 || alpha_height <= 0)
		{
			out_texture->error_message = "Invalid color/alpha texture size";
			return false;
		}

		std::vector<unsigned char> combined_rgba_pixels(static_cast<size_t>(color_width) * static_cast<size_t>(color_height) * 4u, 0);
		for (int y = 0; y < color_height; ++y)
		{
			const int alpha_y = (y * alpha_height) / color_height;
			for (int x = 0; x < color_width; ++x)
			{
				const int alpha_x = (x * alpha_width) / color_width;
				const size_t color_offset = (static_cast<size_t>(y) * static_cast<size_t>(color_width) + static_cast<size_t>(x)) * 4u;
				const size_t alpha_offset = (static_cast<size_t>(alpha_y) * static_cast<size_t>(alpha_width) + static_cast<size_t>(alpha_x)) * 4u;
				combined_rgba_pixels[color_offset + 0u] = color_rgba_pixels[color_offset + 0u];
				combined_rgba_pixels[color_offset + 1u] = color_rgba_pixels[color_offset + 1u];
				combined_rgba_pixels[color_offset + 2u] = color_rgba_pixels[color_offset + 2u];

				const unsigned char alpha_value = alpha_rgba_pixels[alpha_offset + 3u] != 0
					? alpha_rgba_pixels[alpha_offset + 3u]
					: static_cast<unsigned char>(
						(alpha_rgba_pixels[alpha_offset + 0u] +
						 alpha_rgba_pixels[alpha_offset + 1u] +
						 alpha_rgba_pixels[alpha_offset + 2u]) / 3u);
				combined_rgba_pixels[color_offset + 3u] = alpha_value;
			}
		}

		out_texture->source_path = std::string(color_file_path != NULL ? color_file_path : "") +
			"|" + std::string(alpha_file_path != NULL ? alpha_file_path : "");
		out_texture->width = color_width;
		out_texture->height = color_height;
		return UploadBootstrapTexture(combined_rgba_pixels, out_texture->width, out_texture->height, out_texture);
	}

	void DestroyBootstrapTexture(BootstrapTexture* texture)
	{
		if (texture == NULL)
		{
			return;
		}

		if (texture->texture_id != 0)
		{
			glDeleteTextures(1, &texture->texture_id);
		}

		ResetTexture(texture);
	}
}

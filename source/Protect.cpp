#if defined(_WIN32)
#include "stdafx.h"
#else
#include <cstdio>
#include <cstring>
#endif

#include "Protect.h"
#include "CCRC32.h"

CProtect* gProtect;

namespace
{
	static BYTE bBuxCode[3] = { 0xF2, 0x95, 0x54 };

	template <typename T>
	bool ReadWholeFileExact(const char* path, T* out_value)
	{
		if (path == NULL || out_value == NULL)
		{
			return false;
		}

		FILE* file = fopen(path, "rb");
		if (file == NULL)
		{
			return false;
		}

		if (fseek(file, 0, SEEK_END) != 0)
		{
			fclose(file);
			return false;
		}

		const long file_size = ftell(file);
		if (file_size != static_cast<long>(sizeof(T)))
		{
			fclose(file);
			return false;
		}

		rewind(file);
		const size_t bytes_read = fread(out_value, 1, sizeof(T), file);
		fclose(file);
		return bytes_read == sizeof(T);
	}
}

CProtect::CProtect() // OK
{
}

CProtect::~CProtect() // OK
{
}

bool CProtect::ReadMainFile(const char* path) // OK
{
	CCRC32 CRC32;
	unsigned long client_file_crc = 0;

	if (CRC32.FileCRC(path, &client_file_crc, 1024) == 0)
	{
		return 0;
	}

	this->m_ClientFileCRC = static_cast<DWORD>(client_file_crc);

	if (!ReadWholeFileExact(path, &this->m_MainInfo))
	{
		return 0;
	}

	for (int n = 0; n < static_cast<int>(sizeof(MAIN_FILE_INFO)); n++)
	{
		((BYTE*)&this->m_MainInfo)[n] ^= bBuxCode[n % 3];
	}

	return 1;
}

bool CProtect::ReadCustomJewelConfig(const char* path)
{
	if (!ReadWholeFileExact(path, &this->m_CustomJewel))
	{
		return 0;
	}

	for (int n = 0; n < static_cast<int>(sizeof(LOAD_CUSTOM_JEWEL_INFO)); n++)
	{
		((BYTE*)&this->m_CustomJewel)[n] += (BYTE)(0xFC ^ ((n >> 8) & 0xFF));
		((BYTE*)&this->m_CustomJewel)[n] ^= (BYTE)(0xCA ^ (n & 0xFF));
	}

	return 1;
}

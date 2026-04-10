// PacketManager.h: interface for the CPacketManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#if !defined(_WIN32)
#include <stdint.h>
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
#ifndef MAKEWORD
#define MAKEWORD(a, b) ((WORD)(((BYTE)((a) & 0xff)) | ((WORD)((BYTE)((b) & 0xff))) << 8))
#endif
#endif

#pragma pack(push, 1)
struct ENCDEC_HEADER
{
	WORD header;
	DWORD size;
};
#pragma pack(pop)

static_assert(sizeof(ENCDEC_HEADER) == 6, "ENCDEC_HEADER must remain packed to match Enc1.dat/Dec2.dat");

struct ENCDEC_DATA
{
	DWORD Modulus[4];
	DWORD Key[4];
	DWORD Xor[4];
};

class CPacketManager
{
public:
	CPacketManager();
	virtual ~CPacketManager();
	void Init();
	bool LoadEncryptionKey(const char* name);
	bool LoadDecryptionKey(const char* name);
	bool LoadKey(const char* name,WORD header,bool type);
	int Encrypt(BYTE* lpTarget,BYTE* lpSource,int size);
	int Decrypt(BYTE* lpTarget,BYTE* lpSource,int size);
	int EncryptBlock(BYTE* lpTarget,BYTE* lpSource,int size);
	int DecryptBlock(BYTE* lpTarget,BYTE* lpSource);
	int AddBits(BYTE* lpTarget,int TargetBitPos,BYTE* lpSource,int SourceBitPos,int size);
	int GetByteOfBit(int value);
	void Shift(BYTE* lpBuff,int size,int ShiftSize);
	bool AddData(BYTE* lpBuff,int size);
	bool ExtractPacket(BYTE* lpBuff);
	void XorData(int start,int end);
private:
	ENCDEC_DATA m_Encryption;
	ENCDEC_DATA m_Decryption;
	DWORD m_SaveLoadXor[4];
	BYTE m_buff[2048];
	DWORD m_size;
	BYTE m_XorFilter[32];
};

extern CPacketManager gPacketManager;

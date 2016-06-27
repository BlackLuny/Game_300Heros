// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "detours.h"
#include "HotKey.h"
#include <stdlib.h>
#include <set>
#include <string>
#include "md5_c.h"
#include "xorstr.h"
#include <vector>
#include "SESDK.h"

#pragma pack(1)

int md5file(char *real_num2, char *file)
{
	FILE* fp;
	long f_size;
	int numread;
	int i;

	char r_num[33] = "";
	char *buffer = NULL;
	unsigned char binary[17];
	struct MD5Context context;

	fp = fopen(file, "rb");

	if (fp == NULL) {
		return (-1);
	}

	MD5Init(&context);

	fseek(fp, 0L, SEEK_END);
	f_size = ftell(fp);
	buffer = (char*)malloc(f_size + 1);
	fseek(fp, 0L, SEEK_SET);

	while ((numread = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
		MD5Update(&context, (const unsigned char*)buffer, numread);
	}

	MD5Final(binary, &context);
	free(buffer);

	for (i = 0; i < 16; i++) {
		sprintf(r_num, "%s%02X", r_num, binary[i]);
	}

	strcpy(real_num2, r_num);

	fclose(fp);
	fp = NULL;

	return (0);
}

struct file_node_s
{
	char f_name[260];
	unsigned int position;
	unsigned int compr_size;
	unsigned int source_size;
	char f_reserved[0x20];
};

struct file_header_s
{
	char header[0x32];
	unsigned int f_count;
};

#pragma pack()

void Initialize();
void UnInitialize();
bool(*pSendDestroyPacket)(BOOL b, BYTE *p);
bool(__stdcall *pReceive)(net_header *hdr);
LPTHREAD_START_ROUTINE g_pStartWindowThread = (LPTHREAD_START_ROUTINE)0x00467860;
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		Initialize();
		break;
	case DLL_PROCESS_DETACH:
		UnInitialize();
		break;
	}
	return TRUE;
}

class CNetwork
{
public:
	virtual void none1() = 0;
	virtual void none2() = 0;
	virtual void none3() = 0;
	virtual bool send(net_header *hdr) = 0;
};

CNetwork *pNetwork;

CNetwork *network() {
	CNetwork *_pNetwork;
	__asm
	{
		pushad;
		mov eax, 0x0084CBE0;
		call eax;
		mov _pNetwork, eax;
		popad;
	}
	return _pNetwork;
}


DWORD GetPlayerRole() {
	DWORD result = 0;

	__asm mov eax, 0x00453780;
	__asm call eax;
	__asm mov result, eax;
	return result;
}



DWORD GetEncodePtr(DWORD valueA, DWORD valueB)
{
	DWORD result = 0;
	DWORD dwClassPtr = GetPlayerRole();
	dwClassPtr += 0x8CC0;
	SE_PROTECT_START_MUTATION;

	__asm
	{
		pushad;
		mov ecx, dwClassPtr;
		push valueB;
		push valueA;
		mov eax, 0x007F0F20;
		call eax;
		mov result, eax;
		popad;
	}

	SE_PROTECT_END;
	return result;
}

VOID BuildDestroyPacket(uint8_t serial, uint8_t* index)
{
	DWORD dwResult;

	net_destroy_packet pkt = { 0 };

	SE_PROTECT_START_MUTATION;

	pkt.hdr.idenfitier = 0x426D;
	pkt.hdr.length = sizeof(pkt);
	pkt.serial = serial;
	pkt.index[0] = index[0];
	pkt.index[1] = index[1];
	dwResult = GetEncodePtr(index[0], index[1]);
	if (dwResult == 0) {
		pkt.check1 = 0;
		pkt.check2 = 0;
	}
	else {
		pkt.check1 = *(DWORD*)(dwResult + 0x5);
		pkt.check2 = *(DWORD*)(dwResult + 0x9);
	}

	network()->send(&pkt.hdr);

	SE_PROTECT_END;
}
unsigned char GetSerial()
{
	unsigned char r;

	__asm mov eax, 0x007F0EE0;
	__asm call eax;
	__asm mov r, al;

	return r;
}
bool SendDestroyPacket(BOOL b, BYTE *p)
{
	uint8_t index[2];
	uint8_t serial;

	if (b == FALSE) {
		return pSendDestroyPacket(b, p);
	}

	index[0] = p[0xC];
	index[1] = p[0xD];

	serial = GetSerial();
	BuildDestroyPacket(serial, index);
	return true;
}


void *windowUIClass;

void SetScreenNonBlock()
{
	if (!windowUIClass) {
		return;
	}

	__asm
	{
		pushfd;
		pushad;

		push 0;
		push 0;
		mov ecx, windowUIClass;
		mov eax, 0x004C1D60;
		call eax;

		popad;
		popfd;
	}
}
bool __stdcall RealSendMsg(net_header *hdr);

bool MergeItemPacket(net_header *hdr)
{
	bool result;
	net_header *nhdr;
	uint32_t len = hdr->length + 0x4;
	unsigned char *buf = (unsigned char *)malloc(len);
	memset(buf, 0, len);
	memcpy(buf, hdr, hdr->length);

	nhdr = (net_header *)buf;
	nhdr->length += 0x4;

	result = RealSendMsg(nhdr);

	free(buf);
	return result;
}
void HeroDieCheck(net_header *hdr)
{
	// 	unsigned char sig[] = { 0x00,0x0B,0x08,0xB1,0x24 };
	// 	if (hdr->idenfitier == 0x2B19)
	// 	{
	// 		if (memcmp(sig, &hdr[1], sizeof(sig)) == 0)
	// 		{
	// 			SetScreenNonBlock();
	// 		}
	// 	}
}

void *pRecordwindowUIClass;
__declspec(naked) void __asm_RecordwindowUIClass()
{
	__asm
	{
		pushad;
		pushfd;

		mov windowUIClass, esi;

		popfd;
		popad;
		jmp pRecordwindowUIClass;
	}
}

struct OItemInfo
{
	unsigned char data[0xB5];
};

struct NItemInfo
{
	unsigned char data[0xc4];
};

void UpdateItemUpdater(net_header *hdr)
{
	SE_PROTECT_START_MUTATION;
	if (hdr->idenfitier == 0x426C)
	{
		unsigned char *p = (unsigned char *)hdr;
		int count = *(unsigned short*)&p[0xD];
		unsigned char *buf = (unsigned char*)calloc(count, sizeof(struct OItemInfo));

		for (int i = 0; i < count; i++)
		{
			unsigned char *src = (p + 0xe) + i * sizeof(struct NItemInfo);
			unsigned char *dst = buf + i * sizeof(struct OItemInfo);
			memcpy(dst, src, sizeof(struct OItemInfo));
		}

		hdr->length = count * sizeof(struct OItemInfo) + 0xe;
		memcpy(&p[0xe], buf, count * sizeof(struct OItemInfo));
		free(buf);
	}
	SE_PROTECT_END;
}

void* g_UncompressAuth = (void*)0x0084CD30;
int(*uncompress) (unsigned char *dest, unsigned int *destLen, const unsigned char *source, unsigned int sourceLen) = (int(*)(unsigned char *dest, unsigned int *destLen, const unsigned char *source, unsigned int sourceLen))0x009106E0;
unsigned char g_AuthPkg[0x9687];

struct net_header *compr_hdr;
bool FixLoginAuthInfo(net_header *hdr)
{
	if (hdr->idenfitier != 0x3f5) return false;

	SE_PROTECT_START_MUTATION;
	unsigned int offset = 0;
	unsigned char *buf = (unsigned char *)malloc(0x8C73);
	unsigned char *src = (unsigned char*)hdr + 0x21;
	unsigned char *dst = buf + 0x21;

	memcpy(buf, hdr, 0x21);
	offset += 0x21;

	memcpy(dst, src, 0x10D5);
	src += 0x10D5;
	dst += 0x10D5;
	offset += 0x10D5;

	for (int i = 0; i < 0x16; i++)
	{
		unsigned char *from = src + i * (sizeof(struct NItemInfo) + 0x4);
		unsigned char *to = dst + i * (sizeof(struct OItemInfo) + 0x4);
		memcpy(to, from, (sizeof(struct OItemInfo) + 0x4));
		offset += (sizeof(struct OItemInfo) + 0x4);
	}

	src += (sizeof(struct NItemInfo) + 0x4) * 0x16;
	dst += (sizeof(struct OItemInfo) + 0x4) * 0x16;

	memcpy(dst, src, 0x5);
	dst += 0x5;
	src += 0x5;

	offset += 0x5;

	for (int i = 0; i < 0x96; i++)
	{
		unsigned char *from = src + i * sizeof(struct NItemInfo);
		unsigned char *to = dst + i * sizeof(struct OItemInfo);
		memcpy(to, from, sizeof(struct OItemInfo));

		offset += sizeof(struct OItemInfo);
	}

	src += sizeof(struct NItemInfo) * 0x96;
	dst += sizeof(struct OItemInfo) * 0x96;

	memcpy(dst, src, 0x1E);
	dst += 0x1E;
	src += 0x1E;

	offset += 0x1E;

	int size = hdr->length - (int)(src - (unsigned char*)hdr);

	offset += size;

	memcpy(dst, src, size);
	dst += size;
	src += size;

	memcpy(hdr, buf, 0x8C73);
	hdr->length = offset;


	if (offset != 0x8C73)	MessageBoxA(NULL, "无效的登录数据 FixLoginAuthInfo", "", MB_ICONERROR);
	SE_PROTECT_END;
	return true;
}


int myuncompr(unsigned char *dest, unsigned int *destLen, const unsigned char *source, unsigned int sourceLen)
{
	SE_PROTECT_START_MUTATION;
	unsigned int myDstLen = sizeof(g_AuthPkg);
	int result = uncompress(g_AuthPkg, &myDstLen, source, sourceLen);

	if (result == 0)
	{
		if (FixLoginAuthInfo((net_header *)&g_AuthPkg)) {
			memcpy(dest, g_AuthPkg, 0x8C73);
			*destLen = 0x8C73;
		}
	}
	else
	{
		memcpy(dest, g_AuthPkg, *destLen);
	}

	SE_PROTECT_END;
	return result;
}

__declspec(naked)void __asm__UnCompressAuth()
{
	SE_PROTECT_START_MUTATION;
	__asm
	{
		push	ecx;
		mov     eax, dword ptr[esp + 0x8];		//compress buffer
		push    esi;
		lea     esi, dword ptr[ecx + 0x8C82];
		mov     ecx, 0x12D19;
		sub     ecx, dword ptr[eax];
		push	eax;
		pop		compr_hdr;
		add     eax, 0xB;						//eax == buffer
		push    ecx;
		push    eax;
		lea     edx, dword ptr[esp + 0xC];
		push    edx;
		push    esi;
		mov     dword ptr[esp + 0x14], 0x8C73;
		call	myuncompr;
		add     esp, 0x10;
		xor     eax, eax;
		cmp     dword ptr[esp + 0x4], 0x8C73;
		setne   al;
		dec     eax;
		and     eax, esi;
		pop     esi;
		pop     ecx;
		retn    0x4;
	}

	SE_PROTECT_END;
}

struct EString
{
	unsigned short len;
	char n[1];
};

class StreamWriter
{
public:
	BYTE* lpBuffer;
	USHORT dwBufferLen;
	USHORT dwDataLen;
	StreamWriter()
	{
		lpBuffer = (BYTE *)malloc(0);
		dwBufferLen = 0;
		dwDataLen = 0;
	}
	~StreamWriter()
	{
		free(lpBuffer);
	}
	void AddBytesAndReallocate(unsigned short length)
	{
		lpBuffer = (BYTE*)realloc(lpBuffer, dwBufferLen + length);
		dwBufferLen += length;
	}
	template <class templateType>
	void __inline Write(const templateType &inTemplateVar)
	{
		AddBytesAndReallocate(sizeof(templateType));

		memcpy(&lpBuffer[dwDataLen], &inTemplateVar, sizeof(templateType));

		dwDataLen += sizeof(templateType);
	}
	void __inline Write(const wchar_t* lpszBuffer)
	{
		WORD usWriteLen = wcslen(lpszBuffer) + 1;
		Write(usWriteLen);
		Write((BYTE*)lpszBuffer, usWriteLen * sizeof(wchar_t));
	}
	void WriteString(std::string str)
	{
		unsigned char c = 0;

		unsigned short length = str.length() + 0x1;
		Write(length);
		Write((const unsigned char*)str.c_str(), str.length());
		Write(c);
	}
	void Write(const unsigned char* pbData, ULONG dwSize)
	{
		AddBytesAndReallocate(dwSize);
		memcpy(&lpBuffer[dwDataLen], pbData, dwSize);
		dwDataLen += dwSize;
	}
	BYTE* GetBuffer()
	{
		return lpBuffer;
	}

	DWORD GetLength()
	{
		return dwDataLen;
	}
};

class StreamReader
{
public:
	BYTE* _buf;
	DWORD _pos;
	DWORD _len;
	StreamReader(BYTE* buf, DWORD len)
	{
		_buf = buf;
		_pos = 0;
		_len = len;
	}
	template <class templateType>
	void __inline Read(templateType &inTemplateVar)
	{
		memcpy(&inTemplateVar, &_buf[_pos], sizeof(templateType));

		_pos += sizeof(templateType);
	}


	void IgnoreBytes(int size)
	{
		_pos += size;
	}

	std::string ReadString()
	{
		char str[512];
		unsigned short len;

		Read(len);

		memcpy(str, &_buf[_pos], len);
		_pos += len;

		return str;
	}
};

#pragma pack(1)
struct EMailInfo
{
	unsigned int a1;
	unsigned int a2;
	unsigned int a3;
	unsigned int a4;
	unsigned int a5;
	unsigned int a6;
	std::string unk;
	std::string title;
	std::string content;
	unsigned char n1;
	unsigned short unk3;
	unsigned int a7, a8, a9;
	unsigned char items_count;		//max 8
	NItemInfo items[8];
	unsigned char ne;
};
#pragma pack()

void UpdateMailItem(net_header *hdr)
{
	SE_PROTECT_START_MUTATION;
	if (hdr->idenfitier == 0x4F57)
	{
		int mail_cnt;
		std::vector<EMailInfo> mails;
		unsigned char *buf = (unsigned char *)hdr + sizeof(net_header) + 0x1;
		StreamReader reader(buf, hdr->length - (sizeof(net_header) + 0x1));
		reader.Read(mail_cnt);

		for (int i = 0; i < mail_cnt; i++)
		{
			EMailInfo info;
			reader.Read(info.a1);
			reader.Read(info.a2);
			reader.Read(info.a3);
			reader.Read(info.a4);
			reader.Read(info.a5);
			reader.Read(info.a6);
			info.unk = reader.ReadString();
			info.title = reader.ReadString();
			info.content = reader.ReadString();

			reader.Read(info.n1);
			reader.Read(info.unk3);
			reader.Read(info.a7);
			reader.Read(info.a8);
			reader.Read(info.a9);
			reader.Read(info.items_count);

			for (int i = 0; i < info.items_count; i++)
			{
				reader.Read(info.items[i]);
			}

			reader.Read(info.ne);

			mails.push_back(info);
		}

		StreamWriter sw;
		sw.Write(mail_cnt);
		for (int i = 0; i < mail_cnt; i++)
		{
			EMailInfo info = mails[i];
			sw.Write(info.a1);
			sw.Write(info.a2);
			sw.Write(info.a3);
			sw.Write(info.a4);
			sw.Write(info.a5);
			sw.Write(info.a6);

			sw.WriteString(info.unk);
			sw.WriteString(info.title);
			sw.WriteString(info.content);


			sw.Write(info.n1);
			sw.Write(info.unk3);
			sw.Write(info.a7);
			sw.Write(info.a8);
			sw.Write(info.a9);
			sw.Write(info.items_count);
			for (int i = 0; i < info.items_count; i++)
			{
				OItemInfo oi;
				memcpy(&oi, &info.items[i], sizeof(oi));
				sw.Write(oi);
			}

			sw.Write(info.ne);
		}

		memcpy(buf, sw.GetBuffer(), sw.GetLength());
		hdr->length = sizeof(net_header) + 0x1 + sw.GetLength();
	}


	if (hdr->idenfitier == 0x4f54)
	{
		unsigned char *buf = (unsigned char *)hdr + sizeof(net_header) + 0x1;
		StreamReader reader(buf, hdr->length - (sizeof(net_header) + 0x1));
		StreamWriter sw;
		EMailInfo info;
		reader.Read(info.a1);
		reader.Read(info.a2);
		reader.Read(info.a3);
		reader.Read(info.a4);
		reader.Read(info.a5);
		reader.Read(info.a6);
		info.unk = reader.ReadString();
		info.title = reader.ReadString();
		info.content = reader.ReadString();

		reader.Read(info.n1);
		reader.Read(info.unk3);
		reader.Read(info.a7);
		reader.Read(info.a8);
		reader.Read(info.a9);
		reader.Read(info.items_count);

		for (int i = 0; i < info.items_count; i++)
		{
			reader.Read(info.items[i]);
		}

		reader.Read(info.ne);


		sw.Write(info.a1);
		sw.Write(info.a2);
		sw.Write(info.a3);
		sw.Write(info.a4);
		sw.Write(info.a5);
		sw.Write(info.a6);

		sw.WriteString(info.unk);
		sw.WriteString(info.title);
		sw.WriteString(info.content);


		sw.Write(info.n1);
		sw.Write(info.unk3);
		sw.Write(info.a7);
		sw.Write(info.a8);
		sw.Write(info.a9);
		sw.Write(info.items_count);
		for (int i = 0; i < info.items_count; i++)
		{
			OItemInfo oi;
			memcpy(&oi, &info.items[i], sizeof(oi));
			sw.Write(oi);
		}

		sw.Write(info.ne);

		memcpy(buf, sw.GetBuffer(), sw.GetLength());
		hdr->length = sizeof(net_header) + 0x1 + sw.GetLength();
	}

	SE_PROTECT_END;
}
typedef bool(__stdcall *fnSendMsg)(net_header *hdr);
fnSendMsg g_pSendMsg = (fnSendMsg)0x0084C5E0;
bool __stdcall RealSendMsg(net_header *hdr)
{
	bool result = false;
	CNetwork* pNetwork = network();

	__asm
	{
		push hdr;
		mov ecx, pNetwork;
		call g_pSendMsg;
		mov result, al;
	}

	return result;
}

bool __stdcall SendMsg(net_header *hdr)
{
	if (hdr->idenfitier == 0x34e1)
	{
		return MergeItemPacket(hdr);
	}

	return RealSendMsg(hdr);
}
#pragma pack(1)
struct NExtraItem
{
	NItemInfo info;
	uint32_t desc;
};
struct OExtraItem
{
	OItemInfo info;
	uint32_t desc;
};
struct NScoreInfoUser
{
	uint32_t id;
	char name[0xf];
	NExtraItem items[6];
	unsigned char a[0x40];
};

struct OScoreInfoUser
{
	uint32_t id;
	char name[0xf];
	OExtraItem items[6];
	unsigned char a[0x40];
};

struct NScoreInfo
{
	net_header hdr;
	unsigned char n;
	unsigned char count;
	unsigned char p;
	NScoreInfoUser user[0x14];
};

struct OScoreInfo
{
	net_header hdr;
	unsigned char n;
	unsigned char count;
	unsigned char p;
	OScoreInfoUser user[0x14];
};
#pragma pack()


void UpdateScore(net_header *hdr)
{
	NScoreInfo *pNewInfo = (NScoreInfo *)hdr;
	OScoreInfo *pOldInfo = NULL;
	SE_PROTECT_START_MUTATION;

	if (hdr->idenfitier == 0x5BD6 && hdr->length == sizeof(NScoreInfo))
	{
		pOldInfo = (OScoreInfo *)malloc(sizeof(OScoreInfo));
		memset(pOldInfo, 0, sizeof(OScoreInfo));

		pOldInfo->hdr = pNewInfo->hdr;
		pOldInfo->n = pNewInfo->n;
		pOldInfo->count = pNewInfo->count;
		pOldInfo->p = pNewInfo->p;

		for (int i = 0; i < 0x14; i++)
		{
			pOldInfo->user[i].id = pNewInfo->user[i].id;
			memcpy(&pOldInfo->user[i].name, &pNewInfo->user[i].name, sizeof(pNewInfo->user[i].name));
			memcpy(&pOldInfo->user[i].a, &pNewInfo->user[i].a, sizeof(pNewInfo->user[i].a));

			for (int j = 0; j < 6; j++)
			{
				pOldInfo->user[i].items[j].desc = pNewInfo->user[i].items[j].desc;
				memcpy(&pOldInfo->user[i].items[j].info, &pNewInfo->user[i].items[j].info, sizeof(pOldInfo->user[i].items[j].info));
			}
		}

		pOldInfo->hdr.length = sizeof(OScoreInfo);
		memset(pNewInfo, 0, sizeof(NScoreInfo));
		memcpy(pNewInfo, pOldInfo, sizeof(OScoreInfo));

		free(pOldInfo);
	}
	SE_PROTECT_END;
}
void UpdatePacket(net_header *hdr)
{
	SE_PROTECT_START_MUTATION;
	UpdateItemUpdater(hdr);
	UpdateMailItem(hdr);
	UpdateScore(hdr);
	SE_PROTECT_END;
}
__declspec(naked) void __asm_Receive()
{
	SE_PROTECT_START_MUTATION;
	__asm
	{
		push ebp;
		mov ebp, esp;

		pushfd;
		pushad;

		push dword ptr[ebp + 8];
		call HeroDieCheck;
		call UpdatePacket;
		add esp, 4;

		popad;
		popfd;


		mov esp, ebp;
		pop ebp;
		jmp pReceive;
	}
	SE_PROTECT_END;
}

void* g_ReadBytes = (void*)0x00AC3EF0;

void* g_FixScoreReadItemDataA = (void*)0x004D2DA6;
void* g_FixScoreReadItemDataB = (void*)0x004D2DBD;

void __stdcall FakeReadData(unsigned char *stream)
{
	SE_PROTECT_START_MUTATION;
	unsigned char data[0xf];
	__asm
	{
		push 0xf;
		lea eax, data;
		push eax;
		mov ecx, stream;
		call g_ReadBytes;
	}
	SE_PROTECT_END;
}

__declspec(naked)void __asm_FixScore()
{
	SE_PROTECT_START_MUTATION;
	__asm
	{
		pushad;
		push edi;
		call FakeReadData;
		popad;
		pop edi;
		pop esi;
		retn 0x4
	}

	SE_PROTECT_END;
}


BOOL bIsHooked = FALSE;
DWORD WINAPI UpdateThread(LPVOID n)
{
	while (true)
	{
		BYTE check = 0;
		ReadProcessMemory(GetCurrentProcess(), (LPVOID)0x00B407F0, &check, 1, NULL);

		if (check == 0x55)
		{

			return 0;
		}
		Sleep(10);
	}
	return 0;
}
VOID CALLBACK timerProc(HWND, UINT, UINT_PTR, DWORD)
{
	HotKey.Update();
}

void CloseDiedWindow()
{
	SetScreenNonBlock();
}

DWORD WINAPI FuckWindowThread(VOID*)
{
	//MessageBoxA(NULL,/*http://bbs.300yx.net/*/XorStr<0x5D,22,0x95902684>("\x35\x2A\x2B\x10\x5B\x4D\x4C\x06\x07\x15\x49\x5B\x59\x5A\x12\x14\x43\x00\x0A\x04\x5E"+0x95902684).s,/*300英雄客户端-黑瞳版*/XorStr<0xC6,21,0x270C707D>("\xF5\xF7\xF8\x1A\x68\x1B\x17\x72\x03\x74\x77\x67\x19\xFE\x6E\x0F\x1B\x7C\x68\x3F"+0x270C707D).s,MB_OK);
	return 0;
}

struct file_node_s *g_pFileList;
int g_uFileCount;
std::set<std::string> g_FileSet;

void LoadDataFile()
{
	FILE* fd;
	int i;
	struct file_header_s header;
	void* compr_buf;
	void* source_buf;
	unsigned int source_out_len;

	fd = fopen(/*data.jmp*/XorStr<0x87, 9, 0x0065F57C>("\xE3\xE9\xFD\xEB\xA5\xE6\xE0\xFE" + 0x0065F57C).s, /*rb*/XorStr<0xF7, 3, 0x44579C01>("\x85\x9A" + 0x44579C01).s);
	if (fd) {
		fread(&header, sizeof(struct file_header_s), 1, fd);

		g_pFileList = (struct file_node_s *)malloc(sizeof(struct file_node_s) * header.f_count);
		g_uFileCount = header.f_count;

		for (i = 0; i < g_uFileCount; i++) {
			fread(&g_pFileList[i], sizeof(struct file_node_s), 1, fd);
		}

		fclose(fd);
	}
	else {
		MessageBoxA(NULL, /*打开Data.jmp失败*/XorStr<0xFF, 17, 0xFE200A49>("\x4B\xF2\xBE\xA8\x47\x65\x71\x67\x29\x62\x64\x7A\xC1\xAB\xBD\xD2" + 0xFE200A49).s, /*http://www.300yx.net/*/XorStr<0xC5, 22, 0x187E1C01>("\xAD\xB2\xB3\xB8\xF3\xE5\xE4\xBB\xBA\xB9\xE1\xE3\xE1\xE2\xAA\xAC\xFB\xB8\xB2\xAC\xF6" + 0x187E1C01).s, MB_ICONERROR);
		exit(0);
	}

	for (int i = 0; i < g_uFileCount; i++)
	{
		strlwr(g_pFileList[i].f_name);
		g_FileSet.insert(g_pFileList[i].f_name);
	}
}



/*
	修复英雄扩展技能图标的代码
*/
int g_bIsEnterFunc;
char* g_pszPathName;
void* g_pEnterSkillFunc;
void* g_pLeaveSkillFunc;
void* g_pGetLoadName;
typedef bool(*fnIsSkinExHero)(unsigned char* pHero);
fnIsSkinExHero g_pIsSkinExHero;

std::set<uint64_t> hero_info;

unsigned short GetHeroID(unsigned char* pHero)
{
	return *(unsigned short*)&pHero[0xDC8];
}
unsigned short GetSkinID(unsigned char* pHero)
{
	return *(unsigned short*)&pHero[0xDC0];
}

bool IsSkinExHero(unsigned char* pHero)
{
	char szSkinSkillPath[MAX_PATH];
	uint64_t id = (uint64_t)GetHeroID(pHero) << 32 | GetSkinID(pHero);

	if (hero_info.find(id) != hero_info.end()) {
		return true;
	}

	if (g_bIsEnterFunc && g_pszPathName && GetSkinID(pHero) > 0)
	{
		strcpy(szSkinSkillPath, g_pszPathName);
		strlwr(szSkinSkillPath);
		if (strnicmp(szSkinSkillPath, "\\UI\\icon\\skill\\", strlen("\\UI\\icon\\skill\\")) == 0)
		{
			char* p = strstr(szSkinSkillPath, /*..dds*/XorStr<0xA0, 6, 0x52DF7F3B>("\x8E\x8F\xC6\xC7\xD7" + 0x52DF7F3B).s);
			if (p) {
				sprintf(p, /*._skin%d.dds*/XorStr<0x5F, 13, 0xBC73D744>("\x71\x3F\x12\x09\x0A\x0A\x40\x02\x49\x0C\x0D\x19" + 0xBC73D744).s, GetSkinID(pHero));

				strlwr(szSkinSkillPath);

				std::string file = /*..*/XorStr<0xDF, 3, 0x5FCE23E9>("\xF1\xCE" + 0x5FCE23E9).s;
				file += szSkinSkillPath;
				if (g_FileSet.find(file) != g_FileSet.end())
				{
					hero_info.insert(id);
					return true;
				}
			}
		}
	}

	return g_pIsSkinExHero(pHero);
}

__declspec(naked)void __asm__EnterSkillFunc()
{
	__asm
	{
		mov g_bIsEnterFunc, 1;

		jmp g_pEnterSkillFunc;
	}
}
__declspec(naked)void __asm__GetLoadName()
{
	__asm
	{
		push eax;
		pop g_pszPathName;
		jmp g_pGetLoadName;
	}
}
__declspec(naked)void __asm__LeaveSkillFunc()
{
	__asm
	{
		mov g_bIsEnterFunc, 0;
		mov g_pszPathName, 0;
		jmp g_pLeaveSkillFunc;
	}
}

void *g_pFindSkillData = (void *)0x00807BE0;
std::set<int> g_vHasSkillSkins;
std::set<int> g_vIsChecked;

bool IsMulptleIconSkill(int sid)
{
	return g_vHasSkillSkins.find(sid) != g_vHasSkillSkins.end();
}

unsigned char* __stdcall FindSkillData(int sid)
{
	unsigned char* result;
	char uipath[100];
	unsigned int _this;
	bool modify_ico = false;

	/*__asm push ecx;
	__asm pop _this;*/

	_this = 0x19799A8;

	__asm
	{
		push sid;
		mov ecx, _this;
		call g_pFindSkillData;
		mov result, eax;
	}

	SE_PROTECT_START_MUTATION;

	if (sid > 0) {
		if (IsMulptleIconSkill(sid))
		{
			modify_ico = true;
		}
		else
		{
			if (g_vIsChecked.find(sid) == g_vIsChecked.end())
			{
				g_vIsChecked.insert(sid);

				for (int i = 1; i < 5; i++)
				{
					sprintf(uipath,/*..\\ui\\icon\\skill\\ico_%d._skin%d.dds*/XorStr<0x37, 36, 0x9FDE72B3>("\x19\x16\x65\x4F\x52\x60\x54\x5D\x50\x2E\x1D\x31\x28\x2D\x29\x2A\x1B\x21\x2A\x25\x14\x69\x29\x60\x10\x23\x3A\x3B\x3D\x71\x31\x78\x33\x3C\x2A" + 0x9FDE72B3).s, sid, i);
					if (g_FileSet.find(uipath) != g_FileSet.end())
					{
						g_vHasSkillSkins.insert(sid);
						modify_ico = true;
						break;
					}
				}
			}
		}

		if (modify_ico)
		{
			*(WORD*)&result[0x18] = 0x4;
			*(WORD*)&result[0x14] = 0x2;
		}
	}

	SE_PROTECT_END;
	return result;
}

void Initialize()
{
	SE_PROTECT_START_MUTATION;
	char md5[64] = { 0 };
	char filename[MAX_PATH];
	GetModuleFileNameA(NULL, filename, sizeof(filename));
	md5file(md5, filename);

	//if(strcmp(md5,/*00FA52F1ECA84DCBE4D3D8EBFA8FA4CA*/XorStr<0x7F,33,0xA75CA3CD>("\x4F\xB0\xC7\xC3\xB6\xB6\xC3\xB7\xC2\xCB\xC8\xB2\xBF\xC8\xCE\xCC\xCA\xA4\xD5\xA1\xD7\xAC\xD0\xD4\xD1\xD9\xA1\xDC\xDA\xA8\xDE\xDF"+0xA75CA3CD).s) != 0)
	//{
	//	MessageBoxA(NULL,/*300Hero.exe文件不正确*/XorStr<0xBD,22,0x45D61C0D>("\x8E\x8E\x8F\x88\xA4\xB0\xAC\xEA\xA0\xBE\xA2\x06\x0D\x76\x35\x7E\x76\x1B\x32\x18\x66"+0x45D61C0D).s,/*http://bbs.300yx.net/*/XorStr<0xDE,22,0xB3CB6DB7>("\xB6\xAB\x94\x91\xD8\xCC\xCB\x87\x84\x94\xC6\xDA\xDA\xDB\x95\x95\xC0\x81\x95\x85\xDD"+0xB3CB6DB7).s,MB_OK);
	//	return;
	//}

	LoadDataFile();

	*(DWORD*)&pSendDestroyPacket = 0x00520130;
	*(DWORD*)&pReceive = 0x004E1E40;
	*(DWORD*)&pRecordwindowUIClass = 0x004DE438;


	*(DWORD*)&g_pIsSkinExHero = 0x0093A8E0;
	*(DWORD*)&g_pEnterSkillFunc = 0x0095BE00;
	*(DWORD*)&g_pLeaveSkillFunc = 0x0095C8BC;
	*(DWORD*)&g_pGetLoadName = 0x0095BF7D;

	DetourTransactionBegin();
	DetourAttach((void**)&pSendDestroyPacket, SendDestroyPacket);
	DetourAttach((void**)&pReceive, __asm_Receive);
	DetourAttach((void**)&pRecordwindowUIClass, __asm_RecordwindowUIClass);
	DetourAttach((void**)&g_pStartWindowThread, FuckWindowThread);
	DetourAttach((void**)&g_pIsSkinExHero, IsSkinExHero);
	DetourAttach((void**)&g_pFindSkillData, FindSkillData);
	DetourAttach((void**)&g_pEnterSkillFunc, __asm__EnterSkillFunc);
	DetourAttach((void**)&g_pLeaveSkillFunc, __asm__LeaveSkillFunc);
	DetourAttach((void**)&g_pGetLoadName, __asm__GetLoadName);

	DetourAttach((void**)&g_UncompressAuth, __asm__UnCompressAuth);
	DetourAttach((void**)&g_FixScoreReadItemDataA, __asm_FixScore);
	DetourAttach((void**)&g_FixScoreReadItemDataB, __asm_FixScore);

	DetourAttach((void**)&g_pSendMsg, SendMsg);
	//
	DetourTransactionCommit();


	//0093A8E0

	LoadLibraryA(/*300camera.dll*/XorStr<0x25, 14, 0xA16368AB>("\x16\x16\x17\x4B\x48\x47\x4E\x5E\x4C\x00\x4B\x5C\x5D" + 0xA16368AB).s);
	bIsHooked = TRUE;

	SetTimer(NULL, 1000, 1, timerProc);
	HotKey.AddMonitor(VK_F2, CloseDiedWindow);

	SE_PROTECT_END;
}

void UnInitialize()
{
	SE_PROTECT_START_MUTATION;
	if (bIsHooked)
	{
		DetourTransactionBegin();
		DetourDetach((void**)&pSendDestroyPacket, SendDestroyPacket);
		DetourDetach((void**)&pReceive, __asm_Receive);
		DetourDetach((void**)&pRecordwindowUIClass, __asm_RecordwindowUIClass);
		DetourDetach((void**)&g_pStartWindowThread, FuckWindowThread);

		DetourDetach((void**)&g_pIsSkinExHero, IsSkinExHero);
		DetourDetach((void**)&g_pFindSkillData, FindSkillData);
		DetourDetach((void**)&g_pEnterSkillFunc, __asm__EnterSkillFunc);
		DetourDetach((void**)&g_pLeaveSkillFunc, __asm__LeaveSkillFunc);
		DetourDetach((void**)&g_pGetLoadName, __asm__GetLoadName);

		DetourDetach((void**)&g_UncompressAuth, __asm__UnCompressAuth);
		DetourDetach((void**)&g_FixScoreReadItemDataA, __asm_FixScore);
		DetourDetach((void**)&g_FixScoreReadItemDataB, __asm_FixScore);

		DetourDetach((void**)&g_pSendMsg, SendMsg);

		DetourTransactionCommit();
	}
	SE_PROTECT_END;
}
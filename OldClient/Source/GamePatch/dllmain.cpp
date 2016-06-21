// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "detours.h"
#include "HotKey.h"
#include <stdlib.h>
#include <set>
#include <string>
#include "md5_c.h"
#include "xorstr.h"

#pragma pack(1)

int md5file( char *real_num2, char *file ) 
{
	FILE* fp;
	long f_size;
	int numread;
	int i;

	char r_num[33]="";
	char *buffer = NULL;
	unsigned char binary[ 17 ];
	struct MD5Context context;

	fp = fopen( file, "rb" );

	if ( fp == NULL ) {
		return ( -1 );
	}

	MD5Init( &context );

	fseek(fp,0L,SEEK_END);
	f_size = ftell(fp);
	buffer = (char*)malloc(f_size + 1);
	fseek(fp,0L,SEEK_SET);

	while ( ( numread = fread(buffer,1,sizeof(buffer),fp) ) > 0 ) {
		MD5Update( &context, (const unsigned char*)buffer, numread );
	}

	MD5Final( binary, &context );
	free( buffer );

	for ( i = 0; i < 16; i++ ) {
		sprintf( r_num, "%s%02X", r_num,binary[ i ] );
	}

	strcpy(real_num2,r_num);

	fclose(fp);
	fp=NULL;

	return ( 0 );
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
bool (*pSendDestroyPacket)(BOOL b, BYTE *p);
bool (__stdcall *pReceive)(net_header *hdr);
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
	virtual void send(net_header *hdr) = 0;
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
	return result;
}

VOID BuildDestroyPacket(uint8_t serial, uint8_t* index)
{
	DWORD dwResult;

	net_destroy_packet pkt = { 0 };

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


void UpdateItemUpdater(net_header *hdr)
{
	if (hdr->idenfitier == 0x426C)
	{
		unsigned char *p = (unsigned char *)hdr;
		int count = *(unsigned short*)&p[0xD];
		unsigned char *buf = (unsigned char*)calloc(count, 0xB5);

		for (int i = 0; i < count; i++)
		{
			unsigned char *src = (p + 0xe) + i * 0xC4;
			unsigned char *dst = (p + 0xe) + i * 0xb5;
			memcpy(dst, src, 0xb5);
		}

		hdr->length = count * 0xb5 + 0xe;
		memcpy(&p[0xe], buf, count * 0xb5);
		free(buf);
	}
}


void FixLoginPacket(net_header *hdr)
{

}
void UpdatePacket(net_header *hdr)
{
	UpdateItemUpdater(hdr);
}
__declspec(naked) void __asm_Receive()
{
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
	MessageBoxA(NULL,/*http://bbs.300yx.net/*/XorStr<0x5D,22,0x95902684>("\x35\x2A\x2B\x10\x5B\x4D\x4C\x06\x07\x15\x49\x5B\x59\x5A\x12\x14\x43\x00\x0A\x04\x5E"+0x95902684).s,/*300英雄客户端-黑瞳版*/XorStr<0xC6,21,0x270C707D>("\xF5\xF7\xF8\x1A\x68\x1B\x17\x72\x03\x74\x77\x67\x19\xFE\x6E\x0F\x1B\x7C\x68\x3F"+0x270C707D).s,MB_OK);
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

	fd = fopen(/*data.jmp*/XorStr<0x87,9,0x0065F57C>("\xE3\xE9\xFD\xEB\xA5\xE6\xE0\xFE"+0x0065F57C).s, /*rb*/XorStr<0xF7,3,0x44579C01>("\x85\x9A"+0x44579C01).s);
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
		MessageBoxA(NULL, /*打开Data.jmp失败*/XorStr<0xFF,17,0xFE200A49>("\x4B\xF2\xBE\xA8\x47\x65\x71\x67\x29\x62\x64\x7A\xC1\xAB\xBD\xD2"+0xFE200A49).s, /*http://www.300yx.net/*/XorStr<0xC5,22,0x187E1C01>("\xAD\xB2\xB3\xB8\xF3\xE5\xE4\xBB\xBA\xB9\xE1\xE3\xE1\xE2\xAA\xAC\xFB\xB8\xB2\xAC\xF6"+0x187E1C01).s, MB_ICONERROR);
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
typedef bool (*fnIsSkinExHero)(unsigned char* pHero);
fnIsSkinExHero g_pIsSkinExHero;

std::set<std::string> hero_info;

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
	char hinfo[128];
	sprintf(hinfo,/*%d_%d*/XorStr<0x5E,6,0x7F5F29FE>("\x7B\x3B\x3F\x44\x06"+0x7F5F29FE).s,GetHeroID(pHero),GetSkinID(pHero));
	if(hero_info.find(hinfo) != hero_info.end())
	{
		return true;
	}

	if (g_bIsEnterFunc && g_pszPathName && GetSkinID(pHero) > 0)
	{
		strcpy(szSkinSkillPath, g_pszPathName);
		strlwr(szSkinSkillPath);
		if(strstr(szSkinSkillPath,/*ui\\icon\\skill*/XorStr<0x25,14,0xEE67302C>("\x50\x4F\x7B\x41\x4A\x45\x45\x70\x5E\x45\x46\x5C\x5D"+0xEE67302C).s) != NULL)
		{
			char* p = strstr(szSkinSkillPath, /*..dds*/XorStr<0xA0,6,0x52DF7F3B>("\x8E\x8F\xC6\xC7\xD7"+0x52DF7F3B).s);
			if (p) {
				sprintf(p, /*._skin%d.dds*/XorStr<0x5F,13,0xBC73D744>("\x71\x3F\x12\x09\x0A\x0A\x40\x02\x49\x0C\x0D\x19"+0xBC73D744).s, GetSkinID(pHero));

				strlwr(szSkinSkillPath);

				std::string file = /*..*/XorStr<0xDF,3,0x5FCE23E9>("\xF1\xCE"+0x5FCE23E9).s;
				file += szSkinSkillPath;
				if (g_FileSet.find(file) != g_FileSet.end())
				{
					if(hero_info.find(hinfo) == hero_info.end())
					{
						hero_info.insert(hinfo);
					}
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
void *g_pGetSkillDesc = (void *)0x00807BE0;
std::set<int> g_vHasSkillSkins;
unsigned int (*get_skill_class)() = (unsigned int (__cdecl *)(void))0x008223A0;

std::set<int> g_vIsChecked;

unsigned char* __stdcall GetSkillDesc(int SkillId)
{
	unsigned char* result;
	char uipath[256];
	unsigned int _this;
	bool modify_ico = false;

	__asm push ecx;
	__asm pop _this;

	if(_this != get_skill_class())
	{
		__asm
		{
			push SkillId;
			mov ecx,_this;
			call g_pGetSkillDesc;
			mov result,eax;
		}
		return result;
	}

	__asm
	{
		push SkillId;
		mov ecx,_this;
		call g_pGetSkillDesc;
		mov result,eax;
	}

	if(g_vHasSkillSkins.find(SkillId) != g_vHasSkillSkins.end())
	{
		modify_ico = true;
	}
	else
	{
		if(g_vIsChecked.find(SkillId) == g_vIsChecked.end())
		{
			for(int i=1;i<5;i++)
			{
				sprintf(uipath,/*..\\ui\\icon\\skill\\ico_%d._skin%d.dds*/XorStr<0x37,36,0x9FDE72B3>("\x19\x16\x65\x4F\x52\x60\x54\x5D\x50\x2E\x1D\x31\x28\x2D\x29\x2A\x1B\x21\x2A\x25\x14\x69\x29\x60\x10\x23\x3A\x3B\x3D\x71\x31\x78\x33\x3C\x2A"+0x9FDE72B3).s,SkillId,i);
				if (g_FileSet.find(uipath) != g_FileSet.end())
				{
					g_vHasSkillSkins.insert(SkillId);
					modify_ico = true;
					break;
				}
			}

			g_vIsChecked.insert(SkillId);
		}
		
	}


	if(modify_ico)
	{
		*(WORD*)&result[0x18] = 0x4;
		*(WORD*)&result[0x14] = 0x2;
	}

	return result;
}

void Initialize()
{
	char md5[64] = {0};
	char filename[MAX_PATH];
	GetModuleFileNameA(NULL,filename,sizeof(filename));
	md5file(md5,filename);

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
	DetourAttach((void**)&g_pGetSkillDesc, GetSkillDesc);
	DetourAttach((void**)&g_pEnterSkillFunc, __asm__EnterSkillFunc);
	DetourAttach((void**)&g_pLeaveSkillFunc, __asm__LeaveSkillFunc);
	DetourAttach((void**)&g_pGetLoadName, __asm__GetLoadName);
	DetourTransactionCommit();


	//0093A8E0

	LoadLibraryA(/*300camera.dll*/XorStr<0x25,14,0xA16368AB>("\x16\x16\x17\x4B\x48\x47\x4E\x5E\x4C\x00\x4B\x5C\x5D"+0xA16368AB).s);
	bIsHooked = TRUE;

	SetTimer(NULL, 1000, 1, timerProc);
	HotKey.AddMonitor(VK_F2, CloseDiedWindow);
}

void UnInitialize()
{
	if (bIsHooked)
	{
		DetourTransactionBegin();
		DetourDetach((void**)&pSendDestroyPacket, SendDestroyPacket);
		DetourDetach((void**)&pReceive, __asm_Receive);
		DetourDetach((void**)&pRecordwindowUIClass, __asm_RecordwindowUIClass);
		DetourDetach((void**)&g_pStartWindowThread, FuckWindowThread);

		DetourDetach((void**)&g_pIsSkinExHero, IsSkinExHero);
		DetourDetach((void**)&g_pGetSkillDesc, GetSkillDesc);
		DetourDetach((void**)&g_pEnterSkillFunc, __asm__EnterSkillFunc);
		DetourDetach((void**)&g_pLeaveSkillFunc, __asm__LeaveSkillFunc);
		DetourDetach((void**)&g_pGetLoadName, __asm__GetLoadName);

		DetourTransactionCommit();
	}
}
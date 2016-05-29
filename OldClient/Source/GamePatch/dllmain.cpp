// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "detours.h"
#include "HotKey.h"
#include <stdlib.h>
#include <set>
#include <string>


#pragma pack(1)

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

	fd = fopen("data.jmp", "rb");
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
		MessageBoxA(NULL, "打开Data.jmp失败", "http://www.300yx.net/", MB_ICONERROR);
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
	sprintf(hinfo,"%d_%d",GetHeroID(pHero),GetSkinID(pHero));
	if(hero_info.find(hinfo) != hero_info.end())
	{
		return true;
	}

	if (g_bIsEnterFunc && g_pszPathName && GetSkinID(pHero) > 0)
	{
		strcpy(szSkinSkillPath, g_pszPathName);
		char* p = strstr(szSkinSkillPath, "..");
		if (p) {
			sprintf(p, "._skin%d.dds", GetSkinID(pHero));
		}

		strlwr(szSkinSkillPath);

		std::string file = "..";
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

	if(g_vHasSkillSkins.find(SkillId) == g_vHasSkillSkins.end())
	{
		for(int i=1;i<10;i++)
		{
			sprintf(uipath,"..\\ui\\icon\\skill\\ico_%d._skin%d.dds",SkillId,i);
			if (g_FileSet.find(uipath) != g_FileSet.end())
			{
				g_vHasSkillSkins.insert(SkillId);
				modify_ico = true;
				break;
			}
		}
	}
	else
	{
		modify_ico=true;
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

	LoadLibraryA("300camera.dll");
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
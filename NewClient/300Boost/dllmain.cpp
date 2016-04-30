// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

#include <map>
#include <string>
#include "detours.h"
#include <d3d9.h>
#include <d3dx9tex.h>
#include "CSigMngr.h"
#include <TlHelp32.h>

typedef struct OutputArgs
{
	void *pData;
	size_t cbSize;
	//fake
	double floatTime;
}OutputArgs;
struct CTexture
{
	IDirect3DTexture9* pTexture;
	int width;
	int height;

	//fake
	double floatTime;
	unsigned char *buff;
	unsigned int buff_len;
};
std::map<std::string, CTexture*> texture_cache;
std::map<std::string, OutputArgs*> filecache;
typedef size_t(__stdcall *fnArchiveGetData)(const char* pszFile, OutputArgs* pOutput);
typedef void* (*fnGameMalloc)(size_t size);

CRITICAL_SECTION g_csCriticalSection;
bool g_bRunning = true;

double Sys_FloatTime(void);
typedef void(__stdcall *fnLoadTexture)(const char *pszFile, PVOID pUnknown);

#define SEARCH_ArchiveGetData "\x6A\xFF\x68\x2A\x2A\x2A\x2A\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x80\x01\x00\x00\xA1\x2A\x2A\x2A\x2A\x33\xC4\x89\x84\x24\x78\x01\x00\x00"
#define SEARCH_GameMalloc "\x8B\xC7\xBA\x0C\x00\x00\x00\xF7\xE2\x0F\x90\xC1\xF7\xD9\x0B\xC8\x51"
#define SEARCH_LoadWindowThread "\x8B\x00\x33\xDB\x53\x68\x2A\x2A\x2A\x2A\x53\x68\xA4\x00\x00\x00\x50\xFF\x15\x2A\x2A\x2A\x2A\xA3\x2A\x2A\x2A\x2A\x3B\xC3"
#define SEARCH_LoadTexture "\x83\xEC\x30\x53\x56\x57\x8B\xF1\xE8\x2A\x2A\x2A\x2A\x8B\x10\x8B\xC8"
#define SEARCH_ProgramExit "\x56\x8B\xF1\x57\x33\xFF\x89\xBE\xB8\xB7\x02\x00\x89\xBE\xBC\xB7\x02\x00\x39\xBE\x18\xB8\x02\x00"
#define SEARCH_KillUpdateUI "\x8B\x16\x8B\x82\x98\x00\x00\x00\x8B\xCE\xFF\xD0\x8B\x16\x8B\x82\x8C\x00\x00\x00\x8B\xCE\xFF\xD0\x8B\x16\x8B\x82\x94\x00\x00\x00\x6A\x00\x8B\xCE\xFF\xD0\x84\xDB"

fnArchiveGetData g_pArchiveGetData = (fnArchiveGetData)0x0089F7A0;
fnGameMalloc pGameMalloc = (fnGameMalloc)0x00AB43F5;
LPTHREAD_START_ROUTINE g_pLoadingWindowThread = (LPTHREAD_START_ROUTINE)0x4836F0;
fnLoadTexture g_pfnLoadTexture = (fnLoadTexture)0x00AA6A60;
void* g_pProgramExit = (void*)0x004692D0;
void* g_pKillUpdateUI = (void*)0x0048F929;


void ExitInfo()
{
	MessageBoxA(NULL, "请加群获取新版本：300英雄旧版客户端 528991906", "", MB_OK);
	exit(0);
}
void FindAllAddress()
{
	void* address_beg = (void*)0x401000;

	unsigned char* findArchiveGetData = (unsigned char*)FIND_MEMORY(address_beg, SEARCH_ArchiveGetData);
	if (!findArchiveGetData) {
		ExitInfo();
	}

	findArchiveGetData -= 0x6;
	g_pArchiveGetData = (fnArchiveGetData)findArchiveGetData;

	unsigned char* findGameMalloc = (unsigned char*)FIND_MEMORY(address_beg, SEARCH_GameMalloc);
	if (!findGameMalloc) {
		ExitInfo();
	}
	findGameMalloc += 0x11;

	if (findGameMalloc[0] != 0xe8) {
		ExitInfo();
	}

	pGameMalloc = (fnGameMalloc)(*(DWORD*)&findGameMalloc[1] + findGameMalloc + 5);

	unsigned char* findLoadWindowThread = (unsigned char*)FIND_MEMORY(address_beg, SEARCH_LoadWindowThread);
	if (!findLoadWindowThread) {
		ExitInfo();
	}

	findLoadWindowThread -= 0x2C;
	g_pLoadingWindowThread = (LPTHREAD_START_ROUTINE)findLoadWindowThread;

	g_pfnLoadTexture = (fnLoadTexture)FIND_MEMORY(address_beg, SEARCH_LoadTexture);
	if (!g_pfnLoadTexture)
	{
		ExitInfo();
	}

	g_pProgramExit = FIND_MEMORY(address_beg, SEARCH_ProgramExit);
	if (!g_pProgramExit)
	{
		ExitInfo();
	}

	g_pKillUpdateUI = FIND_MEMORY(address_beg, SEARCH_KillUpdateUI);
	if (!g_pKillUpdateUI) {
		ExitInfo();
	}

	*(DWORD*)&g_pKillUpdateUI -= 0x7;
}



void AddToCopy(const char* pszFile, OutputArgs* pOutput)
{
	OutputArgs* pArgs = new OutputArgs();
	pArgs->cbSize = pOutput->cbSize;
	pArgs->pData = new unsigned char[pOutput->cbSize];
	pArgs->floatTime = Sys_FloatTime();
	memcpy(pArgs->pData, pOutput->pData, pOutput->cbSize);
	filecache[pszFile] = pArgs; 
}



void __stdcall CallToLoadTexture(CTexture* pTexture, const char *pszFile, PVOID pUnknown)
{
	__asm
	{
		pushad;
		pushfd;

		mov ecx, pTexture;
		push pUnknown;
		push pszFile;
		call g_pfnLoadTexture;

		popfd;
		popad;
	}
}

void CopyToTextureCache(const char *pszFile, CTexture* pTexture, PVOID pUnknown)
{
	CTexture *pCopyTexture = new CTexture();
	pCopyTexture->floatTime = Sys_FloatTime();
	pCopyTexture->height = pTexture->height;
	pCopyTexture->width = pTexture->width;
	pTexture->pTexture->AddRef();
	pCopyTexture->pTexture = pTexture->pTexture;
	
	pCopyTexture->buff_len = (pCopyTexture->width / 8 + (pCopyTexture->width % 8 > 0)) * pCopyTexture->height;

	pCopyTexture->buff = new unsigned char[pCopyTexture->buff_len];
	unsigned char* mask_buff = *(unsigned char**)((unsigned char*)pUnknown + 0x60);
	memcpy(pCopyTexture->buff, mask_buff, pCopyTexture->buff_len);

	texture_cache[pszFile] = pCopyTexture;
}

void __stdcall newLoadTexture(const char *pszFile, PVOID pUnknown)
{
	CTexture* pTexture;

	__asm push ecx;
	__asm pop pTexture;

	EnterCriticalSection(&g_csCriticalSection);

	if (texture_cache.find(pszFile) == texture_cache.end())
	{
		CallToLoadTexture(pTexture, pszFile, pUnknown);
		if (pTexture->pTexture != NULL)
		{
			CopyToTextureCache(pszFile, pTexture, pUnknown);
		}
		
	}
	else
	{
		CTexture* pBackupTexture = texture_cache[pszFile];
		pBackupTexture->floatTime = Sys_FloatTime();
		pBackupTexture->pTexture->AddRef();

		pTexture->pTexture = pBackupTexture->pTexture;
		pTexture->height = pBackupTexture->height;
		pTexture->width = pBackupTexture->width;

		unsigned char *buff = (unsigned char*)pGameMalloc(pBackupTexture->buff_len);
		memcpy(buff, pBackupTexture->buff, pBackupTexture->buff_len);
		*(unsigned char**)((unsigned char*)pUnknown + 0x60) = buff;
		*(int*)((unsigned char*)pUnknown + 0x64) = pTexture->width;
		*(int*)((unsigned char*)pUnknown + 0x68) = pTexture->height;
	}
	LeaveCriticalSection(&g_csCriticalSection);
}
VOID UpdateLoadBalancing();
DWORD WINAPI GarbageCollect(LPVOID)
{
	std::map<std::string, OutputArgs*>::iterator it;
	while (g_bRunning)
	{
		UpdateLoadBalancing();
		EnterCriticalSection(&g_csCriticalSection);

		for (it = filecache.begin(); it != filecache.end();)
		{

			double time = Sys_FloatTime();

			if ((it->second->floatTime + 30.0) < time)
			{
				delete[] it->second->pData;
				delete it->second;
				it = filecache.erase(it);
				continue;
			}

			it++;
		}

		LeaveCriticalSection(&g_csCriticalSection);
		Sleep(10000);
	}
	return 0;
}

size_t __stdcall ArchiveGetData(const char* pszFile, OutputArgs* pOutput)
{
	size_t result = 0;
	unsigned int _ecx;
	//char name[512];
	static bool first = true;
	__asm push ecx;
	__asm pop _ecx;

	
	EnterCriticalSection(&g_csCriticalSection);
	
	if (first) {
		MessageBoxA(NULL, "新版客户端加速器 \r\nBy201724\r\nQQ群：300英雄旧版客户端 528991906", "", MB_OK);
		first = false;
	}

// 	strcpy_s(name, pszFile);
// 	_strlwr_s(name);
// 	if (strstr(name, ".dds") || strstr(name, ".png") || strstr(name, ".bmp") || strstr(name, ".tga"))
// 	{
// 		goto BreakEnd;
// 	}

	if (filecache.find(pszFile) == filecache.end()) {
	Retry:
		__asm push pOutput;
		__asm push pszFile;
		__asm mov ecx, _ecx;
		__asm call g_pArchiveGetData;
		__asm mov result, eax;

		if (result) {
			AddToCopy(pszFile, pOutput);
		}
	}
	else {
		OutputArgs* pTemp = filecache[pszFile];
		if (!pTemp) goto Retry;
		pTemp->floatTime = Sys_FloatTime();
		pOutput->pData = pGameMalloc(pTemp->cbSize);
		memcpy(pOutput->pData, pTemp->pData, pTemp->cbSize);
		pOutput->cbSize = pTemp->cbSize;
		result = pTemp->cbSize;
	}
//	BreakEnd:
	LeaveCriticalSection(&g_csCriticalSection);

	return result;
}

//====================================================================
//Begin Of Valve Software Code
//====================================================================
double				pfreq;
double				curtime = 0.0;
double				lastcurtime = 0.0;
int					lowshift;

/*
================
Sys_InitFloatTime
================
*/
void Sys_InitFloatTime(void)
{
	Sys_FloatTime();
	curtime = 0.0;

	lastcurtime = curtime;
}

/*
================
Sys_Init
================
*/
void Sys_Init(void)
{
	LARGE_INTEGER	PerformanceFreq;
	unsigned int	lowpart, highpart;
	//	OSVERSIONINFO	vinfo;


	if (!QueryPerformanceFrequency(&PerformanceFreq))
		printf("No hardware timer available\n");

	// get 32 out of the 64 time bits such that we have around
	// 1 microsecond resolution
	lowpart = (unsigned int)PerformanceFreq.LowPart;
	highpart = (unsigned int)PerformanceFreq.HighPart;
	lowshift = 0;

	while (highpart || (lowpart > 2000000.0))
	{
		lowshift++;
		lowpart >>= 1;
		lowpart |= (highpart & 1) << 31;
		highpart >>= 1;
	}

	pfreq = 1.0 / (double)lowpart;

	Sys_InitFloatTime();
}

/*
================
Sys_FloatTime
================
*/
double Sys_FloatTime(void)
{

	static int			sametimecount;
	static unsigned int	oldtime;
	static int			first = 1;
	LARGE_INTEGER		PerformanceCount;
	unsigned int		temp, t2;
	double				time;

	QueryPerformanceCounter(&PerformanceCount);

	temp = ((unsigned int)PerformanceCount.LowPart >> lowshift) |
		((unsigned int)PerformanceCount.HighPart << (32 - lowshift));

	if (first)
	{
		oldtime = temp;
		first = 0;
	}
	else
	{
		// check for turnover or backward time
		if ((temp <= oldtime) && ((oldtime - temp) < 0x10000000))
		{
			oldtime = temp;	// so we can't get stuck
		}
		else
		{
			t2 = temp - oldtime;

			time = (double)t2 * pfreq;
			oldtime = temp;

			curtime += time;

			if (curtime == lastcurtime)
			{
				sametimecount++;

				if (sametimecount > 100000)
				{
					curtime += 1.0;
					sametimecount = 0;
				}
			}
			else
			{
				sametimecount = 0;
			}

			lastcurtime = curtime;
		}
	}
	return curtime;
}

//====================================================================
//End Of Valve Software Code
//====================================================================

DWORD WINAPI FuckWindow(VOID* p)
{
	GarbageCollect(NULL);
	return 0;
}

void ReleaseAll()
{
	std::map<std::string, OutputArgs*>::iterator it;

	EnterCriticalSection(&g_csCriticalSection);
	for (it = filecache.begin(); it != filecache.end();)
	{
		delete[] it->second->pData;
		delete it->second;
		it = filecache.erase(it);
	}


	auto ti = texture_cache.begin();
	while (ti != texture_cache.end())
	{
		CTexture *tex = ti->second;
		tex->pTexture->Release();
		delete[] tex->buff;
		delete tex;
		ti = texture_cache.erase(ti);
	}

	LeaveCriticalSection(&g_csCriticalSection);
}

__declspec(naked) void __asm_ProgramExit()
{
	__asm
	{
		pushfd;
		pushad;
		call ReleaseAll;
		popad;
		popfd;
		jmp g_pProgramExit;
	}
}


double g_fUpdateUITime = 0;

int g_IsCanUpdate;

// void UpdateUIProc()
// {
// 	DWORD a = 0x009DDBD0;
// 	DWORD b = 0x009DCC20;
// 	if(g_IsCanUpdate)
// 	{
// 		__asm
// 		{
// 			call a;
// 			mov     ecx, eax;
// 			call b;
// 		}
// 	}
// }
// 
// BOOL CanUpdateUI()
// {
// 	double now = Sys_FloatTime();
// 
// 	if(now - g_fUpdateUITime >= 0.1)
// 	{
// 		g_fUpdateUITime = now;
// 		return TRUE;
// 	}
// 	return FALSE;
// }

/*
妈的智障：
00A60D00  /$  6A FF         push    -0x1
00A60D02  |.  68 3837BF00   push    00BF3738
00A60D07  |.  64:A1 0000000>mov     eax, dword ptr fs:[0]
00A60D0D  |.  50            push    eax
00A60D0E  |.  83EC 1C       sub     esp, 0x1C
00A60D11  |.  53            push    ebx
00A60D12  |.  55            push    ebp
00A60D13  |.  56            push    esi
00A60D14  |.  57            push    edi
00A60D15  |.  A1 EC4FDE00   mov     eax, dword ptr [0xDE4FEC]
00A60D1A  |.  33C4          xor     eax, esp
00A60D1C  |.  50            push    eax
00A60D1D  |.  8D4424 30     lea     eax, dword ptr [esp+0x30]
00A60D21  |.  64:A3 0000000>mov     dword ptr fs:[0], eax
00A60D27  |.  8BD9          mov     ebx, ecx
00A60D29  |.  E8 52BA0300   call    00A9C780
00A60D2E  |.  33ED          xor     ebp, ebp
00A60D30  |.  39AB A8000000 cmp     dword ptr [ebx+0xA8], ebp
00A60D36  |.  0F84 E2000000 je      00A60E1E
00A60D3C  |.  8D4C24 18     lea     ecx, dword ptr [esp+0x18]
00A60D40  |.  E8 EB17BAFF   call    00602530
00A60D45  |.  896C24 38     mov     dword ptr [esp+0x38], ebp
00A60D49  |.  8B83 A8000000 mov     eax, dword ptr [ebx+0xA8]
00A60D4F  |.  8B70 10       mov     esi, dword ptr [eax+0x10]
00A60D52  |.  2B70 0C       sub     esi, dword ptr [eax+0xC]
00A60D55  |.  C1FE 02       sar     esi, 0x2
00A60D58      83EE 01       sub     esi, 0x1
00A60D5B      78 45         js      short 00A60DA2
00A60D5D  |.  8D49 00       lea     ecx, dword ptr [ecx]
00A60D60  |>  8BBB A8000000 /mov     edi, dword ptr [ebx+0xA8]
00A60D66  |.  8B47 10       |mov     eax, dword ptr [edi+0x10]
00A60D69  |.  2B47 0C       |sub     eax, dword ptr [edi+0xC]
00A60D6C  |.  C1F8 02       |sar     eax, 0x2
00A60D6F  |.  3BF0          |cmp     esi, eax
00A60D71  |.  72 05         |jb      short 00A60D78
00A60D73  |.  E8 FF220500   |call    00AB3077
00A60D78  |>  8B4F 0C       |mov     ecx, dword ptr [edi+0xC]
00A60D7B  |.  8B3CB1        |mov     edi, dword ptr [ecx+esi*4]
00A60D7E  |.  8BCF          |mov     ecx, edi
00A60D80  |.  897C24 14     |mov     dword ptr [esp+0x14], edi
00A60D84  |.  E8 77FFFFFF   |call    00A60D00
00A60D89  |.  F647 74 10    |test    byte ptr [edi+0x74], 0x10
00A60D8D  |.  74 0E         |je      short 00A60D9D
00A60D8F  |.  8D5424 14     |lea     edx, dword ptr [esp+0x14]
00A60D93  |.  52            |push    edx
00A60D94  |.  8D4C24 1C     |lea     ecx, dword ptr [esp+0x1C]
00A60D98  |.  E8 130DE4FF   |call    008A1AB0
00A60D9D  |>  83EE 01       |sub     esi, 0x1
00A60DA0  |.^ 79 BE         \jns     short 00A60D60
00A60DA2  |>  8B4424 28     mov     eax, dword ptr [esp+0x28]
00A60DA6  |.  2B4424 24     sub     eax, dword ptr [esp+0x24]
00A60DAA  |.  33F6          xor     esi, esi
00A60DAC  |.  C1F8 02       sar     eax, 0x2
00A60DAF  |.  3BC5          cmp     eax, ebp
00A60DB1      76 31         jbe     short 00A60DE4
00A60DB3  |.  3BF0          cmp     esi, eax
00A60DB5  |.  72 05         jb      short 00A60DBC
00A60DB7  |.  E8 BB220500   call    00AB3077
00A60DBC  |>  8B4424 24     /mov     eax, dword ptr [esp+0x24]
00A60DC0  |.  8B3CB0        |mov     edi, dword ptr [eax+esi*4]
00A60DC3  |.  57            |push    edi
00A60DC4  |.  8BCB          |mov     ecx, ebx
00A60DC6  |.  E8 D5E6FFFF   |call    00A5F4A0
00A60DCB  |.  57            |push    edi
00A60DCC  |.  E8 8FCBFFFF   |call    00A5D960
00A60DD1  |.  8B4424 2C     |mov     eax, dword ptr [esp+0x2C]
00A60DD5  |.  2B4424 28     |sub     eax, dword ptr [esp+0x28]
00A60DD9  |.  46            |inc     esi
00A60DDA  |.  C1F8 02       |sar     eax, 0x2
00A60DDD  |.  83C4 04       |add     esp, 0x4
00A60DE0  |.  3BF0          |cmp     esi, eax
00A60DE2  |.^ 72 D8         \jb      short 00A60DBC
00A60DE4  |>  C74424 38 010>mov     dword ptr [esp+0x38], 0x1
00A60DEC  |.  8B4424 24     mov     eax, dword ptr [esp+0x24]
00A60DF0  |.  3BC5          cmp     eax, ebp
00A60DF2  |.  74 09         je      short 00A60DFD
00A60DF4  |.  50            push    eax
00A60DF5  |.  E8 8D1F0500   call    00AB2D87
00A60DFA  |.  83C4 04       add     esp, 0x4
00A60DFD  |>  896C24 24     mov     dword ptr [esp+0x24], ebp
00A60E01  |.  896C24 28     mov     dword ptr [esp+0x28], ebp
00A60E05  |.  896C24 2C     mov     dword ptr [esp+0x2C], ebp
00A60E09  |.  C74424 38 FFF>mov     dword ptr [esp+0x38], -0x1
00A60E11  |.  8B4C24 18     mov     ecx, dword ptr [esp+0x18]
00A60E15  |.  51            push    ecx
00A60E16  |.  E8 6C1F0500   call    00AB2D87
00A60E1B  |.  83C4 04       add     esp, 0x4
00A60E1E  |>  8B13          mov     edx, dword ptr [ebx]
00A60E20  |.  8B42 24       mov     eax, dword ptr [edx+0x24]
00A60E23  |.  8BCB          mov     ecx, ebx
00A60E25  |.  FFD0          call    eax
00A60E27  |.  6A 09         push    0x9
00A60E29  |.  8BCB          mov     ecx, ebx
00A60E2B  |.  E8 30C8FFFF   call    00A5D660
00A60E30  |.  8B4C24 30     mov     ecx, dword ptr [esp+0x30]
00A60E34  |.  64:890D 00000>mov     dword ptr fs:[0], ecx
00A60E3B  |.  59            pop     ecx
00A60E3C  |.  5F            pop     edi
00A60E3D  |.  5E            pop     esi
00A60E3E  |.  5D            pop     ebp
00A60E3F  |.  5B            pop     ebx
00A60E40  |.  83C4 28       add     esp, 0x28
00A60E43  \.  C3            retn
*/

DWORD g_tick;
BOOL IsCanUpdate()
{
	 if(GetAsyncKeyState(VK_LBUTTON) == 0) return FALSE;

	 if(GetTickCount() - g_tick > 2)
	 {
		 g_tick = GetTickCount();
		 return TRUE;
	 }

	 return FALSE;
}
__declspec(naked)void __asm_KillUpdateUI()
{
	__asm
	{
		PUSH g_pKillUpdateUI;
		pushad;
		call IsCanUpdate;
		test eax,eax
		je NoUpdate;
		popad;
		retn;
NoUpdate:
		popad;
		ADD DWORD PTR[esp], 0x7;
		retn;
	}
}

bool IsGameProcess()
{
	char szFileName[MAX_PATH];

	GetModuleFileNameA(NULL, szFileName, sizeof(szFileName));

	_strupr_s(szFileName);

	if (strstr(szFileName,"300.EXE"))
	{
		return true;
	}
	return false;

}
int cpudata[10] = { 0 };
int cpuused = 0;
DWORD ProcessAff;
DWORD SystemAff;
DWORD MainThreadCPUTID;

void UpdateCPUInfo()
{
	GetProcessAffinityMask(GetCurrentProcess(), &ProcessAff, &SystemAff);
	for (int i = 0; i < 10; i++)
	{
		if ((int)(ProcessAff & (1 << i)))
			cpudata[i] = 1;
		else
			cpudata[i] = 0;
	}
}


void SetThreadAff(HANDLE hThread)
{
	UpdateCPUInfo();
	//char szinfo[128];
	int count = 0;
	bool isdone = false;
	for (int i = 0; i < 10; i++) {
		if (cpudata[i])
			count++;
	}
	if (count > 2)
	{
		for (int i = 0; i < 10; i++)
		{
			if (cpudata[i] && !(cpuused & (1 << i)) && MainThreadCPUTID != i)
			{
				isdone = true;
				cpuused += 1 << i;
				SetThreadIdealProcessor(hThread, i);
				//SetThreadAffinityMask(hThread,(1 << i));
				break;
			}
		}
		if (!isdone)
		{
			cpuused = 0;
			for (int i = 0; i < 10; i++)
			{
				if (cpudata[i] && !(cpuused & (1 << i)) && MainThreadCPUTID != i)
				{
					isdone = true;
					cpuused += 1 << i;
					SetThreadIdealProcessor(hThread, i);
					//SetThreadAffinityMask(hThread,(1 << i));
					break;
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < 10; i++)
		{
			if (cpudata[i] && MainThreadCPUTID != i)
			{
				SetThreadIdealProcessor(hThread, i);
				//SetThreadAffinityMask(hThread,(1 << i));
				break;
			}
		}
	}
}
void SetMainThreadAff()
{
	for (int i = 2; i < 10; i++)
	{
		if (cpudata[i])
		{
			MainThreadCPUTID = i;
			SetThreadIdealProcessor(GetCurrentThread(), i);
			//SetThreadAffinityMask(GetCurrentThread(),(1 << i));
			break;
		}
	}
}
std::map<DWORD, int> g_LoadBalancingThread;


VOID UpdateLoadBalancing()
{
	THREADENTRY32 te32;
	te32.dwSize = sizeof(te32);
	HANDLE hTrap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
	if (hTrap)
	{
		if (Thread32First(hTrap, &te32))
		{
			do 
			{
				if (te32.th32OwnerProcessID == GetCurrentProcessId())
				{
					if (g_LoadBalancingThread.find(te32.th32ThreadID) == g_LoadBalancingThread.end())
					{
						HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
						if (hThread)
						{
							SetThreadAff(hThread);
							g_LoadBalancingThread[te32.th32ThreadID] = 1;
							CloseHandle(hThread);
						}
					}
					
				}
			} while (Thread32Next(hTrap,&te32));
		}
		CloseHandle(hTrap);
	}
}

// DWORD WINAPI UpdateItemPosUI(PVOID)
// {
// 	while(true)
// 	{
// 		UpdateUIProc();
// 		Sleep(10);
// 	}
// 	return 0;
// }

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		if (IsGameProcess())
		{
			InitializeCriticalSection(&g_csCriticalSection);
			Sys_Init();

			UpdateCPUInfo();
			SetMainThreadAff();
			//CreateThread(NULL,NULL,UpdateItemPosUI,NULL,NULL,NULL);
			g_LoadBalancingThread[GetCurrentThreadId()] = 1;
			FindAllAddress();
			DetourTransactionBegin();
			DetourAttach((void**)&g_pArchiveGetData, ArchiveGetData);
			DetourAttach((void**)&g_pLoadingWindowThread, FuckWindow);
			DetourAttach((void**)&g_pfnLoadTexture, newLoadTexture);
			DetourAttach((void**)&g_pProgramExit, __asm_ProgramExit);
			DetourAttach((void**)&g_pKillUpdateUI, __asm_KillUpdateUI);
			DetourTransactionCommit();
		}
		
		break;

	case DLL_PROCESS_DETACH:
		if (IsGameProcess())
		{
			DetourTransactionBegin();
			DetourDetach((void**)&g_pArchiveGetData, ArchiveGetData);
			DetourDetach((void**)&g_pLoadingWindowThread, FuckWindow);
			DetourDetach((void**)&g_pfnLoadTexture, newLoadTexture);
			DetourDetach((void**)&g_pProgramExit, __asm_ProgramExit);
			DetourDetach((void**)&g_pKillUpdateUI, __asm_KillUpdateUI);
			DetourTransactionCommit();
			DeleteCriticalSection(&g_csCriticalSection);
		}

		
		break;
	}
	return TRUE;
}


// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

#include <map>
#include <string>
#include "detours.h"
#include <d3d9.h>
#include <d3dx9tex.h>
#include "CSigMngr.h"

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

fnArchiveGetData g_pArchiveGetData = (fnArchiveGetData)0x0089F7A0;
fnGameMalloc pGameMalloc = (fnGameMalloc)0x00AB43F5;
LPTHREAD_START_ROUTINE g_pLoadingWindowThread = (LPTHREAD_START_ROUTINE)0x4836F0;
fnLoadTexture g_pfnLoadTexture = (fnLoadTexture)0x00AA6A60;
void* g_pProgramExit = (void*)0x004692D0;


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
		CopyToTextureCache(pszFile, pTexture, pUnknown);
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

DWORD WINAPI GarbageCollect(LPVOID)
{
	std::map<std::string, OutputArgs*>::iterator it;
	while (g_bRunning)
	{
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
		Sleep(30000);
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
	BreakEnd:
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
__declspec(dllexport)void Sys_Init(void)
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

			FindAllAddress();
			DetourTransactionBegin();
			DetourAttach((void**)&g_pArchiveGetData, ArchiveGetData);
			DetourAttach((void**)&g_pLoadingWindowThread, FuckWindow);
			DetourAttach((void**)&g_pfnLoadTexture, newLoadTexture);
			DetourAttach((void**)&g_pProgramExit, __asm_ProgramExit);
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
			DetourTransactionCommit();
			DeleteCriticalSection(&g_csCriticalSection);
		}

		
		break;
	}
	return TRUE;
}


// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

#include <map>
#include <string>
#include "detours.h"
#include <d3d9.h>
#include <d3dx9tex.h>


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
};

std::map<std::string, OutputArgs*> filecache;

typedef size_t(__stdcall *fnArchiveGetData)(const char* pszFile, OutputArgs* pOutput);

typedef void* (*fnGameMalloc)(size_t size);

fnArchiveGetData g_pArchiveGetData = (fnArchiveGetData)0x0089F7A0;
fnGameMalloc pGameMalloc = (fnGameMalloc)0x00AB43F5;

CRITICAL_SECTION g_csCriticalSection;
bool g_bRunning = true;

typedef HWND(WINAPI *fnCreateDialogParamA)(
	__in_opt HINSTANCE hInstance,
	__in LPCSTR lpTemplateName,
	__in_opt HWND hWndParent,
	__in_opt DLGPROC lpDialogFunc,
	__in LPARAM dwInitParam);

fnCreateDialogParamA g_pfnCreateDialogParamA = CreateDialogParamA;
double Sys_FloatTime(void);
LPTHREAD_START_ROUTINE g_pLoadingWindowThread = (LPTHREAD_START_ROUTINE)0x4836F0;


void AddToCopy(const char* pszFile, OutputArgs* pOutput)
{
	OutputArgs* pArgs = new OutputArgs();
	pArgs->cbSize = pOutput->cbSize;
	pArgs->pData = new unsigned char[pOutput->cbSize];
	pArgs->floatTime = Sys_FloatTime();
	memcpy(pArgs->pData, pOutput->pData, pOutput->cbSize);
	filecache[pszFile] = pArgs; 
}

typedef HRESULT(WINAPI *fnD3DXCreateTextureFromFileInMemoryEx)(
	_In_    LPDIRECT3DDEVICE9  pDevice,
	_In_    LPCVOID            pSrcData,
	_In_    UINT               SrcDataSize,
	_In_    UINT               Width,
	_In_    UINT               Height,
	_In_    UINT               MipLevels,
	_In_    DWORD              Usage,
	_In_    D3DFORMAT          Format,
	_In_    D3DPOOL            Pool,
	_In_    DWORD              Filter,
	_In_    DWORD              MipFilter,
	_In_    D3DCOLOR           ColorKey,
	_Inout_ D3DXIMAGE_INFO     *pSrcInfo,
	_Out_   PALETTEENTRY       *pPalette,
	_Out_   LPDIRECT3DTEXTURE9 *ppTexture
);


fnD3DXCreateTextureFromFileInMemoryEx g_pfnD3DXCreateTextureFromFileInMemoryEx;

HRESULT WINAPI newD3DXCreateTextureFromFileInMemoryEx(
	_In_    LPDIRECT3DDEVICE9  pDevice,
	_In_    LPCVOID            pSrcData,
	_In_    UINT               SrcDataSize,
	_In_    UINT               Width,
	_In_    UINT               Height,
	_In_    UINT               MipLevels,
	_In_    DWORD              Usage,
	_In_    D3DFORMAT          Format,
	_In_    D3DPOOL            Pool,
	_In_    DWORD              Filter,
	_In_    DWORD              MipFilter,
	_In_    D3DCOLOR           ColorKey,
	_Inout_ D3DXIMAGE_INFO     *pSrcInfo,
	_Out_   PALETTEENTRY       *pPalette,
	_Out_   LPDIRECT3DTEXTURE9 *ppTexture
	)
{
	HRESULT hr = g_pfnD3DXCreateTextureFromFileInMemoryEx(pDevice, pSrcData, SrcDataSize, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);

	if (SUCCEEDED(hr))
	{
		LPDIRECT3DTEXTURE9 pTexture = *ppTexture;
		pTexture->AddRef();
	}
	return hr;
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

			if ((it->second->floatTime + 120.0) < time)
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

	__asm push ecx;
	__asm pop _ecx;

	EnterCriticalSection(&g_csCriticalSection);

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

	LeaveCriticalSection(&g_csCriticalSection);

	return result;
}

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

DWORD WINAPI FuckWindow(VOID* p)
{
	GarbageCollect(NULL);
	return 0;
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
		InitializeCriticalSection(&g_csCriticalSection);
		Sys_Init();

		*(FARPROC*)&g_pfnD3DXCreateTextureFromFileInMemoryEx = GetProcAddress(LoadLibraryA("d3dx9_30.dll"), "D3DXCreateTextureFromFileInMemoryEx");

		DetourTransactionBegin();
		DetourAttach((void**)&g_pArchiveGetData, ArchiveGetData);
		DetourAttach((void**)&g_pLoadingWindowThread, FuckWindow);
		DetourAttach((void**)&g_pfnD3DXCreateTextureFromFileInMemoryEx, newD3DXCreateTextureFromFileInMemoryEx);
		DetourTransactionCommit();
		break;

	case DLL_PROCESS_DETACH:
		DetourTransactionBegin();
		DetourDetach((void**)&g_pArchiveGetData, ArchiveGetData);
		DetourDetach((void**)&g_pLoadingWindowThread, FuckWindow);
		DetourDetach((void**)&g_pfnD3DXCreateTextureFromFileInMemoryEx, newD3DXCreateTextureFromFileInMemoryEx);
		DetourTransactionCommit();
		DeleteCriticalSection(&g_csCriticalSection);
		break;
	}
	return TRUE;
}


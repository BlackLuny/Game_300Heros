// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

#include <map>
#include <string>
#include "detours.h"

typedef struct OutputArgs
{
	void *pData;
	size_t cbSize;
}OutputArgs;

std::map<std::string, OutputArgs*> filecache;

typedef size_t (__stdcall *fnArchiveGetData)(const char* pszFile, OutputArgs* pOutput );

typedef void* (*fnGameMalloc)(size_t size);

fnArchiveGetData g_pArchiveGetData = (fnArchiveGetData)0x0089F7A0;
fnGameMalloc pGameMalloc = (fnGameMalloc)0x00AB43F5;

void AddToCopy(const char* pszFile, OutputArgs* pOutput )
{
	OutputArgs* pArgs = new OutputArgs();
	pArgs->cbSize = pOutput->cbSize;
	pArgs->pData =  new unsigned char[pOutput->cbSize];
	memcpy(pArgs->pData,pOutput->pData,pOutput->cbSize);
	filecache[pszFile] = pArgs;
}

size_t __stdcall ArchiveGetData(const char* pszFile, OutputArgs* pOutput )
{
	size_t result = 0;
	unsigned int _ecx;

	__asm push ecx;
	__asm pop _ecx;

	if(filecache.find(pszFile) == filecache.end()){
		Retry:
		__asm push pOutput;
		__asm push pszFile;
		__asm mov ecx,_ecx;
		__asm call g_pArchiveGetData;
		__asm mov result,eax;

		if(result){
			AddToCopy(pszFile, pOutput);
		}
	} else {
		OutputArgs* pTemp = filecache[pszFile];
		if(!pTemp) goto Retry;
		pOutput->pData = pGameMalloc(pTemp->cbSize);
		memcpy(pOutput->pData, pTemp->pData, pTemp->cbSize);
		pOutput->cbSize = pTemp->cbSize;
		result = pTemp->cbSize;
	}

	return result;
}

double				pfreq;
double				curtime = 0.0;
double				lastcurtime = 0.0;
int					lowshift;
double Sys_FloatTime (void);

/*
================
Sys_InitFloatTime
================
*/
void Sys_InitFloatTime (void)
{
//	int		j;

	Sys_FloatTime ();
	curtime = 0.0;

	lastcurtime = curtime;
}

/*
================
Sys_Init
================
*/
__declspec(dllexport)void Sys_Init (void)
{
	LARGE_INTEGER	PerformanceFreq;
	unsigned int	lowpart, highpart;
//	OSVERSIONINFO	vinfo;


	if (!QueryPerformanceFrequency (&PerformanceFreq))
		printf ("No hardware timer available\n");

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
double Sys_FloatTime (void)
{
#ifdef _WIN32
	static int			sametimecount;
	static unsigned int	oldtime;
	static int			first = 1;
	LARGE_INTEGER		PerformanceCount;
	unsigned int		temp, t2;
	double				time;

	//if ( !cs_initialized )
	//	return 1.0;

	//EnterCriticalSection( &cs );

	//Sys_PushFPCW_SetHigh ();

	QueryPerformanceCounter (&PerformanceCount);

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

	//Sys_PopFPCW ();

	//LeaveCriticalSection( &cs );

	return curtime;
#else
	static struct timespec start_time;
	static qboolean bInitialized;
	struct timespec now;

	if ( !bInitialized )
	{
		bInitialized = 1;
		clock_gettime(1, &start_time);
	}

	clock_gettime(1, &now);
	return (now.tv_sec - start_time.tv_sec) + now.tv_nsec/1000000.0;
#endif
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		Sys_Init();
		DetourTransactionBegin();
		DetourAttach((void**)&g_pArchiveGetData,ArchiveGetData);
		DetourAttach((void**)&g_pArchiveGetData,ArchiveGetData);
		DetourTransactionCommit();
		break;

	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


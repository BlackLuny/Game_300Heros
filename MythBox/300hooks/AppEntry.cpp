#include "pch.h"
#include <TlHelp32.h>

#include "PSHelper.h"
#include "MessageChannel.h"
#include "GamePacket.h"
#include "GameTimer.h"
#include "GameCamera.h"


HMODULE g_hModule;

MessageChannel g_Channel;

DWORD OriginEip;

void Startup()
{
	g_Channel.SetConnectName("300HeroBox");

	GamePacket::GetInstance()->Attach();
	GameTimer::GetInstance()->Attach();
	GameCamera::GetInstance()->Attach();
}
__declspec(naked)void _asm_Startup()
{
	__asm
	{
		pushfd;
		pushad;
		call Startup;
		popad;
		popfd;
		jmp OriginEip;
	}
}

BOOL EnableDebugPrivilege(BOOL bEnable)
{
	BOOL fOK = FALSE;
	HANDLE hToken;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		fOK  = (GetLastError() ==  ERROR_SUCCESS);
		CloseHandle(hToken);
	}
	return fOK;
}

DWORD EnumProcess(char* pszProcessName)
{
	PROCESSENTRY32 pe32;
	HANDLE HandleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	pe32.dwSize = sizeof(pe32);
	if(HandleSnap!=INVALID_HANDLE_VALUE)
	{
		if(Process32First(HandleSnap,&pe32))
		{
			do
			{
				if(_stricmp(pe32.szExeFile,pszProcessName)==0)
				{
					CloseHandle(HandleSnap);
					return pe32.th32ProcessID;
				}
			}
			while(Process32Next(HandleSnap,&pe32));
		}
	}
	CloseHandle(HandleSnap);
	return 0;
}

bool __stdcall GameHook(DWORD pid)
{
	bool rs = false;
	char szFileName[MAX_PATH];
	GetModuleFileName(g_hModule,szFileName,sizeof(szFileName));

	EnableDebugPrivilege(TRUE);

	if(pid > 0)
	{
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
		if(hProcess)
		{
			LPVOID lpszPath = VirtualAllocEx(hProcess,NULL,0x1000,MEM_COMMIT,PAGE_EXECUTE_READWRITE);

			if(lpszPath)
			{
				WriteProcessMemory(hProcess,lpszPath,szFileName,sizeof(szFileName),NULL);

				HANDLE hThread = CreateRemoteThread(hProcess,NULL,NULL,(LPTHREAD_START_ROUTINE)LoadLibraryA,lpszPath,NULL,NULL);

				if(hThread)
				{
					rs = true;

					WaitForSingleObject(hThread,INFINITE);

					
					CloseHandle(hThread);
				}

				VirtualFreeEx(hProcess,lpszPath,NULL,MEM_RELEASE);
			}

			CloseHandle(hProcess);
		}
	}

	return rs;
}
BOOL WINAPI DllMain(HMODULE hModule,DWORD dwReason,LPVOID lpReserved){
	if(dwReason == DLL_PROCESS_ATTACH){

		char szFileName[MAX_PATH];
		g_hModule = hModule;
		DWORD ThreadId = PSHelper::GetInstance()->GetMainThreadId();

		GetModuleFileName(NULL,szFileName,sizeof(szFileName));

		char* p = strrchr(szFileName,'\\');
		if(p)
		{
			p++;
			if(stricmp(p,"300.exe") != 0)
			{
				return TRUE;
			}
		}

		if(ThreadId == GetCurrentThreadId()){
			Startup();
		}
		else
		{
			HANDLE ThreadHandle = OpenThread(THREAD_ALL_ACCESS,FALSE,ThreadId);
			if(ThreadHandle)
			{
				CONTEXT Context;
				Context.ContextFlags = CONTEXT_ALL;

				SuspendThread(ThreadHandle);
				GetThreadContext(ThreadHandle,&Context);

				OriginEip = Context.Eip;
				Context.Eip = (DWORD)&_asm_Startup;

				SetThreadContext(ThreadHandle,&Context);

				ResumeThread(ThreadHandle);

				CloseHandle(ThreadHandle);
			}
		}
		
		
	}

	return TRUE;
}
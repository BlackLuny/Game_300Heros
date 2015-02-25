#include "pch.h"
#include "PSHelper.h"
#include "GamePacket.h"


DWORD OriginEip;

void Startup()
{
	MessageBoxA(NULL,"Debug","",MB_OK);
	GamePacket::GetInstance()->Attach();
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

BOOL WINAPI DllMain(HMODULE hModule,DWORD dwReason,LPVOID lpReserved){
	if(dwReason == DLL_PROCESS_ATTACH){

		DWORD ThreadId = PSHelper::GetInstance()->GetMainThreadId();

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
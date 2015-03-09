#include "pch.h"
#include <TlHelp32.h>
#include "PSHelper.h"


PSHelper::PSHelper(void)
{
}


PSHelper::~PSHelper(void)
{
}


DWORD PSHelper::GetMainThreadId()
{
	HANDLE SnapshotHandle;
	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);

	SnapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,GetCurrentProcessId());

	if(SnapshotHandle != INVALID_HANDLE_VALUE)
	{
		if(Thread32First(SnapshotHandle,&te32))
		{
			do
			{
				if(te32.th32OwnerProcessID == GetCurrentProcessId())
				{
					CloseHandle(SnapshotHandle);
					return te32.th32ThreadID;
				}

			}while(Thread32Next(SnapshotHandle,&te32));
		}

		CloseHandle(SnapshotHandle);
	}
	return 0;
}
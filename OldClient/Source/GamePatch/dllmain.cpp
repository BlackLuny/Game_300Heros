// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "detours.h"

void Initialize();
void UnInitialize();
bool (*pSendDestroyPacket)(BOOL b, BYTE *p);
bool (__stdcall *pReceive)(net_header *hdr);

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
	unsigned char sig[] = { 0x00,0x0B,0x08,0xB1,0x24 };
	if (hdr->idenfitier == 0x2B19)
	{
		if (memcmp(sig, &hdr[1], sizeof(sig)) == 0)
		{
			SetScreenNonBlock();
		}
	}
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
			*(DWORD*)&pSendDestroyPacket = 0x00520130;
			*(DWORD*)&pReceive = 0x004E1E40;
			*(DWORD*)&pRecordwindowUIClass = 0x004DE438;

			DetourTransactionBegin();
			DetourAttach((void**)&pSendDestroyPacket, SendDestroyPacket);
			DetourAttach((void**)&pReceive, __asm_Receive);
			DetourAttach((void**)&pRecordwindowUIClass, __asm_RecordwindowUIClass);
			DetourTransactionCommit();

			LoadLibraryA("300camera.dll");
			bIsHooked = TRUE;
			return 0;
		}
		Sleep(10);
	}
	return 0;
}
void Initialize()
{
	CloseHandle(CreateThread(NULL, NULL, UpdateThread, NULL, NULL, NULL));
}

void UnInitialize()
{
	if (bIsHooked)
	{
		DetourTransactionBegin();
		DetourDetach((void**)&pSendDestroyPacket, SendDestroyPacket);
		DetourDetach((void**)&pReceive, __asm_Receive);
		DetourDetach((void**)&pRecordwindowUIClass, __asm_RecordwindowUIClass);
		DetourTransactionCommit();
	}

}
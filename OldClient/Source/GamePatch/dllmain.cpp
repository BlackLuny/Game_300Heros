// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "detours.h"

void Initialize();
void UnInitialize();
bool (*pSendDestroyPacket)(BOOL b, BYTE *p);

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
	virtual void Send(net_header *hdr) = 0;
};

CNetwork *pNetwork;

CNetwork *GetNetworkClass() {
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

void SendPacket(net_header *hdr) {
	pNetwork = GetNetworkClass();
	pNetwork->Send(hdr);
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

	SendPacket(&pkt.hdr);
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

void Initialize()
{
	*(DWORD*)&pSendDestroyPacket = 0x00520130;
	DetourTransactionBegin();
	DetourAttach((void**)&pSendDestroyPacket, SendDestroyPacket);
	DetourTransactionCommit();

	LoadLibraryA("300hooks.dll");
}

void UnInitialize()
{
	DetourTransactionBegin();
	DetourDetach((void**)&pSendDestroyPacket, SendDestroyPacket);
	DetourTransactionCommit();
}
#include "pch.h"
#include "CSigMngr.h"

#include "GamePacket.h"
#include <Psapi.h>

/*
receive:
8D 49 00 56 8B CB E8 ?? ?? ?? ?? 8B 06 03 F8 03 F0 3B 7C 24 3C
getPlayer:
(80 B8 BD 28 00 00 00 75 6D)(size) => CALL
netClass:
(8B 00 8B CE FF D0 8B 7F 2D)(size) => CALL
Virtual: 0xC => SendMessage

*/

//fix

#pragma comment(lib,"psapi.lib")
#define RECV_MSG_SIG "\x8D\x49\x00\x56\x8B\xCB\xE8\x2A\x2A\x2A\x2A\x8B\x06\x03\xF8\x03\xF0\x3B\x7C\x24\x3C"
#define RECV_MSG_SIG_BYTES sizeof(RECV_MSG_SIG)-1

#define NET_INSTANCE_SIG "\xB8\xE6\x32\x00\x00\xC6\x44\x24\x0A\x00\x66\x89\x44\x24\x08\xC7\x04\x24\x0B\x00\x00\x00"
#define NET_INSTANCE_SIG_BYTES sizeof(NET_INSTANCE_SIG)-1

#define SYS_PLAYERMNGR_SIG "\x80\xB8\xBD\x28\x00\x00\x00\x75\x6D"
#define SYS_PLAYERMNGR_SIG_BYTES sizeof(SYS_PLAYERMNGR_SIG)-1

GamePacket::GamePacket(void)
{
}


GamePacket::~GamePacket(void)
{
}
PVOID GamePacket::GetCNetwork()
{
	static void* pCNetwork = NULL;

	if(pCNetwork){
		return pCNetwork;
	}

	unsigned char *p = (unsigned char*)FIND_MEMORY(GAME_BASE_ADDRESS,NET_INSTANCE_SIG);

	if( p )
	{
		p += NET_INSTANCE_SIG_BYTES;
		if(*p == 0xE8){
			pCNetwork = p + *(DWORD*)(p + 0x1) + 0x5;
		}
	}

	return pCNetwork;
}
PVOID GamePacket::GetCPlayerMngr()
{
	static void* pCPlayerMngr = NULL;
	MODULEINFO info;
	char s[] = "Create CPlayerMgr";
	char buf[100];
	unsigned char *resolv_func = NULL;
	void*(*func)();

	if(pCPlayerMngr == NULL)
	{
		HMODULE hModule = GetModuleHandleA(NULL);

		GetModuleInformation(GetCurrentProcess(), hModule,&info,sizeof(info));
		unsigned char *p = (unsigned char *)info.lpBaseOfDll;

		for(DWORD i=0;i<(info.SizeOfImage - 0x1000);i++)
		{
			if(p[i] == 0x68){
				unsigned char *push_val = *(unsigned char**)&p[i+1];

				if(push_val > p && push_val < p + info.SizeOfImage){
					ReadProcessMemory(GetCurrentProcess(),push_val,buf,sizeof(s),NULL);

					if(memcmp(buf,s,sizeof(s)) == 0){
						resolv_func = &p[i];
						break;
					}
				}
			}
		}


		if(resolv_func){
			for(int i=0;i<0x100;i++)
			{
				unsigned char *p = resolv_func - i;
				if(p[0] == 0x6a && p[1] == 0xff){
					func = (void*(*)())p;
					pCPlayerMngr = func;
					break;
				}
			}

		}
	}


	return pCPlayerMngr;
}
PVOID GamePacket::GetSendFunc()
{
	PVOID pGetCNetwork = GetCNetwork();
	PVOID pC;
	__asm{
		call pGetCNetwork;
		mov pC,eax;
	}

	return (PVOID)*(DWORD*)((*(DWORD*)pC) + 0xC);
}
PVOID GamePacket::GetReceiveFunc()
{
	static void* pReceive = NULL;

	if(pReceive){
		return pReceive;
	}
	unsigned char *p = (unsigned char*)FIND_MEMORY(GAME_BASE_ADDRESS,RECV_MSG_SIG);

	if( p )
	{
		p += 0x6;
		if(*p == 0xE8){
			pReceive = p + *(DWORD*)(p + 0x1) + 0x5;
		}
	}

	return pReceive;
}
void GamePacket::Attach()
{
	m_pGetPlayerMngr = GetCPlayerMngr();
	m_pPreSendMessage = GetSendFunc();
	m_pPreRecvMessage = GetReceiveFunc();
	
	m_pGetNetInstance = GetCNetwork();

	
	DetourTransactionBegin();
	DetourAttach((void**)&m_pPreSendMessage,PreSendMessage);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourAttach((void**)&m_pPreRecvMessage,PreRecvMessage);
	DetourTransactionCommit();

	m_pPrevHook = SetWindowsHookExA(WH_CALLWNDPROC,HOOK_WndProc,NULL,GetCurrentThreadId());
}
void GamePacket::SwapMessage(net_packet_t* packet,BOOL IsSend)
{
	COPYDATASTRUCT copy_buf;

	copy_buf.dwData = IsSend;
	copy_buf.cbData = packet->length;
	copy_buf.lpData = packet;

	HWND swap_window = FindWindow(NULL,L"GameKit_Protocol");

	if(swap_window)
	{
		SendMessage(swap_window,WM_COPYDATA,NULL,(LPARAM)&copy_buf);
	}
}
void __stdcall GamePacket::PreRecvMessage(net_packet_t* packet)
{
	void* self = GamePacket::GetInstance()->m_pGetPlayerMngr;
	void* org = GamePacket::GetInstance()->m_pPreRecvMessage;

	GamePacket::GetInstance()->SwapMessage(packet,FALSE);

	__asm
	{
		call self;
		mov ecx,eax;
		push packet;
		call org;
	}
}

void __stdcall GamePacket::PreSendMessage(net_packet_t* packet)
{
	void* self = GamePacket::GetInstance()->m_pGetNetInstance;
	void* org = GamePacket::GetInstance()->m_pPreSendMessage;

	GamePacket::GetInstance()->SwapMessage(packet,TRUE);

	__asm
	{
		call self;
		mov ecx,eax;
		push packet;
		call org;
	}
}
LRESULT CALLBACK GamePacket::HOOK_WndProc(int code, WPARAM wParam, LPARAM lParam)
{
	PCWPSTRUCT pMsg = (PCWPSTRUCT)lParam;
	if( pMsg->message == WM_COPYDATA )
	{
		PCOPYDATASTRUCT copy_buf = (PCOPYDATASTRUCT)pMsg->lParam;

		if(copy_buf->dwData == TRUE)
		{
			PreSendMessage((net_packet_t*)copy_buf->lpData);
		}
		else
		{
			PreRecvMessage((net_packet_t*)copy_buf->lpData);
		}
	}
	return CallNextHookEx(GamePacket::GetInstance()->m_pPrevHook,code,wParam,lParam);
}
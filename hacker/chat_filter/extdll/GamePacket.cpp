#include "pch.h"
#include "CSigMngr.h"

#include "GamePacket.h"

#define SEND_MSG_SIG "\x56\x8B\x74\x24\x08\x57\x8B\xF9\x85\xF6\x75\x17\x68\x2A\x2A\x2A\x2A\x8D\x44\x24\x10\x50"
#define SEND_MSG_SIG_BYTES sizeof(SEND_MSG_SIG)-1

#define RECV_MSG_SIG "\x55\x8B\xEC\x83\xE4\xF8\x6A\xFF\x68\x2A\x2A\x2A\x2A\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\xF0\x02\x00\x00\xA1"
#define RECV_MSG_SIG_BYTES sizeof(RECV_MSG_SIG)-1

#define NET_INSTANCE_SIG "\x6A\xFF\x68\x2A\x2A\x2A\x2A\x64\xA1\x00\x00\x00\x00\x50\x51\xA1\x2A\x2A\x2A\x2A\x33\xC4\x50\x8D\x44\x24\x08\x64\xA3\x00\x00\x00\x00\xA1\x2A\x2A\x2A\x2A\x85\xC0\x75\x3C\x6A\x01\xE8\x2A\x2A\x2A\x2A\x68\x98\x00\x00\x00"
#define NET_INSTANCE_SIG_BYTES sizeof(NET_INSTANCE_SIG)-1

#define SYS_PLAYERMNGR_SIG "\x6A\x01\x50\x51\x52\x83\xEC\x08\xD9\x5C\x24\x04\xD9\x44\x24\x1C\xD9\x1C\x24"
#define SYS_PLAYERMNGR_SIG_BYTES sizeof(SYS_PLAYERMNGR_SIG)-1

GamePacket::GamePacket(void)
{
}


GamePacket::~GamePacket(void)
{
}

void* GamePacket::FindPlayerMngr()
{
	unsigned char* p = (unsigned char*)FIND_MEMORY(GAME_BASE_ADDRESS,SYS_PLAYERMNGR_SIG);
	if(p)
	{
		p = &p[0x13];
		return  p + *(unsigned long*)&p[1] + 5;
	}
	return 0;
}
void GamePacket::Attach()
{
	m_pPreSendMessage = FIND_MEMORY(GAME_BASE_ADDRESS,SEND_MSG_SIG);
	m_pPreRecvMessage = FIND_MEMORY(GAME_BASE_ADDRESS,RECV_MSG_SIG);
	m_pGetNetInstance = FIND_MEMORY(GAME_BASE_ADDRESS,NET_INSTANCE_SIG);
	m_pGetPlayerMngr = FindPlayerMngr();


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
	//COPYDATASTRUCT copy_buf;

	//copy_buf.dwData = IsSend;
	//copy_buf.cbData = packet->length;
	//copy_buf.lpData = packet;

	

	//HWND swap_window = FindWindow(NULL,"GameKit_Protocol");

	//if(swap_window)
	//{
	//	SendMessage(swap_window,WM_COPYDATA,NULL,(LPARAM)&copy_buf);
	//}
}
void __stdcall GamePacket::PreRecvMessage(net_packet_t* packet)
{
	void* self = GamePacket::GetInstance()->m_pGetPlayerMngr;
	void* org = GamePacket::GetInstance()->m_pPreRecvMessage;

	GamePacket::GetInstance()->SwapMessage(packet,FALSE);

	if(packet->identifier == 0x044F){
		return;
	}

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
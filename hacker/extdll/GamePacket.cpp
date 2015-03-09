#include "pch.h"
#include "CSigMngr.h"
#include <direct.h>

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
#pragma pack(1)
struct st_skill_info
{
	
	unsigned char u[5];
	int delta;
	unsigned short id;
};
#pragma pack()

unsigned short source_id = 0;
unsigned short target_id = 0;

void GamePacket::SwapMessage(net_packet_t* packet,BOOL IsSend)
{
	if(IsSend == TRUE)
	{
		BYTE buf[] = {0x00,0x13,0x53,0x08,0xA0};
		if(packet->identifier == 0x2AFA)
		{
			st_skill_info* start = (st_skill_info*)&((unsigned char*)packet)[sizeof(net_packet_t)];
			if(start->id == source_id)
			{
				memcpy(start,buf,sizeof(buf));
				start->delta = -1;
				start->id = target_id;
			}
		}
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

		if(copy_buf->dwData)
		{
			source_id = (unsigned short)((copy_buf->dwData >> 16) & 0xFFFF);
			target_id = (unsigned short)(copy_buf->dwData & 0xFFFF);
		
		}
	}
	return CallNextHookEx(GamePacket::GetInstance()->m_pPrevHook,code,wParam,lParam);
}
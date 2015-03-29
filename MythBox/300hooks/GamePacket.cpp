#include "pch.h"
#include "CSigMngr.h"
#include "MessageChannel.h"
#include "GamePacket.h"



extern MessageChannel g_Channel;

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

bool GamePacket::InvokePreSendMessage(net_packet_t* input,net_packet_t*& output,BOOL& Blocked)
{
	MessageBuffer InputMessage;
	MessageBuffer OutputMessage;

	InputMessage.SetDataSize(input->length);
	memcpy(InputMessage.m_pData,input,input->length);

	output = NULL;

	if(g_Channel.CallApi("PKT_PreSendMessage",InputMessage,OutputMessage))
	{
		if(OutputMessage.m_uSize > 0)
		{
			DWORD dwLength = OutputMessage.m_uSize - sizeof(BOOL);

			Blocked = *(BOOL*)OutputMessage.m_pData;
			output = (net_packet_t*)malloc(dwLength);
			memcpy(output,&OutputMessage.m_pData[sizeof(BOOL)],dwLength);
			return true;
		}
	}

	return false;
}
bool GamePacket::InvokePostSendMessage(net_packet_t* input)
{
	MessageBuffer InputMessage;
	MessageBuffer OutputMessage;
	InputMessage.SetDataSize(input->length);
	memcpy(InputMessage.m_pData,input,input->length);

	if(g_Channel.CallApi("PKT_PostSendMessage",InputMessage,OutputMessage))
	{
		return true;
	}

	return false;
}


bool GamePacket::InvokePreReceiveMessage(net_packet_t* input,net_packet_t*& output,BOOL& Blocked)
{
	MessageBuffer InputMessage;
	MessageBuffer OutputMessage;

	InputMessage.SetDataSize(input->length);
	memcpy(InputMessage.m_pData,input,input->length);

	output = NULL;

	if(g_Channel.CallApi("PKT_PreReceiveMessage",InputMessage,OutputMessage))
	{
		if(OutputMessage.m_uSize > 0)
		{
			DWORD dwLength = OutputMessage.m_uSize - sizeof(BOOL);

			Blocked = *(BOOL*)OutputMessage.m_pData;
			output = (net_packet_t*)malloc(dwLength);
			memcpy(output,&OutputMessage.m_pData[sizeof(BOOL)],dwLength);
			return true;
		}
	}

	return false;
}
bool GamePacket::InvokePostReceiveMessage(net_packet_t* input)
{
	MessageBuffer InputMessage;
	MessageBuffer OutputMessage;
	InputMessage.SetDataSize(input->length);
	memcpy(InputMessage.m_pData,input,input->length);

	if(g_Channel.CallApi("PKT_PostReceiveMessage",InputMessage,OutputMessage))
	{
		return true;
	}

	return false;
}


void GamePacket::Attach()
{
	m_pPreSendMessage = FIND_MEMORY(GAME_BASE_ADDRESS,SEND_MSG_SIG);
	m_pPreRecvMessage = FIND_MEMORY(GAME_BASE_ADDRESS,RECV_MSG_SIG);
	m_pGetNetInstance = FIND_MEMORY(GAME_BASE_ADDRESS,NET_INSTANCE_SIG);
	m_pGetPlayerMngr = FindPlayerMngr();

	DetourTransactionBegin();
	DetourAttach((void**)&m_pPreSendMessage,NET_SendMessage);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourAttach((void**)&m_pPreRecvMessage,NET_ReceiveMessage);
	DetourTransactionCommit();

	g_Channel.HookMessage("PKT_UserSendMessage",NET_UserSendMessage);
	g_Channel.HookMessage("PKT_UserReceiveMessage",NET_UserReceiveMessage);
}
void __stdcall GamePacket::NET_ReceiveMessage(net_packet_t* packet)
{
	BOOL Blocked = FALSE;
	net_packet_t* output;
	void* self = GamePacket::GetInstance()->m_pGetPlayerMngr;
	void* org = GamePacket::GetInstance()->m_pPreRecvMessage;

	if(GamePacket::GetInstance()->InvokePreReceiveMessage(packet,output,Blocked))
	{
		if(Blocked == FALSE)
		{
			__asm
			{
				call self;
				mov ecx,eax;
				push output;
				call org;
			}
		}

		free(output);
	}
	else
	{
		__asm
		{
			call self;
			mov ecx,eax;
			push packet;
			call org;
		}
	}

	GamePacket::GetInstance()->InvokePostReceiveMessage(packet);
}

void __stdcall GamePacket::NET_SendMessage(net_packet_t* packet)
{
	net_packet_t* output;
	BOOL Blocked = FALSE;

	void* self = GamePacket::GetInstance()->m_pGetNetInstance;
	void* org = GamePacket::GetInstance()->m_pPreSendMessage;


	if(GamePacket::GetInstance()->InvokePreSendMessage(packet,output,Blocked))
	{
		if(Blocked == FALSE)
		{
			__asm
			{
				call self;
				mov ecx,eax;
				push output;
				call org;
			}
		}

		free(output);
	}
	else
	{
		__asm
		{
			call self;
			mov ecx,eax;
			push packet;
			call org;
		}
	}

	GamePacket::GetInstance()->InvokePostSendMessage(packet);
}


void GamePacket::NET_UserSendMessage(MessageBuffer& Input,MessageBuffer& Output)
{
	void* self = GamePacket::GetInstance()->m_pGetNetInstance;
	void* org = GamePacket::GetInstance()->m_pPreSendMessage;
	void* packet = Input.m_pData;
	__asm
	{
		call self;
		mov ecx,eax;
		push packet;
		call org;
	}
}
void GamePacket::NET_UserReceiveMessage(MessageBuffer& Input,MessageBuffer& Output)
{
	void* self = GamePacket::GetInstance()->m_pGetPlayerMngr;
	void* org = GamePacket::GetInstance()->m_pPreRecvMessage;
	void* input = Input.m_pData;
	__asm
	{
		call self;
		mov ecx,eax;
		push input;
		call org;
	}
}
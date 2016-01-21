#pragma once

#pragma pack(1)
struct net_packet_s
{
	unsigned long length;
	unsigned long timestamp;
	unsigned short identifier;
};

typedef net_packet_s net_packet_t;
#pragma pack()
class GamePacket
{
public:
	GamePacket(void);
	~GamePacket(void);

	HHOOK m_pPrevHook;
	void* m_pGetPlayerMngr;
	void* m_pGetNetInstance;
	void* m_pPreRecvMessage;
	void* m_pPreSendMessage;
	

	DECLARE_INSTANCE(GamePacket);

	PVOID GetCNetwork();
	PVOID GetCPlayerMngr();
	PVOID GetSendFunc();
	PVOID GetReceiveFunc();

	static void __stdcall PreRecvMessage(net_packet_t* packet);
	static void __stdcall PreSendMessage(net_packet_t* packet);
	static LRESULT CALLBACK HOOK_WndProc(int code, WPARAM wParam, LPARAM lParam);

	void SwapMessage(net_packet_t* packet,BOOL IsSend);

	void Attach();
	void Detach();
};


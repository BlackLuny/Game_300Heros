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

	void* m_pGetPlayerMngr;
	void* m_pGetNetInstance;
	void* m_pPreRecvMessage;
	void* m_pPreSendMessage;
	
	DECLARE_INSTANCE(GamePacket);

	void* FindPlayerMngr();

	bool InvokePreSendMessage(net_packet_t* input,net_packet_t*& output,BOOL& Blocked);
	bool InvokePostSendMessage(net_packet_t* input);

	bool InvokePreReceiveMessage(net_packet_t* input,net_packet_t*& output,BOOL& Blocked);
	bool InvokePostReceiveMessage(net_packet_t* input);

	static void __stdcall NET_ReceiveMessage(net_packet_t* packet);
	static void __stdcall NET_SendMessage(net_packet_t* packet);

	static void NET_UserSendMessage(MessageBuffer& Input,MessageBuffer& Output);
	static void NET_UserReceiveMessage(MessageBuffer& Input,MessageBuffer& Output);

	void Attach();
	void Detach();
};


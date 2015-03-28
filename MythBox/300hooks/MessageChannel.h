#pragma once

#include <vector>

class MessageBuffer
{
public:
	PBYTE m_pData;
	INT m_uSize;

	MessageBuffer();
	~MessageBuffer();


	VOID SetDataSize(INT uSize);
};

typedef VOID (* HOOK_MESSAGE_FUNC)(MessageBuffer& Input,MessageBuffer& Output);

#define CH_CALL 0x00000001
#define CH_RET 0x00000002

class MessageOpertaion
{
public:
	const char* ApiName;
	HOOK_MESSAGE_FUNC Func;
};

class MessageChannel
{
private:
	HANDLE m_hPipe;
	const char* m_pszConnectName;

	std::vector<MessageOpertaion> m_vMessageList;

public:
	MessageChannel(void);
	~MessageChannel(void);

	DECLARE_INSTANCE(MessageChannel);

	VOID SetConnectName(const char* ConnectName);
	BOOL CallApi(const char* ApiName,MessageBuffer& Input,MessageBuffer& Output);

	BOOL UpdateConnection();

	BOOL MSG_WriteByte(BYTE s);
	BOOL MSG_WriteAPI(const char* ApiName);
	BOOL MSG_WriteBytes(MessageBuffer& Input);

	BOOL MSG_ReadByte(BYTE& s);
	BOOL MSG_ReadAPI(char* ApiName);
	BOOL MSG_ReadBytes(MessageBuffer& Output);


	BOOL WriteBuffer(const unsigned char* Buffer,int BufferSize);
	BOOL ReadBuffer(unsigned char* Buffer,int BufferSize);


	VOID CallLocalApi(const char* ApiName,MessageBuffer& Input,MessageBuffer& Output);
	VOID HookMessage(const char* ApiName,HOOK_MESSAGE_FUNC HookMessageFunc);
};


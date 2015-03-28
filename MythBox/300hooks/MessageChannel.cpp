#include "pch.h"
#include "MessageChannel.h"

MessageBuffer::MessageBuffer()
{
	m_uSize = 0;
	m_pData = (PBYTE)malloc(m_uSize);
}
MessageBuffer::~MessageBuffer()
{
	m_uSize = 0;
	if(m_pData)
	{
		free(m_pData);
		m_pData = NULL;
	}
}

VOID MessageBuffer::SetDataSize(INT uSize)
{
	if(uSize == 0){
		free(m_pData);
		m_pData = (PBYTE)malloc(0);
	}
	else
	{
		m_pData = (PBYTE)realloc(m_pData,uSize);
	}
	m_uSize = uSize;
}

MessageChannel::MessageChannel(void)
{
	m_pszConnectName = "";
	m_hPipe = INVALID_HANDLE_VALUE;
}


MessageChannel::~MessageChannel(void)
{
}

VOID MessageChannel::SetConnectName(const char* ConnectName)
{
	m_pszConnectName = ConnectName;
}

BOOL MessageChannel::CallApi(const char* ApiName,MessageBuffer& Input,MessageBuffer& Output)
{
	BYTE s;

	if(UpdateConnection())
	{
		if((MSG_WriteByte(CH_CALL) == FALSE) || (MSG_WriteAPI(ApiName) == FALSE) || (MSG_WriteBytes(Input) == FALSE))
		{
			return FALSE;
		}

		while(MSG_ReadByte(s))
		{
			if(s == CH_CALL)
			{
				char RemoteAPIName[32];
				MessageBuffer RemoteInput;
				MessageBuffer RemoteOutput;

				MSG_ReadAPI(RemoteAPIName);
				MSG_ReadBytes(RemoteInput);

				CallLocalApi(RemoteAPIName,RemoteInput,RemoteOutput);

				MSG_WriteByte(CH_RET);
				MSG_WriteBytes(RemoteOutput);
			}

			if(s == CH_RET)
			{
				return MSG_ReadBytes(Output);
			}
		}
	}

	return FALSE;
}

BOOL MessageChannel::MSG_ReadByte(BYTE& s)
{
	return ReadBuffer((unsigned char*)&s,sizeof(s));
}
BOOL MessageChannel::MSG_ReadAPI(char* ApiName)
{
	return ReadBuffer((unsigned char*)ApiName,32);
}
BOOL MessageChannel::MSG_ReadBytes(MessageBuffer& Output)
{
	INT nSize = 0;

	if(ReadBuffer((unsigned char*)&nSize,sizeof(nSize)) == FALSE)
	{
		return FALSE;
	}

	Output.SetDataSize(nSize);

	if(nSize == 0)
		return TRUE;

	return ReadBuffer(Output.m_pData,nSize);
}

BOOL MessageChannel::UpdateConnection()
{
	if(m_hPipe == INVALID_HANDLE_VALUE)
	{
		char szNamedPipe[128];
		sprintf(szNamedPipe,"\\\\.\\pipe\\%s",m_pszConnectName);
		if(WaitNamedPipeA(szNamedPipe,NMPWAIT_USE_DEFAULT_WAIT))
		{
			m_hPipe = CreateFileA(szNamedPipe,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			if(m_hPipe != INVALID_HANDLE_VALUE)
			{
				return TRUE;
			}
		}

		return FALSE;
	}

	return TRUE;
}
BOOL MessageChannel::MSG_WriteByte(BYTE s)
{
	return WriteBuffer((const unsigned char*)&s,sizeof(s));
}
BOOL MessageChannel::MSG_WriteAPI(const char* ApiName)
{
	char szBuf[32] = {0};
	strcpy(szBuf,ApiName);

	return WriteBuffer((const unsigned char*)&szBuf,sizeof(szBuf));
}
BOOL MessageChannel::MSG_WriteBytes(MessageBuffer& Input)
{
	if(WriteBuffer((const unsigned char*)&Input.m_uSize,sizeof(Input.m_uSize)) == FALSE)
	{
		return FALSE;
	}
	if(Input.m_uSize > 0)
	{
		return WriteBuffer((const unsigned char*)Input.m_pData,Input.m_uSize);
	}

	return TRUE;
}
BOOL MessageChannel::WriteBuffer(const unsigned char* Buffer,int BufferSize)
{
	BOOL IsSuccess = TRUE;
	DWORD NumberOfBytesWritten;

	if(WriteFile(m_hPipe,Buffer,BufferSize,&NumberOfBytesWritten,NULL) == FALSE)
	{
		IsSuccess = FALSE;
		CloseHandle(m_hPipe);
		m_hPipe = INVALID_HANDLE_VALUE;
	}

	return IsSuccess;
}
BOOL MessageChannel::ReadBuffer(unsigned char* Buffer,int BufferSize)
{
	BOOL IsSuccess = TRUE;

	DWORD NumberOfBytesRead;

	if(ReadFile(m_hPipe,Buffer,BufferSize,&NumberOfBytesRead,NULL) == FALSE)
	{
		IsSuccess = FALSE;
		CloseHandle(m_hPipe);
		m_hPipe = INVALID_HANDLE_VALUE;
	}

	return IsSuccess;
}

VOID MessageChannel::HookMessage(const char* ApiName,HOOK_MESSAGE_FUNC HookMessageFunc)
{
	for(size_t i=0;i<m_vMessageList.size();i++)
	{
		if(strcmp(m_vMessageList[i].ApiName,ApiName)==0)
		{
			m_vMessageList[i].Func = HookMessageFunc;
			return;
		}
	}

	MessageOpertaion Msg;

	Msg.ApiName = ApiName;
	Msg.Func = HookMessageFunc;

	m_vMessageList.push_back(Msg);
}
VOID MessageChannel::CallLocalApi(const char* ApiName,MessageBuffer& Input,MessageBuffer& Output)
{
	for(size_t i=0;i<m_vMessageList.size();i++)
	{
		if(strcmp(m_vMessageList[i].ApiName,ApiName)==0)
		{
			m_vMessageList[i].Func(Input,Output);
			return;
		}
	}
}
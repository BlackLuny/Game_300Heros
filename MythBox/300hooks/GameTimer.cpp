#include "pch.h"
#include "MessageChannel.h"

#include "GameTimer.h"


extern MessageChannel g_Channel;

GameTimer::GameTimer(void)
{
	TimerId = 0;
}


GameTimer::~GameTimer(void)
{
}

void GameTimer::Attach()
{
	TimerId = SetTimer(NULL,NULL,30,Timer);
}
void GameTimer::Detach()
{
	if(TimerId)
	{
		KillTimer(NULL,TimerId);
		TimerId = 0;
	}
}

VOID CALLBACK GameTimer::Timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	MessageBuffer InputMessage;
	MessageBuffer OutputMessage;

	g_Channel.CallApi("SYS_Update",InputMessage,OutputMessage);
}
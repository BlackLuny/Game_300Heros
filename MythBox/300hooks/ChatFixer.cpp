#include "pch.h"
#include "ChatFixer.h"
#include "CSigMngr.h"

#define EDIT_MESSAGE_PROC "\x80\xBE\x25\x07\x00\x00\x00\x75\xF0\x8B\x86\xB8\x04\x00\x00\xC1\xE8\x0A\xA8\x01\x74\xE3\x32\xDB"
#define EDIT_MESSAGE_PROC_BYTES (sizeof(EDIT_MESSAGE_PROC) - 1)

void* g_pChatMessageProc;


ChatFixer::ChatFixer(void)
{
}


ChatFixer::~ChatFixer(void)
{
}


void ChatFixer::Attach()
{
	unsigned char *p = (unsigned char *)FIND_MEMORY( GAME_BASE_ADDRESS, EDIT_MESSAGE_PROC );

	if( p )
	{
		unsigned char* head = p - 0x44;

		g_pChatMessageProc = head;

		DetourTransactionBegin();
		DetourAttach( (void**)&g_pChatMessageProc, Proc );
		DetourTransactionCommit();
	}
}
void __declspec(naked) ChatFixer::Proc()
{
	__asm
	{
		PUSHFD;
		PUSHAD;

		MOV ESI,ECX;
		LEA ESI,[ESI + 0x748];

		CMP DWORD PTR [ESI] ,0;
		JNZ _BreakPatch;

		MOV DWORD PTR [ESI],0x3C

_BreakPatch:

		POPAD;
		POPFD;

		JMP g_pChatMessageProc;
	}
}
.486
.model flat,stdcall
option casemap:none

include 300hooks.inc

.data
g_szWindowClass db 'GameKit',0
g_szGameClassName db 'WWW_JUMPW_COM',0

g_szNotifMessage db 'active',0


.data?
g_pPrevHooks dd ?
g_pNET_Send dd ?


g_pRIP dd ?
g_pRInvoke dd ?
.code


UI_ShowData proc uses edi esi,swapbuf
	LOCAL r_length
	LOCAL r_copydata:COPYDATASTRUCT
	
	mov edi,swapbuf
	
	mov eax,dword ptr [edi]
	
	lea esi,r_copydata
	assume esi:ptr COPYDATASTRUCT
	
	mov dword ptr [esi].cbData,eax
	mov dword ptr [esi].lpData,edi
	mov dword ptr [esi].dwData,0
	
	
	invoke FindWindow,offset g_szWindowClass,NULL
	test eax,eax
	je @F
	
	lea edx,r_copydata
	invoke SendMessage,eax,WM_COPYDATA,NULL,edx
	
@@:
	
	ret

UI_ShowData endp

NET_GetInstance proc
	
	mov eax,0083D8F0h
	call eax
	
	ret
NET_GetInstance endp

NET_SendReal proc

	push    esi
	mov     esi, dword ptr [esp+8h]
	
	push 0083D315h
	retn

NET_SendReal endp

NET_Send proc uses esi,param
	
	invoke NET_GetInstance
	mov esi,eax
	
	
	;swap buffers to ui applications
	
	push param
	call UI_ShowData
	
	;done----------------------------
	
	mov ecx,esi
	push param
	call NET_SendReal
	
	ret
NET_Send endp



DetourAttach proc uses esi,source,target
	LOCAL op
	
	invoke VirtualProtect,source,5h,PAGE_EXECUTE_READWRITE,addr op
	
	mov esi,source
	
	mov eax,target
	sub eax,esi
	sub eax,5
	
	mov byte ptr[esi],0E9h
	inc esi
	mov dword ptr [esi],eax
	
	ret

DetourAttach endp

HOOK_WndProc proc uses edi esi,nCode:dword,wParam:dword,lParam:dword

	mov edi,lParam
	assume edi:ptr CWPSTRUCT
	
	.if [edi].message == WM_COPYDATA
		
		mov esi,[edi].lParam
		assume esi:ptr COPYDATASTRUCT
		
		
		invoke NET_Send,[esi].lpData
		
		
		assume esi:nothing
	.endif
	
	assume edi:nothing
	
	
	invoke CallNextHookEx,g_pPrevHooks,nCode,wParam,lParam
	ret
HOOK_WndProc endp

HOOK_Attach proc
	
	invoke GetCurrentThreadId
	invoke SetWindowsHookEx,WH_CALLWNDPROC,offset HOOK_WndProc,0,eax
	
	mov g_pPrevHooks,eax
	
	invoke DetourAttach,0083D310h,offset NET_Send
	
	
	ret

HOOK_Attach endp


SYS_GatewayProc proc
	
	pushfd
	pushad
	
	call g_pRInvoke
	
	popad
	popfd
	
	jmp g_pRIP
SYS_GatewayProc endp

SYS_InvokeTo proc uses esi,ThreadId,func
	LOCAL context:CONTEXT
	invoke OpenThread,THREAD_ALL_ACCESS,FALSE,ThreadId
	
	.if eax
		mov esi,eax
		
		mov context.ContextFlags,CONTEXT_ALL
		
		invoke SuspendThread,esi
		
		mov eax,func
		mov g_pRInvoke,eax
		
		invoke GetThreadContext,esi,addr context
		
		
		mov eax,context.regEip
		mov g_pRIP,eax
		
		mov context.regEip,offset SYS_GatewayProc
		
		
		
		invoke SetThreadContext,esi,addr context
		
		invoke ResumeThread,esi
		
		invoke CloseHandle,esi
	.endif
	
	ret

SYS_InvokeTo endp



SYS_Attach proc uses esi
	
	invoke MessageBox,NULL,offset g_szNotifMessage,offset g_szNotifMessage,MB_OK
	invoke FindWindow,offset g_szGameClassName,NULL
	test eax,eax
	je @F
	
	invoke GetWindowThreadProcessId,eax,NULL
	
	test eax,eax
	je @F
	mov esi,eax
	invoke GetCurrentThreadId
	cmp eax,esi
	je @OnCurThread
	invoke SYS_InvokeTo,eax,offset HOOK_Attach
	jmp @F
@OnCurThread:
	call HOOK_Attach
	
	
@@:

	ret
SYS_Attach endp

SYS_Detach proc
	
	
	ret
SYS_Detach endp

DllEntry proc hInst:HINSTANCE, reason:DWORD, reserved1:DWORD
	
	.if reason == DLL_PROCESS_ATTACH
		invoke SYS_Attach
	.elseif reason == DLL_PROCESS_DETACH
		invoke SYS_Detach
	.endif
	mov eax, TRUE
	ret
DllEntry Endp


;#########################################################################

End DllEntry

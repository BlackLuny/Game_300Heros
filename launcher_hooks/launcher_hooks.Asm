.386
.model flat,stdcall
option casemap:none

include windows.inc
include kernel32.inc
include user32.inc

includelib kernel32.lib
includelib user32.lib


.data
g_szProcName db 'launcher.exe',0
g_szClassNameByName db 'WWW_JUMPW_COM',0


g_szUser32 db 'user32.dll',0
g_szFindWindowA db 'FindWindowA',0
.data?

g_pFindWindowA dd ?
.code

NewFindWindow proc lpClassName,lpWindowName
	LOCAL hWnd
	
	
	push lpWindowName
	push lpClassName
	call g_pFindWindowA
	
	mov hWnd,eax
	
	mov ecx,lpClassName
	test ecx,ecx
	jz @F
	
	invoke lstrcmpi,lpClassName,offset g_szClassNameByName
	test eax,eax
	jnz @F
	
	xor eax,eax
	ret
	
@@:
	mov eax,hWnd
	ret
	
NewFindWindow endp



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

TL_GetProc proc DllName,ProcName
	
	invoke GetModuleHandle,DllName
	invoke GetProcAddress,eax,ProcName
	
	ret

TL_GetProc endp

HOOK_Attach proc uses esi edi
	invoke VirtualAlloc,NULL,1000h,MEM_COMMIT,PAGE_EXECUTE_READWRITE
	mov g_pFindWindowA,eax
	mov edi,eax
	
	invoke TL_GetProc,offset g_szUser32,offset g_szFindWindowA
	mov esi,eax
	
	mov ecx,5
	cld
	rep movsb
	
	sub esi,edi
	sub esi,5
	mov byte ptr[edi],0e9h
	inc edi
	mov dword ptr [edi],esi
	
	invoke TL_GetProc,offset g_szUser32,offset g_szFindWindowA
	
	mov edi,eax
	push offset NewFindWindow
	push edi
	call DetourAttach
	
	ret
HOOK_Attach endp


DllEntry proc hModule,dwReason,lpReserved
	
	.if dwReason == DLL_PROCESS_ATTACH
		call HOOK_Attach
	.endif
	mov eax,TRUE
	ret

DllEntry endp

end DllEntry
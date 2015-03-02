.386
.model flat, stdcall  ;32 bit memory model
option casemap :none  ;case sensitive

include game_sells.inc
include msvcrt.inc
includelib msvcrt.lib
.data
game_proc_name db '300.exe',0
.code

start:

	invoke GetModuleHandle,NULL
	mov		hInstance,eax

    invoke InitCommonControls
	invoke DialogBoxParam,hInstance,IDD_DIALOG1,NULL,addr DlgProc,NULL
	invoke ExitProcess,0

;########################################################################


find_game_proc proc
	LOCAL pe32:PROCESSENTRY32
	LOCAL hsnap
	
	mov pe32.dwSize,sizeof PROCESSENTRY32
	
	invoke CreateToolhelp32Snapshot,TH32CS_SNAPPROCESS,0
	
	.if eax != INVALID_HANDLE_VALUE
		mov hsnap,eax
		invoke Process32First,hsnap,addr pe32
		.if eax
			.repeat
				invoke lstrcmpi,addr pe32.szExeFile,offset game_proc_name
				.if eax == 0
					invoke CloseHandle,hsnap
					mov eax,pe32.th32ProcessID
					ret
				.endif
				invoke Process32Next,hsnap,addr pe32
			.until !eax
		.endif
		invoke CloseHandle,hsnap
	.endif
	xor eax,eax
	ret

find_game_proc endp


do_some_thing proc
	LOCAL bufs
	
	mov bufs,1
	invoke find_game_proc
	.if eax > 0
		invoke OpenProcess,PROCESS_ALL_ACCESS,FALSE,eax
		.if eax
			push eax
			lea edx,bufs
			invoke WriteProcessMemory,eax,18CAFECh,edx,sizeof bufs,0
			
			pop eax
			
			invoke CloseHandle,eax
		.endif
	.endif
	
	ret

do_some_thing endp

DlgProc proc hWin:HWND,uMsg:UINT,wParam:WPARAM,lParam:LPARAM

	mov		eax,uMsg
	.if eax==WM_INITDIALOG

	.elseif eax==WM_COMMAND
		.if wParam == 1001
			invoke do_some_thing
		.endif
	.elseif eax==WM_CLOSE
		invoke EndDialog,hWin,0
	.else
		mov		eax,FALSE
		ret
	.endif
	mov		eax,TRUE
	ret

DlgProc endp

end start

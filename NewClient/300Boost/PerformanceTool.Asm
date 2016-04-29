.486
.model flat,stdcall
option casemap:none
.data
g_fakePtr dd ?
.code

PerformanceRecordBegin proc
	pushfd
	pushad
	lea ebx,[esp+24h]
	push ebx
	
	
	
	popad
	popfd
	
PerformanceRecordBegin endp

PerformanceRecordEnd proc
	pushfd
	pushad
	
	
	
	popad
	popfd
@ModifyJump:
	jmp g_fakePtr
PerformanceRecordEnd endp

End

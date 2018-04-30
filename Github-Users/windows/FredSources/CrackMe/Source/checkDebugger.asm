
%ifdef WIN64
section .text

global _checkDebugger

_checkDebugger :
BITS 64
	push   rbx
	mov    rax, [gs:30h]  
	mov    rbx, [rax+60h]  
	movzx  eax, byte [rbx+2]  
	pop	   rbx
	ret 
%endif

%ifdef WIN32

section .text

global _checkDebugger

_checkDebugger :
BITS 32
	push ebx
	mov  eax, 0
	mov  ebx, [fs:0x30] 
	mov  al , byte [ebx+2] 
	pop  ebx
	ret
%endif




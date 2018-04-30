
extern  _restauration
extern  _getTrueEIP
extern  _getBuffer_FPU


section .text

global _StartMagicFunction 
global _EndMagicFunction

%ifdef LIN64
%define MODE_64
%endif
%ifdef WIN64
%define MODE_64
%endif

_StartMagicFunction :
%ifdef MODE_64
BITS 64
		pushfq                     ; placement en pile de la valeur des flags cpu
		mov qword [rsp-32],  rax   ; sauvegarde du registre rax
		mov qword rax, [rsp]       ; déplacement de la sauvegarde de flags dans la pile
		mov qword [rsp-8],  rax
		mov rax ,qword [rsp-32]    ; restauration du registre rax
		popfq                      ; supression de la sauvegarde des flags cpu

		sub rsp,144
		mov qword [rsp+8],    rax
		mov qword [rsp+16],   rbx
		mov qword [rsp+24],   rcx
		mov qword [rsp+32],   rdx
		mov qword [rsp+40],   rsi
		mov qword [rsp+48],   rdi
		mov qword [rsp+56],   r8
		mov qword [rsp+64],   r9
		mov qword [rsp+72],   r10
		mov qword [rsp+80],   r11
		mov qword [rsp+88],   r12
		mov qword [rsp+96],   r13
		mov qword [rsp+104],  r14
		mov qword [rsp+112],  r15
		mov qword [rsp+120],  rbp
		; ESP+128 ==> Flags

		mov rax,  qword _getBuffer_FPU  ; sauvegarde des registres FPU
		call rax
		fxsave64  [rax]

		mov rax,  qword _getTrueEIP   ; sauvegarde de l'adresse de retour a la fin de la gestion de l'IT
		call rax
		mov qword [rsp+136],   rax

		mov rax,  qword _EndMagicFunction ; sauvegarde de l'adresse de la fonction de restauration de contexte
		mov qword [rsp],   rax
		ret

%else
BITS 32
		pushf                      ; placement en pile de la valeur des flags cpu
		mov dword [esp-16],  eax   ; sauvegarde du registre eax
		mov dword eax, [esp]       ; déplacement de la sauvegarde de flags dans la pile
		mov dword [esp-4],  eax
		mov eax ,dword [esp-16]    ; sauvegarde du registre eax
		popf                       ; supression de la sauvegarde des flags cpu

		sub esp,40                 ; on fait un peu de place pour sauvegarder les infos
		
		mov dword [esp+4],  eax    ; sauvegarde du registre eax
		mov dword [esp+8],  ebx    ; sauvegarde du registre ebx
		mov dword [esp+12], ecx    ; sauvegarde du registre ecx
		mov dword [esp+16], edx    ; sauvegarde du registre edx
		mov dword [esp+20], esi    ; sauvegarde du registre esi
		mov dword [esp+24], edi    ; sauvegarde du registre edi
		mov dword [esp+28], ebp    ; sauvegarde du registre ebp
		; ESP+32 ==> Flags

		mov eax,  dword _getBuffer_FPU  ; restauration des registres FPU
		call eax
		fxsave  [eax]

		mov eax,  dword _getTrueEIP ; sauvegarde de l'adresse de retour a la fin de la gestion de l'IT
		call eax
		mov dword [esp+36],   eax

		mov eax,  dword _EndMagicFunction ; sauvegarde de l'adresse de la fonction de restauration de contexte
		mov dword [esp],   eax
	
		ret
%endif


_EndMagicFunction :
%ifdef MODE_64
BITS 64
		call _restauration

		mov rax,  qword _getBuffer_FPU  ; restauration des registres FPU
		call rax
		fxrstor64  [rax]

		mov rax, qword [rsp+0]    
		mov rbx, qword [rsp+8]  
		mov rcx, qword [rsp+16]   
		mov rdx, qword [rsp+24]   
		mov rsi, qword [rsp+32]   
		mov rdi, qword [rsp+40]   
		mov r8,  qword [rsp+48]   
		mov r9,  qword [rsp+56]  
		mov r10, qword [rsp+64]  
		mov r11, qword [rsp+72]  
		mov r12, qword [rsp+80]  
		mov r13, qword [rsp+88]  
		mov r14, qword [rsp+96]  
		mov r15, qword [rsp+104]  
		mov rbp, qword [rsp+112]  
		add rsp,120
		popfq
		ret
%else
BITS 32
		call _restauration

		mov eax,  dword _getBuffer_FPU  ; restauration des registres FPU
		call eax
		fxrstor  [eax]

		mov eax, dword [esp+0]    ; sauvegarde du registre eax
		mov ebx, dword [esp+4]    ; sauvegarde du registre ebx
		mov ecx, dword [esp+8]    ; sauvegarde du registre ecx
		mov edx, dword [esp+12]   ; sauvegarde du registre edx
		mov esi, dword [esp+16]   ; sauvegarde du registre esi
		mov edi, dword [esp+20]   ; sauvegarde du registre edi
		mov ebp, dword [esp+24]   ; sauvegarde du registre ebp
		add esp,28
		popf
		ret
%endif



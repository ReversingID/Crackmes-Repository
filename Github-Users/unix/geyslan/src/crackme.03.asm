;; Crackme.03.32 - Assembly Language - Linux/x86
;; Copyright (C) 2013 Geyslan G. Bem, Hacking bits
;;
;;   http://hackingbits.com
;;   geyslan@gmail.com
;;
;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program. If not, see <http://www.gnu.org/licenses/>.


;;   crackme.03.32
;;
;;   Created based on teensy ELF (Handcrafted ELF Program Header)
;;   http://www.muppetlabs.com/~breadbox/software/tiny/teensy.html
;;
;;   # nasm -f bin crackme.03.32.asm
;;   # chmod +x crackme.03.32
;;   # ./crackme.03.32


BITS 32
  
	org 0x00010000

	db 0x7F, "ELF"			; e_ident
	dd 1						; p_type
	dd 0						; p_offset
	dd $$						; p_vaddr
	dw 2				; e_type	; p_paddr
	dw 3				; e_machine
	dd _start			; e_version	; p_filesz
	dd _start			; e_entry	; p_memsz
	dd 4				; e_phoff	; p_flags
	;; Entry point is inside the Handcrafted ELF Header! ;)
_start:
	dw 0x2ab3			; e_shoff	; p_align
	dw 0xc031
	db 0x40				; e_flags
	jmp short checksum
	db 0
	dw 0x34				; e_ehsize
	dw 0x20				; e_phentsize
	db 1				; e_phnum
					; e_shentsize
					; e_shnum
					; e_shstrndx
	db 0x00
cryptedkey:

	;; First checksum encrypted key
	;; dw 0x503 << 2 = 0x140c
	dw 0x140c
cryptedstring:

	;; https://github.com/geyslan/SLAE/blob/master/7th.assignment/uzumaki_crypter.py
	;; ./uzumaki_crypter.py -x ac -a 09 -s $'\x90\x4f\x6d\x65\x64\x65\x74\x6f\x75'
	db 0x90,0x7c,0x97,0xad,0xb6,0xb6,0xc6,0xc0,0xbf
checksum:

	;; ELF Header Checksum
	sub ecx, ecx
	mov ecx, $$
	xor edx, edx
	xor ebx, ebx

checkloop:	
	mov bl, byte [ecx]
	add edx, ebx
	inc ecx

	cmp ecx, cryptedkey
	jne checkloop

	shl edx, 2
	cmp dx, word [cryptedkey]
	jne exit

	xor ebp, ebp
	mov edi, edx
	inc ebp
	mov eax, 0x8010
	inc ebp
	mul ebp
	xchg eax, esi
	mov eax, esi
	sub ax, word [$$+20]
	jnz exit
	
	;; 0x10020 - 0x140c
	sub esi, edi
	
	;; XORing the rest of the subtracting of the entry offset by cryptedkey
	xor si, 0xec14
	jnz exit

	jmp $+3
	db 0xd4
	
	xor eax, eax

	;; First change has to be made here
	;; jz exitsucess
	jnz exitsucess

exit:
	sub edx, edx
	jz $+58
	strexit: db "Try to find the string of success and make me print it.", 0xA

	mov eax, 4
	mov ebx, 1
	mov ecx, strexit
	mov edx, 56
	int 0x80

getout:	
	mov eax, 1
	mov ebx, 0
	int 0x80

exitsucess:
	
	xor edx, edx
	push checksum
	sub word [esp], 11
	pop esi
	lea esi, [esi + 1]
		
	sub ecx, ecx
	jnz exitsucess
	
stackloop:
	inc esi
	jmp $+3
	db 0xc3
	mov dl, byte [esi]
	mov byte [esp + ecx], dl
	inc ecx
	cmp ecx, 9
	jne stackloop

	sub edx, edx
	xor ecx, ecx

decryptloop:
	inc ecx
	mov dl, byte [esp + ecx]
	sub dl, 9
	xor dl, 0xac
	jmp $+4
	dw 0x41e8
	xor dl, byte [esp + ecx - 1]
	mov byte [esp + ecx], dl
	cmp ecx, 8
	jne decryptloop

	inc ecx
	mov byte [esp + ecx], 0xA

	dec ecx
	xchg edx, ecx
	inc edx
	inc esp
	jmp $+3
	db 0xe8	
	mov eax, 4
	mov ebx, 1
	mov ecx, esp


	;; Checksum for the entire binary (minus key2)
        ;; # xxd -c 1 crackme.03.32 | awk '{$1="";$3=""; print}' | awk '{total = total + strtonum("0x" $1)}END{print total}'
        ;; ;)

	pushad

	xor ecx, ecx
	push ecx
	mov ecx, $$
	pop edx
	mov ebx, edx

checkloop2:
	mov bl, byte [ecx]
	add edx, ebx
	inc ecx

	cmp ecx, key2
	jne checkloop2

	jmp $+3
	db 0xcd
	cmp dx, word [key2]

	;; Second change could be made here
	;; je exit
	jne exit

	popad

	
	jmp $+3
	db 0xc9
	int 0x80

	jmp getout
key2:

	;; Second checksum key
	;; Second change could be made here
	;; dw 0x7f6c
	dw 0x7f6d

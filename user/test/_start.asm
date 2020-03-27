extern main
[bits 32]
[section .text]

global _start
_start:
	;jmp $
	push ebx
	push ecx
	call main 
	push eax
	jmp $

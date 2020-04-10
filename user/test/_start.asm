extern main
extern exit	

[bits 32]
[section .text]

global _start
_start:
	push ebx
	push ecx
	call main 
	push eax
	call exit
	jmp $

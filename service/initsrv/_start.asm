extern main
extern exit	
extern thread_exit	

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

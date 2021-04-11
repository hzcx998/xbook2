; program entry

extern _enter_preload
extern exit	
extern main	

[bits 32]
[section .text]

global _start
_start:
    ; save arg
    push edx
	push ebx
	push ecx
	call _enter_preload
    ;restore
    pop ecx
    pop ebx
    pop edx

    ; put arg on stack
    push edx
    push ebx
    push ecx
    ; call main func
    call main
    ; call _exit
	push eax    ; exit status
	call exit
    ; should never be here.
	jmp $
; *** end of program ***
; program entry

extern _enter_preload
extern _exit	
extern main	

[bits 32]
[section .text]

global _start
_start:
    ; save arg
	push ebx
	push ecx
	call _enter_preload
    ;restore
    pop ecx
    pop ebx

    ; put arg on stack
    push ebx
    push ecx
    ; call main func
    call main
    ; call _exit
	push eax    ; exit status
	call _exit
    ; should never be here.
	jmp $
; *** end of program ***
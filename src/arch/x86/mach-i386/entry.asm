%include "arch/const.inc"

[bits 32]
[section .text]

extern kernel_main
extern arch_init

global kernel_start
;这个标签是整个内核的入口，从loader跳转到这儿
kernel_start:
	mov byte [0x800b8000+160*4+0], 'K'
	mov byte [0x800b8000+160*4+1], 0X07
	mov byte [0x800b8000+160*4+2], 'E'
	mov byte [0x800b8000+160*4+3], 0X07
	mov byte [0x800b8000+160*4+4], 'R'
	mov byte [0x800b8000+160*4+5], 0X07
	mov byte [0x800b8000+160*4+6], 'N'
	mov byte [0x800b8000+160*4+7], 0X07
	mov byte [0x800b8000+160*4+8], 'E'
	mov byte [0x800b8000+160*4+9], 0X07
	mov byte [0x800b8000+160*4+10], 'L'
	mov byte [0x800b8000+160*4+11], 0X07
	
	mov ax, 0x10	;the data 
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov gs, ax 
	mov ss, ax 
	mov esp, KERNEL_STACK_TOP

	call arch_init
	call kernel_main
kernel_stop:
	hlt
	jmp kernel_stop
jmp $	

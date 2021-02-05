
%include "const.inc"
extern setup_entry

[bits 32]
[section .text]
global _start
;切换到保护模式后会跳转到这里
_start:
	mov ax, 0x10	;the data 
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov gs, ax 
	mov ss, ax 
	mov esp, LOADER_STACK_TOP
	
	mov byte [0xb8000+160*2+0], 'P'
	mov byte [0xb8000+160*2+1], 0X07
	mov byte [0xb8000+160*2+2], 'R'
	mov byte [0xb8000+160*2+3], 0X07
	mov byte [0xb8000+160*2+4], 'O'
	mov byte [0xb8000+160*2+5], 0X07
	mov byte [0xb8000+160*2+6], 'T'
	mov byte [0xb8000+160*2+7], 0X07
	mov byte [0xb8000+160*2+8], 'E'
	mov byte [0xb8000+160*2+9], 0X07
	mov byte [0xb8000+160*2+10], 'C'
	mov byte [0xb8000+160*2+11], 0X07
	mov byte [0xb8000+160*2+12], 'T'
	mov byte [0xb8000+160*2+13], 0X07
	mov byte [0xb8000+160*2+14], 'S'
	mov byte [0xb8000+160*2+15], 0X07

	jmp setup_entry

global outb
outb:	; void outb(unsigned int port, unsigned int data);
	push edx
    mov edx, [esp+8]
	mov	al, [esp+12]
	out	dx, al
	pop edx
    ret

global enable_paging
enable_paging:
   	mov eax, [esp + 4]
    mov cr3, eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret	
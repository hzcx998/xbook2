
%include "const.inc"
extern setup_entry

[bits 32]
[section .text]
global _start
;切换到保护模式后会跳转到这里
_start:
	;init all segment registeres
	;初始化保护模式下的段描述符，此时访问内存方式已经改变
	
	;这个地方的0x10是选择子，不是段
	;选择子的格式是这样的
	;|3~15      |2 |0~1		|
	;|描述符索引 |TI|RPL	 |
	;0x10的解析是
	;|2         |0b|00b     |
	;也及时说RPL为0，及要访问的段的特权级为0
	;TI为0，也就是说在GDT中获取描述符，TI为1时，是在IDT中获取描述符。
	;索引为2，也就是第三个描述符，根据GDT可知，他是一个数据段
	;=============
	;也就是说，我们使用了数据段段
	mov ax, 0x10	;the data 
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov gs, ax 
	mov ss, ax 
	mov esp, LOADER_STACK_TOP
	
	;put 'P'
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
		
	; jump to c code.
	jmp setup_entry

global outb
outb:	; void outb(unsigned int port, unsigned int data);
	push edx
    mov		edx,[esp+8]
	mov		al,[esp+12]
	out		dx,al
	pop edx
    ret

global enable_paging
enable_paging:
   	mov eax , [esp + 4]
    mov cr3,eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret	
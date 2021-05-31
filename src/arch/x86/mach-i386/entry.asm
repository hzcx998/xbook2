%include "arch/const.inc"

[bits 32]
[section .text]

extern kernel_main
extern arch_init
extern setup_paging

global kernel_start
;这个标签是整个内核的入口，从loader跳转到这儿
kernel_start:
	mov esp, KERNEL_STACK_TOP_PHY

    mov byte [TEXT_START_ADDR_PHY+160*4+0], 'K'
	mov byte [TEXT_START_ADDR_PHY+160*4+1], 0X07
	mov byte [TEXT_START_ADDR_PHY+160*4+2], 'E'
	mov byte [TEXT_START_ADDR_PHY+160*4+3], 0X07
	mov byte [TEXT_START_ADDR_PHY+160*4+4], 'R'
	mov byte [TEXT_START_ADDR_PHY+160*4+5], 0X07
	mov byte [TEXT_START_ADDR_PHY+160*4+6], 'N'
	mov byte [TEXT_START_ADDR_PHY+160*4+7], 0X07
	mov byte [TEXT_START_ADDR_PHY+160*4+8], 'E'
	mov byte [TEXT_START_ADDR_PHY+160*4+9], 0X07
	mov byte [TEXT_START_ADDR_PHY+160*4+10], 'L'
	mov byte [TEXT_START_ADDR_PHY+160*4+11], 0X07
    ; setup pageing, runing in virtual address
    call setup_paging

%ifndef GDB
    ; set eip & esp to high addr
    xor ebp, ebp
    jmp .virtual_addr + KERN_BASE_VIR_ADDR
.virtual_addr:   
%endif

    mov esp, KERNEL_STACK_TOP_VIR
    
    mov byte [TEXT_START_ADDR_VIR+160*5+0], 'P'
	mov byte [TEXT_START_ADDR_VIR+160*5+1], 0X07
	mov byte [TEXT_START_ADDR_VIR+160*5+2], 'A'
	mov byte [TEXT_START_ADDR_VIR+160*5+3], 0X07
	mov byte [TEXT_START_ADDR_VIR+160*5+4], 'G'
	mov byte [TEXT_START_ADDR_VIR+160*5+5], 0X07
	mov byte [TEXT_START_ADDR_VIR+160*5+6], 'E'
	mov byte [TEXT_START_ADDR_VIR+160*5+7], 0X07


	call arch_init
	call kernel_main
kernel_stop:
	hlt
	jmp kernel_stop
jmp $	

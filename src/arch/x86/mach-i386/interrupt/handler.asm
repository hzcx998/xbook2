%include "arch/const.inc"

extern interrupt_handlers		 
extern interrupt_do_irq		 
extern softirq_handle_in_interrupt
extern syscalls
extern exception_check
extern syscall_check
extern syscall_dispatch

[bits 32]
[section .text]
EXCEPTION_ENTRY 0x00,NO_ERROR_CODE
EXCEPTION_ENTRY 0x01,NO_ERROR_CODE
EXCEPTION_ENTRY 0x02,NO_ERROR_CODE
EXCEPTION_ENTRY 0x03,NO_ERROR_CODE 
EXCEPTION_ENTRY 0x04,NO_ERROR_CODE
EXCEPTION_ENTRY 0x05,NO_ERROR_CODE
EXCEPTION_ENTRY 0x06,NO_ERROR_CODE
EXCEPTION_ENTRY 0x07,NO_ERROR_CODE 
EXCEPTION_ENTRY 0x08,ERROR_CODE
EXCEPTION_ENTRY 0x09,NO_ERROR_CODE
EXCEPTION_ENTRY 0x0a,ERROR_CODE
EXCEPTION_ENTRY 0x0b,ERROR_CODE 
EXCEPTION_ENTRY 0x0c,NO_ERROR_CODE
EXCEPTION_ENTRY 0x0d,ERROR_CODE
EXCEPTION_ENTRY 0x0e,ERROR_CODE
EXCEPTION_ENTRY 0x0f,NO_ERROR_CODE 
EXCEPTION_ENTRY 0x10,NO_ERROR_CODE
EXCEPTION_ENTRY 0x11,ERROR_CODE
EXCEPTION_ENTRY 0x12,NO_ERROR_CODE
EXCEPTION_ENTRY 0x13,NO_ERROR_CODE 
EXCEPTION_ENTRY 0x14,NO_ERROR_CODE
EXCEPTION_ENTRY 0x15,NO_ERROR_CODE
EXCEPTION_ENTRY 0x16,NO_ERROR_CODE
EXCEPTION_ENTRY 0x17,NO_ERROR_CODE 
EXCEPTION_ENTRY 0x18,ERROR_CODE
EXCEPTION_ENTRY 0x19,NO_ERROR_CODE
EXCEPTION_ENTRY 0x1a,ERROR_CODE
EXCEPTION_ENTRY 0x1b,ERROR_CODE 
EXCEPTION_ENTRY 0x1c,NO_ERROR_CODE
EXCEPTION_ENTRY 0x1d,ERROR_CODE
EXCEPTION_ENTRY 0x1e,ERROR_CODE
EXCEPTION_ENTRY 0x1f,NO_ERROR_CODE 
INTERRUPT_ENTRY 0x20,NO_ERROR_CODE	;时钟中断对应的入口
INTERRUPT_ENTRY 0x21,NO_ERROR_CODE	;键盘中断对应的入口
INTERRUPT_ENTRY 0x22,NO_ERROR_CODE	;级联用的
INTERRUPT_ENTRY 0x23,NO_ERROR_CODE	;串口2对应的入口
INTERRUPT_ENTRY 0x24,NO_ERROR_CODE	;串口1对应的入口
INTERRUPT_ENTRY 0x25,NO_ERROR_CODE	;并口2对应的入口
INTERRUPT_ENTRY 0x26,NO_ERROR_CODE	;软盘对应的入口
INTERRUPT_ENTRY 0x27,NO_ERROR_CODE	;并口1对应的入口
INTERRUPT_ENTRY 0x28,NO_ERROR_CODE	;实时时钟对应的入口
INTERRUPT_ENTRY 0x29,NO_ERROR_CODE	;重定向
INTERRUPT_ENTRY 0x2a,NO_ERROR_CODE	;保留
INTERRUPT_ENTRY 0x2b,NO_ERROR_CODE	;保留
INTERRUPT_ENTRY 0x2c,NO_ERROR_CODE	;ps/2鼠标
INTERRUPT_ENTRY 0x2d,NO_ERROR_CODE	;fpu浮点单元异常
INTERRUPT_ENTRY 0x2e,NO_ERROR_CODE	;硬盘
INTERRUPT_ENTRY 0x2f,NO_ERROR_CODE	;保留

;系统调用中断
[bits 32]
[section .text]

global syscall_handler
syscall_handler:
	;1 保存上下文环境
   	push 0			    ; 压入0, 使栈中格式统一

   	push ds
   	push es
   	push fs
   	push gs
   	pushad			    ; PUSHAD指令压入32位寄存器，其入栈顺序是:
				    	; EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI 
    push eax
    mov ax, ss
	mov ds, ax
	mov es, ax
    mov fs, ax
	mov gs, ax
    pop eax
    
   	push 0x40			; 此位置压入0x40也是为了保持统一的栈格式
    
    sti

    ; check syscall num
    push eax
    call syscall_check
    cmp eax, 1
    je .bad_syscall
    pop eax
    
    push esp
    call syscall_dispatch
    add esp, 4
    mov [esp + 8*4], eax	

.check_exception:
    push esp
    call exception_check
    add esp, 4
    cli
    jmp interrupt_exit

.bad_syscall:
    pop eax
    jmp .check_exception

global interrupt_exit
interrupt_exit:
    add esp, 4			   ; 跳过中断号
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4			   ; 跳过error_code
    iretd
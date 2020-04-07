
%include "const.inc"

;intr_handler_table是C中注册的中断处理程序数组
extern intr_handler_table		 
extern do_irq		 
extern do_softirq	

extern current_trap_frame
extern reenter

;extern DoSignal

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

extern dump_value
;系统调用中断
[bits 32]
[section .text]
extern do_usrmsg ; 位于src/kernel/usrmsg.c中
global kern_usrmsg_handler
kern_usrmsg_handler:
	;1 保存上下文环境
   	push 0			    ; 压入0, 使栈中格式统一

   	push ds
   	push es
   	push fs
   	push gs
   	pushad			    ; PUSHAD指令压入32位寄存器，其入栈顺序是:
				    	; EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI 
    mov dx,ss
	mov ds, dx
	mov es, dx
    
   	push 0x40			; 此位置压入0x40也是为了保持统一的栈格式
   
    ;2 传递参数给消息处理
    push esp                ; 传入栈指针，可以用来获取所有陷阱栈框寄存器
    push ebx			    ; 用户消息中消息参数
	;调用用户消息处理
   	call do_usrmsg
    add esp, 8			    ; 跨过上面的2个参数

	;3 将call调用后的返回值存入待当前内核栈中eax的位置
    mov [esp + 8*4], eax
    
    ; 处理完用户消息后再进行信号处理，因为处理过程可能会影响到寄存器的值
    ;4 signal
.do_signal:
    ;push esp         ; 把中断栈指针传递进去
    ;call do_signal
    ;add esp, 4
    
    jmp intr_exit		    ; intr_exit返回,恢复上下文

global intr_exit

; 跳转到用户态执行的切换
global __switch_to_user
__switch_to_user:
    mov esp, [esp + 4]  ; process stack
    
intr_exit:
; 以下是恢复上下文环境
    add esp, 4			   ; 跳过中断号
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4			   ; 跳过error_code
    iretd

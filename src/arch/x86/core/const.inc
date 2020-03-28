KERNEL_STACK_TOP EQU 0x8009f000

EOI EQU 0X20
INT_M_CTL	equ	0x20	; I/O port for interrupt controller         <Master>
INT_M_CTLMASK	equ	0x21	; setting bits in this port disables ints   <Master>
INT_S_CTL	equ	0xA0	; I/O port for second interrupt controller  <Slave>
INT_S_CTLMASK	equ	0xA1	; setting bits in this port disables ints   <Slave>

; 若在相关的异常中cpu已经自动压入了错误码,为保持栈中格式统一,这里不做操作.
%define   ERROR_CODE   nop
; 若在相关的异常中cpu没有压入错误码,为了统一栈中格式,就手工压入一个0
%define   NO_ERROR_CODE push 0

;中断的汇编处理部分函数宏定义
%macro EXCEPTION_ENTRY 2
global exception_entry%1
exception_entry%1:		 ; 每个中断处理程序都要压入中断向量号,所以一个中断类型一个中断处理程序，自己知道自己的中断向量号是多少
    %2				 ; 中断若有错误码会压在eip后面 
; 以下是保存上下文环境
    push ds
    push es
    push fs
    push gs
    pushad			 ; PUSHAD指令压入32位寄存器,其入栈顺序是: EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI

    mov dx,ss
	mov ds, dx
	mov es, dx

    push %1			 ; 不管idt_table中的目标程序是否需要参数,都一律压入中断向量号,调试时很方便
    
    push esp         ; 把中断栈指针传递进去
    call [intr_handler_table + %1*4]       ; 调用idt_table中的C版本中断处理函数
    add esp, 4
    
    ; signal
    push esp         ; 把中断栈指针传递进去
    ;call DoSignal
    add esp, 4

    jmp intr_exit

%endmacro

;中断的汇编处理部分函数宏定义
; 中断处理过程中会关闭中断。
%macro INTERRUPT_ENTRY 2
global irq_entry%1
irq_entry%1:		 ; 每个中断处理程序都要压入中断向量号,所以一个中断类型一个中断处理程序，自己知道自己的中断向量号是多少

    %2				 ; 中断若有错误码会压在eip后面 
; 以下是保存上下文环境
    push ds
    push es
    push fs
    push gs
    pushad			 ; PUSHAD指令压入32位寄存器,其入栈顺序是: EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI

    mov dx,ss
	mov ds, dx
	mov es, dx

   ; 如果是从片上进入的中断,除了往从片上发送EOI外,还要往主片上发送EOI 

    push %1			 ; 不管idt_table中的目标程序是否需要参数,都一律压入中断向量号,调试时很方便
    
    push esp        ; 把中断栈指针传递进去  
    ; 调用DoIRQ，通过传入的参数获取irq值
    call do_irq
    add esp, 4
    
    ; softirq
    ;call do_softirq

    ; signal
    ;push esp         ; 把中断栈指针传递进去
    ;call DoSignal
    ;add esp, 4
    jmp intr_exit

%endmacro


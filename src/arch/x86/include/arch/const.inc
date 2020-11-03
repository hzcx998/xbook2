KERNEL_STACK_TOP EQU 0x8009f000

EOI             EQU 0X20
INT_M_CTL	    EQU	0x20	; I/O port for interrupt controller         <Master>
INT_M_CTLMASK	EQU	0x21	; setting bits in this port disables ints   <Master>
INT_S_CTL	    EQU	0xA0	; I/O port for second interrupt controller  <Slave>
INT_S_CTLMASK	EQU	0xA1	; setting bits in this port disables ints   <Slave>

%define   ERROR_CODE   nop
%define   NO_ERROR_CODE push 0

%macro EXCEPTION_ENTRY 2
global exception_entry%1
exception_entry%1:
    %2				 ; 中断若有错误码会压在eip后面 

    push ds
    push es
    push fs
    push gs
    pushad			 ; PUSHAD指令压入32位寄存器,其入栈顺序是: EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI

    mov dx,ss
	mov ds, dx
	mov es, dx
    mov fs, dx
	mov gs, dx

    push %1
    
    push esp
    call [interrupt_handlers + %1*4]
    add esp, 4
    
    push esp
    call interrupt_do_trigger
    add esp, 4
    
    jmp interrupt_exit
%endmacro

%macro INTERRUPT_ENTRY 2
global irq_entry%1
irq_entry%1:

    %2 
    push ds
    push es
    push fs
    push gs
    pushad

    mov dx,ss
	mov ds, dx
	mov es, dx
    mov fs, dx
	mov gs, dx
    
    push %1
    push esp
    call interrupt_do_irq
    add esp, 4
    
    call softirq_handle_in_interrupt

    push esp
    call interrupt_do_trigger
    add esp, 4
    jmp interrupt_exit

%endmacro


[section .text]
[bits 32]

global ioport_in8
ioport_in8:
    push edx
	mov		edx,[esp+8]
	xor		eax,eax
	in		al,dx
	pop edx
    ret

global ioport_in16
ioport_in16:
    push edx
	mov		edx,[esp+8]
	xor		eax,eax
	in		ax,dx
	pop edx
    ret

global ioport_in32
ioport_in32:
	push edx
    mov		edx,[esp+8]
	in		eax,dx
	pop edx
    ret

global ioport_out8
ioport_out8:
    push edx
    mov		edx,[esp+8]
	mov		al,[esp+12]
	out		dx,al
	pop edx
    ret

global ioport_out16
ioport_out16:
    push edx
    mov		edx,[esp+8]
	mov		ax,[esp+12]
	out		dx,ax
	pop edx
    ret	

global ioport_out32
ioport_out32:
    push edx
    mov		edx,[esp+8]
	mov		eax,[esp+12]
	out		dx,eax
	pop edx
    ret

global interrupt_disable
interrupt_disable:
	cli
	ret

global interrupt_enable
interrupt_enable:
	sti
	ret
	
global cpu_do_nothing
cpu_do_nothing:
	nop
	ret

global task_register_set
task_register_set:
	ltr	[esp+4]
	ret
	
global cpu_cr2_read
cpu_cr2_read:
	mov eax,cr2
	ret

global cpu_cr3_read
cpu_cr3_read:
	mov eax,cr3
	ret

global cpu_cr3_write 
cpu_cr3_write:
	mov eax,[esp+4]
	mov cr3,eax
	ret

global cpu_cr0_read
cpu_cr0_read:
	mov eax,cr0
	ret

global cpu_cr0_write
cpu_cr0_write:
	mov eax,[esp+4]
	mov cr0,eax
	ret	
	
global gdt_register_get 
gdt_register_get:
	mov eax, [esp + 4]
	sgdt [eax]
	ret

global gdt_register_set
gdt_register_set:
	mov ax, [esp + 4]
	mov	[esp+6],ax		
	lgdt [esp+6]
	
    ; flush segment registers
    mov ax, 0x10
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov ss, ax 
	mov gs, ax 
	
	jmp dword 0x08: .l
.l:
	ret

global idt_register_get
idt_register_get:
	mov eax, [esp + 4]
	sidt [eax]
	ret

global idt_register_set
idt_register_set:
	mov		ax,[esp+4]
	mov		[esp+6],ax
	lidt	[esp+6]
	ret

global eflags_save_to
eflags_save_to:
	pushf
	pop		eax
	ret

global eflags_restore_from
eflags_restore_from:
	mov		eax,[ESP+4]
	push	eax
	popfd
	ret

global ioport_read_bytes
ioport_read_bytes:
	mov	edx, [esp + 4]		; port
	mov	edi, [esp + 4 + 4]	; buf
	mov	ecx, [esp + 4 + 4 + 4]	; n
	shr	ecx, 1		;ecx/2
	cld
	rep	insw
	ret

global ioport_write_bytes 
ioport_write_bytes:
	mov	edx, [esp + 4]		; port
	mov	esi, [esp + 4 + 4]	; buf
	mov	ecx, [esp + 4 + 4 + 4]	; n
	shr	ecx, 1	;ecx/2
	cld
	rep	outsw
	ret

global cpu_do_sleep 
cpu_do_sleep: ;void cpu_do_sleep();
	hlt
	ret

global mem_xchg8
; char mem_xchg8(char *ptr, char value);
mem_xchg8:
    push ebx
    
    mov eax, [esp + 4 + 4]      ; ptr  
    mov bl, [esp + 4 + 8]       ; value
    xchg [eax], bl     ; 8 bits
    xor eax, eax    ; eax = 0
    mov al, bl      ; al == old *ptr

    pop ebx
    ret

global mem_xchg16
; short mem_xchg16(short *ptr, short value);
mem_xchg16:
    push ebx
    
    mov eax, [esp + 4 + 4]      ; ptr  
    mov bx, [esp + 4 + 8]       ; value
    xchg [eax], bx     ; 16 bits

    xor eax, eax    ; eax = 0
    mov ax, bx      ; ax == old *ptr

    pop ebx
    ret

global mem_xchg32
; int mem_xchg32(int *ptr, int value);
mem_xchg32:
    push ebx
    
    mov eax, [esp + 4 + 4]      ; ptr  
    mov bx, [esp + 4 + 8]       ; value
    xchg [eax], ebx     ; 32 bits

    mov eax, ebx      ; eax == old *ptr

    pop ebx
    ret
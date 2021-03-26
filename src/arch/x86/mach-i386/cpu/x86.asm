[section .text]
[bits 32]
	
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
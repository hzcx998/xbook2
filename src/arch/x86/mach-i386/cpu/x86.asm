[section .text]
[bits 32]

global __in8
__in8:	;uint8_t __in8(unsigned int port);
	mov		edx,[esp+4]
	xor		eax,eax
	in		al,dx
	ret

global __in16
__in16:	;uint16_t __in16(unsigned int port);
	mov		edx,[esp+4]
	xor		eax,eax
	in		ax,dx
	ret

global __in32
__in32:	;unsigned int __in32(unsigned int port);
	mov		edx,[esp+4]
	in		eax,dx
	ret

global __out8
__out8:	; void __out8(unsigned int port, unsigned int data);
	mov		edx,[esp+4]
	mov		al,[esp+8]
	out		dx,al
	ret

global __out16
__out16:	; void __out16(unsigned int port, unsigned int data);
	mov		edx,[esp+4]
	mov		ax,[esp+8]
	out		dx,ax
	ret	

global __out32
__out32:	; void __out32(unsigned int port, unsigned int data);
	mov		edx,[esp+4]
	mov		eax,[esp+8]
	out		dx,eax
	ret

global __disable_intr
__disable_intr:	; void __disable_intr(void);
	cli
	ret

global __enable_intr
__enable_intr:	; void __enable_intr(void);
	sti
	ret
	
global __cpu_idle
__cpu_idle: ;void __cpu_idle(void);
	nop
	ret

global load_tr
load_tr:		; void load_tr(unsigned int tr);
	ltr	[esp+4]			; tr
	ret
	
global read_cr2
read_cr2:
	mov eax,cr2
	ret

global read_cr3
read_cr3:
	mov eax,cr3
	ret

global write_cr3 
write_cr3:
	mov eax,[esp+4]
	mov cr3,eax
	ret

global read_cr0
read_cr0:
	mov eax,cr0
	ret

global write_cr0
write_cr0:
	mov eax,[esp+4]
	mov cr0,eax
	ret	
	
global store_gdtr 
store_gdtr:
	mov eax, [esp + 4]
	sgdt [eax]
	ret

global load_gdtr
load_gdtr:	;void load_gdtr(unsigned int limit, unsigned int addr);
	mov ax, [esp + 4]
	mov	[esp+6],ax		
	lgdt [esp+6]
	
    mov ax, 0x10
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov ss, ax 
	mov gs, ax 
	
	jmp dword 0x08: .l
.l:
	ret

global store_idtr
store_idtr:
	mov eax, [esp + 4]
	sidt [eax]
	ret

global load_idtr
load_idtr:	;void load_reg_idtr(unsigned int limit, unsigned int addr);
	mov		ax,[esp+4]
	mov		[esp+6],ax
	lidt	[esp+6]
	ret

global load_eflags
load_eflags:	; unsigned int load_eflags(void);
	pushf		; PUSH eflags
	pop		eax
	ret

global store_eflags
store_eflags:	; void store_eflags(unsigned int eflags);
	mov		eax,[ESP+4]
	push	eax
	popfd		; POP eflags
	ret

global __io_read
__io_read:	;void __io_read(u16 port, void* buf, unsigned int n);
	mov	edx, [esp + 4]		; port
	mov	edi, [esp + 4 + 4]	; buf
	mov	ecx, [esp + 4 + 4 + 4]	; n
	shr	ecx, 1		;ecx/2
	cld
	rep	insw
	ret

global __io_write 
__io_write:	;void __io_write(u16 port, void* buf, unsigned int n);
	mov	edx, [esp + 4]		; port
	mov	esi, [esp + 4 + 4]	; buf
	mov	ecx, [esp + 4 + 4 + 4]	; n
	shr	ecx, 1	;ecx/2
	cld
	rep	outsw
	ret


global __invlpg
__invlpg:
	mov eax, [esp + 4]
	invlpg [eax]
	ret	

global __cpuid
__cpuid: 	; void __cpuid(unsigned int id_eax, unsigned int *eax, 
			; 		unsigned int *ebx, unsigned int *ecx, unsigned int *edx);
	mov eax, [esp + 4] ; eax_id
	cpuid
	mov edi, [esp + 8]	;eax
	mov [edi], eax
	mov edi, [esp + 12]	;ebx
	mov [edi], ebx
	mov edi, [esp + 16]	;ecx
	mov [edi], ecx
	mov edi, [esp + 20]	;edx
	mov [edi], edx
	ret

global __cpu_lazy 
__cpu_lazy: ;void __cpu_lazy();
	hlt
	ret

global __xchg8
; char __xchg8(char *ptr, char value);
__xchg8:
    push ebx
    
    mov eax, [esp + 4 + 4]      ; ptr  
    mov bl, [esp + 4 + 8]       ; value
    xchg [eax], bl     ; 8 bits
    xor eax, eax    ; eax = 0
    mov al, bl      ; al == old *ptr

    pop ebx
    ret

global __xchg16
; short __xchg16(short *ptr, short value);
__xchg16:
    push ebx
    
    mov eax, [esp + 4 + 4]      ; ptr  
    mov bx, [esp + 4 + 8]       ; value
    xchg [eax], bx     ; 16 bits

    xor eax, eax    ; eax = 0
    mov ax, bx      ; ax == old *ptr

    pop ebx
    ret

global __xchg32
; int __xchg32(int *ptr, int value);
__xchg32:
    push ebx
    
    mov eax, [esp + 4 + 4]      ; ptr  
    mov bx, [esp + 4 + 8]       ; value
    xchg [eax], ebx     ; 32 bits

    mov eax, ebx      ; eax == old *ptr

    pop ebx
    ret
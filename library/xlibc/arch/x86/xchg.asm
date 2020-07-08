[section .text]
[bits 32]

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
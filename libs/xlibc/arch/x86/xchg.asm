[section .text]
[bits 32]

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
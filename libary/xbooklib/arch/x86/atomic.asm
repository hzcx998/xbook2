[section .text]
[bits 32]
;lock 锁定的是内存地址，所以操作对象值必须时内存才行

; void __atomic_add(int *a, int b);
global __atomic_add
__atomic_add:
    push ebx
	mov eax, [esp + 4 + 4]	; a
	mov ebx, [esp + 4 + 8]	; b
	; 通过lock前缀，让运算时原子运算
	lock add [eax], ebx	; *a += b
	pop ebx
    ret
; void __atomic_sub(int *a, int b);
global __atomic_sub
__atomic_sub:
    push ebx
	mov eax, [esp + 4 + 4]	; a
	mov ebx, [esp + 4 + 8]	; b
	; 通过lock前缀，让运算时原子运算
	lock sub [eax], ebx	; *a -= b
    pop ebx
	ret

; void __atomic_inc(int *a);
global __atomic_inc
__atomic_inc:
	mov eax, [esp + 4]	; a
	; 通过lock前缀，让运算时原子运算
	lock inc dword [eax]	; ++*a
	ret
	

; void __atomic_dec(int *a);
global __atomic_dec
__atomic_dec:
	mov eax, [esp + 4]	; a
	; 通过lock前缀，让运算时原子运算
	lock dec dword [eax]	; --*a
	ret

; void __atomic_or(int *a, int b);
global __atomic_or
__atomic_or:
    push ebx
	mov eax, [esp + 4 + 4]	; a
	mov ebx, [esp + 4 + 8]	; b
	; 通过lock前缀，让运算时原子运算
	lock or [eax], ebx	; *a |= b
    pop ebx
	ret

; void __atomic_and(int *a, int b);
global __atomic_and
__atomic_and:
    push ebx
	mov eax, [esp + 4 + 4]	; a
	mov ebx, [esp + 4 + 8]	; b
	; 通过lock前缀，让运算时原子运算
	lock and [eax], ebx	; *a &= b
    pop ebx
	ret

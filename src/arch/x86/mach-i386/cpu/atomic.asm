[section .text]
[bits 32]
;lock 锁定的是内存地址，所以操作对象值必须时内存才行
global mem_atomic_add
mem_atomic_add:
	mov eax, [esp + 4]	; a
	mov ebx, [esp + 8]	; b
	lock add [eax], ebx	; *a += b
	ret

global mem_atomic_sub
mem_atomic_sub:
	mov eax, [esp + 4]
	mov ebx, [esp + 8]
	lock sub [eax], ebx
	ret

global mem_atomic_inc
mem_atomic_inc:
	mov eax, [esp + 4]
	lock inc dword [eax]
	ret
	
global mem_atomic_dec
mem_atomic_dec:
	mov eax, [esp + 4]
	lock dec dword [eax]
	ret

global mem_atomic_or
mem_atomic_or:
	mov eax, [esp + 4]
	mov ebx, [esp + 8]
	lock or [eax], ebx
	ret

global mem_atomic_and
mem_atomic_and:
	mov eax, [esp + 4]
	mov ebx, [esp + 8]
	lock and [eax], ebx
	ret

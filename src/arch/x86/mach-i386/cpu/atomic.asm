[section .text]
[bits 32]
;lock 锁定的是内存地址，所以操作对象值必须时内存才行

global __atomic_add
__atomic_add:
	mov eax, [esp + 4]	; a
	mov ebx, [esp + 8]	; b
	lock add [eax], ebx	; *a += b
	ret

global __atomic_sub
__atomic_sub:
	mov eax, [esp + 4]
	mov ebx, [esp + 8]
	lock sub [eax], ebx
	ret


global __atomic_inc
__atomic_inc:
	mov eax, [esp + 4]
	lock inc dword [eax]
	ret
	

global __atomic_dec
__atomic_dec:
	mov eax, [esp + 4]
	lock dec dword [eax]
	ret


global __atomic_or
__atomic_or:
	mov eax, [esp + 4]
	mov ebx, [esp + 8]
	lock or [eax], ebx
	ret


global __atomic_and
__atomic_and:
	mov eax, [esp + 4]
	mov ebx, [esp + 8]
	lock and [eax], ebx
	ret

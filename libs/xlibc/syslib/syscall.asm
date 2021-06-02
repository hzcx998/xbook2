[bits 32]
[section .text]

SYSCALL_INT	EQU 0x40

; 0个参数
global __syscall0
__syscall0:
	mov eax, [esp + 4]	; eax = syscall num
	int SYSCALL_INT
	ret

; 1个参数
global __syscall1
__syscall1:
	push ebx
	mov eax, [esp + 4 + 4]	; eax = syscall num
	mov ebx, [esp + 4 + 8]	; ebx = arg0
	int SYSCALL_INT
	pop ebx
	ret

; 2个参数
global __syscall2
__syscall2:
	push ecx
	push ebx
	mov eax, [esp + 8 + 4]	; eax = syscall num
	mov ebx, [esp + 8 + 8]	; ebx = arg0
	mov ecx, [esp + 8 + 12]	; ecx = arg1
	int SYSCALL_INT
	pop ebx
	pop ecx
	ret
				
; 3个参数
global __syscall3
__syscall3:
	push edx
	push ecx
	push ebx
	mov eax, [esp + 12 + 4]	; eax = syscall num
	mov ebx, [esp + 12 + 8]	; ebx = arg0
	mov ecx, [esp + 12 + 12]	; ecx = arg1
	mov edx, [esp + 12 + 16]	; edx = arg2
	int SYSCALL_INT
	pop ebx
	pop ecx
	pop edx
	ret
; 4个参数
global __syscall4
__syscall4:
	push esi
	push edx
	push ecx
	push ebx
	mov eax, [esp + 16 + 4]	; eax = syscall num
	mov ebx, [esp + 16 + 8]	; ebx = arg0
	mov ecx, [esp + 16 + 12]	; ecx = arg1
	mov edx, [esp + 16 + 16]	; edx = arg2
	mov esi, [esp + 16 + 20]	; esi = arg3
	int SYSCALL_INT
	pop ebx
	pop ecx
	pop edx
	pop esi
	ret
; 5个参数
global __syscall5
__syscall5:
	push edi
	push esi
	push edx
	push ecx
	push ebx
	mov eax, [esp + 20 + 4]	; eax = syscall num
	mov ebx, [esp + 20 + 8]	; ebx = arg0
	mov ecx, [esp + 20 + 12]	; ecx = arg1
	mov edx, [esp + 20 + 16]	; edx = arg2
	mov esi, [esp + 20 + 20]	; esi = arg3
	mov edi, [esp + 20 + 24]	; edi = arg4
	int SYSCALL_INT
	pop ebx
	pop ecx
	pop edx
	pop esi
    pop edi
	ret
								
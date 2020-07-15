[section .text]
[bits 32]

; void __switch_to(unsigned long prev, unsigned long next);
global __switch_to
__switch_to:
	push esi
	push edi
	push ebx 
	push ebp
	
	mov eax, [esp + 20]	; 获取prev的地址
	mov [eax], esp		; 保存esp到prev->kstack
	
	mov eax, [esp + 24]	; 获取next的地址
	mov esp, [eax]		; 从next->kstack恢复esp
	
	pop ebp
	pop ebx
	pop edi
	pop esi
	ret

; 跳转到用户态执行的切换
global __switch_to_user
__switch_to_user:
    mov esp, [esp + 4]  ; process stack
; 以下是恢复上下文环境
    add esp, 4			   ; 跳过中断号
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4			   ; 跳过error_code
    iretd

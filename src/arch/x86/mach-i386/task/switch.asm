[section .text]
[bits 32]

global thread_switch_to_next
thread_switch_to_next:
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

global kernel_switch_to_user
kernel_switch_to_user:
    mov esp, [esp + 4]
    add esp, 4			   ; 跳过中断号
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4			   ; 跳过error_code
    iretd

[section .text]
[bits 32]

; void switch_to(struct Task *prev, struct Task *next);
global switch_to
switch_to:
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
	
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

; 当前任务
extern task_current
; 当前中断栈框
extern current_trap_frame
extern dump_trap_frame
extern dump_task
extern dump_value

; ESP  		= RETVAL -> ip
; ESP - 4	= esp 
; ESP - 8	= eflags 
; 生产阻塞时的临时栈，并打开中断，运行产生时钟中断，然后死循环
global make_tmp_kstack
make_tmp_kstack:
	push esp
	pushf
	
	pushad	; 通用寄存器

	push 0	; esp=kstack frame到达内核栈低端

	; save cur esp
	mov [kstack_esp], esp	; esp 指向的地址保存esp的值
	
	; 构建中断栈, 获取block_frame的地址
	mov eax, [task_current]		
	mov ebx, [eax]	; ebx = block_frame addr

	;push dword ebx
	;call dump_value
	;add esp, 4

	; 指向block frame的顶部
	add ebx, 4 * 18

	; 把栈指针放到顶部，准备往里面备份数据
	mov esp, ebx		; esp = block_frame top
	
	; 需要备份的数据的地址
	mov eax, [kstack_esp]
	add eax, 4 * 9	; 指向eflags
	
	; 备份eip, esp, eflags, 8个通用寄存器，其它寄存器已经在之前初始化
	push dword [eax + 4]	; frame->esp := esp
	push dword [eax]		; frame->eflags := eflags
	sub esp, 4				; 跳过cs
	push dword [eax + 8]	; frame->eip := eip
	sub esp, 4 * 5	; esp = &frame->gs，跳过段寄存器
	pushad			; save general register
	
	; 构建临时中断栈完成
	mov esp, [kstack_esp]	; 恢复之前的内核栈
	
	; dump trap frame
	;mov eax, [task_current]		
	;mov ebx, [eax]	; ebx = block_frame addr
	;push ebx
	;call dump_trap_frame
	;add esp, 4
	
	; 打开中断，接收响应，可以产生时钟中断
	sti
	jmp $	; 死循环，不往后面执行，当再次唤醒时，
			;已经在switch_kstack返回地址处执行，因此不会跳转到这儿来了。
	
[section .data]

; 用于临时保存交换信息的变量
kstack_esp: dd 0

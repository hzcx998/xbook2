
%include "const.inc"

org 0x90000
[bits 16]
align 16

entry:
	;初始化段和栈
	;由于从boot跳过来的时候用的jmp LOADER_SEG(0x9000):0
	;所以这个地方的cs是0x9000，其它的段也是这个值
	mov ax, cs
	mov ds, ax 
	mov ss, ax
	mov sp, 0
	mov ax, 0xb800
	mov es, ax
	
	;show 'LOADER'
	mov byte [es:160+0],'L'
	mov byte [es:160+1],0x07
	mov byte [es:160+2],'O'
	mov byte [es:160+3],0x07
	mov byte [es:160+4],'A'
	mov byte [es:160+5],0x07
	mov byte [es:160+6],'D'
	mov byte [es:160+7],0x07
	mov byte [es:160+8],'E'
	mov byte [es:160+9],0x07
	mov byte [es:160+10],'R'
	mov byte [es:160+11],0x07

	; load setup file
	call load_setup_file

    ; load kernel file from disk
	call load_kernel_file

    ;我们不再使用软盘，所以这里关闭软盘驱动器
	call kill_floppy_motor	
    
    ; get memory info
	call get_memory_info

%if CONFIG_GRAPHIC == 1
    call get_vbe_info
%endif  ; CONFIG_GRAPHIC


;保护模式设置的步骤为
;1.关闭中断，防止中间发生中断，因为保护模式中断和实模式中断的方式不一样
;2.加载gdt，保护模式进行内存访问需要用到gdt里面定义的数据结构
;3.打开A20总线，使得可以访问1MB以上的内存空间
;4.设置cr0的最低1位位1，就是切换成保护模式
;5.执行一个远跳转，清空cpu流水线
set_protect_mode:
	;close the interruption
	cli
	;load GDTR
	lgdt	[gdt_reg]
	
	;enable A20 line
	in		al,0x92
	or		al,2
	out		0x92,al
	;set CR0 bit PE
	mov		eax,cr0
	or		eax,1
	mov		cr0,eax
	
	;far jump:to clean the cs
	;这个地方的0x08是选择子，不是段
	;选择子的格式是这样的
	;|3~15      |2 |0~1		|
	;|描述符索引 |TI|RPL	 |
	;0x08的解析是
	;|1         |0b|00b     |
	;也及时说RPL为0，及要访问的段的特权级为0
	;TI为0，也就是说在GDT中获取描述符，TI为1时，是在IDT中获取描述符。
	;索引为1，也就是第二个描述符，第一个是NULL的，根据GDT可知，他是一个代码段
	;=============
	;也就是说，我们使用了代码段
	
	; 跳转到setup里面执行
	jmp	dword 0x08:flush
	
;si=LBA address, from 0
;cx=sectors
;es:dx=buffer address	
;this function was borrowed from internet
floppy_read_sectors:
	push ax 
	push cx 
	push dx 
	push bx 
	
	mov ax, si 
	xor dx, dx 
	mov bx, 18
	
	div bx 
	inc dx 
	mov cl, dl 
	xor dx, dx 
	mov bx, 2
	
	div bx 
	
	mov dh, dl
	xor dl, dl 
	mov ch, al 
	pop bx 
.1:
	mov al, 0x01
	mov ah, 0x02 
	int 0x13 
	jc .1 
	pop dx
	pop cx 
	pop ax
	ret

;ax = 写入的段偏移
;si = 扇区LBA地址
;cx = 扇区数
load_file_block:
	mov es, ax
	xor bx, bx 
.loop:
	call floppy_read_sectors
	add bx, 512
	inc si 
	loop .loop
	ret	

;don't use floppy from now on
kill_floppy_motor:
	push dx
	mov	dx, 03F2h
	mov	al, 0
	out	dx, al
	pop	dx
	ret
;检测内存
;这个方法就是获取内存信息结构体ARDS, 获取所有的信息后就跳到初始化图形
get_memory_info:
	mov ax, ARDS_SEG
	mov es, ax

	; 得到内存数
	mov	ebx, 0			; ebx = 后续值, 开始时需为 0
	mov	di, 4		; es:di 指向一个地址范围描述符结构（Address Range Descriptor Structure）
.mem_loop:
	mov	eax, 0E820h		; eax = 0000E820h
	mov	ecx, 20			; ecx = 地址范围描述符结构的大小
	mov	edx, 0534D4150h		; edx = 'SMAP'
	int	15h			; int 15h
	jc	.mem_failed
	add	di, 20
	inc	dword [es:0]	; dwMCRNumber = ARDS 的个数
	cmp	ebx, 0
	jne	.mem_loop
	jmp	.mem_ok
.mem_failed:
	mov	dword [es:0], 0
.mem_ok:

	ret 

%if CONFIG_GRAPHIC == 1

; 使用的模式
VBE_MODE	EQU	VMODE_1024_768_16

get_vbe_info:
    push ds     ; 保存数据段
    ;获取VBE信息块
    ; Input: AX=4F00H
    ;        ES:DI=储存信息块的缓冲区
    ; Output: AX=VBE return status
	mov	ax, VBE_INFO_SEG	
	mov	es, ax
	mov	di, 0
	mov	ax, VBE_CMD_VBEINFO	;检查VBE存在功能，指定ax=0x4f00
	int	0x10
    
    ;ax=0x004f 获取成功
	cmp	ax, 0x004f	
	jne	.vbe_error
    
	;检查VBE版本，必须是VBE 2.0及其以上
	mov	ax, [es:di + 4]
	cmp	ax, 0x0200      ; VBE2.0的BCD码是0x200
	jb	.vbe_error	    ; if (ax < 0x0200) goto screen_default

    ;获取画面信息， 256字节
	;cx=输入检查的模式
	
    ; 获取VBE模式
    ; Input: AX=4F01H
    ;        CX=模式号
    ;        ES:DI=储存模式块的缓冲区
    ; Output: AX=VBE return status
	mov ax, VBE_MODE_SEG
	mov es, ax
    mov	cx, VBE_MODE	;cx=模式号
	mov	ax, 0x4f01	;获取画面模式功能，指定ax=0x4f01
	int	0x10

	cmp	ax, 0x004f	;ax=0x004f 指定的这种模式可以使用
	jne	.vbe_error

%if CONFIG_GRAPHIC_SWITCH == 1
    ;切换到指定的模式
	mov	bx, VBE_MODE + 0x4000	;bx=模式号和属性
	mov	ax, 0x4f02	;切换模式模式功能，指定ax=0x4f01
	int	0x10
%endif
	;由于初始化图形模式改变了ds的值，这里设置和cs一样
	mov ax, cs
	mov ds, ax
    jmp .finish
.vbe_error:
    mov ax, 0xb800
	mov es, ax
    
    ; 第3排显示 error
    mov byte [es:160*24+0],'N'    
    mov byte [es:160*24+1],0x04
    mov byte [es:160*24+2],'O'    
    mov byte [es:160*24+3],0x04
    mov byte [es:160*24+4],' '    
    mov byte [es:160*24+5],0x04
    mov byte [es:160*24+6],'V'    
    mov byte [es:160*24+7],0x04
    mov byte [es:160*24+8],'B'    
    mov byte [es:160*24+9],0x04
    mov byte [es:160*24+10],'E'    
    mov byte [es:160*24+11],0x04
    
    hlt
    jmp $
.finish:
    pop ds      ; 恢复数据段
    ret
%endif

; load setup file
load_setup_file:
	mov ax, SETUP_SEG
	mov si, SETUP_OFF
	mov cx, SETUP_CNTS
	;调用读取一整个块的扇区数据函数，其实也就是循环读取128个扇区，只是
	;把它做成函数，方便调用
	call load_file_block
	
	ret

;在这个地方把elf格式的内核加载到一个内存，elf文件不能从头执行，
;必须把它的代码和数据部分解析出来，这个操作是进入保护模式之后进行的
load_kernel_file:
    push dx

	;load kernel
	mov ax, KERNEL_SEG
	mov si, KERNEL_OFF
    mov dx, 8           ; 内核占用512kb，每次加载128扇区（64kb），因此需要加载8次
.read_replay:
	mov cx, BLOCK_SIZE
	;调用读取一整个块的扇区数据函数，其实也就是循环读取128个扇区，只是
	;把它做成函数，方便调用
	call load_file_block
	add ax, 0x1000
    dec dx
    cmp dx, 0
    ja .read_replay
	
    pop dx
    ; 总共加载8次，每次加载128扇区，总过512kb
	ret
    
;Global Descriptor Table,GDT
;gdt[0] always null
;1个gdt描述符的内容是8字节，可以根据那个描述得结构来分析这个结构体里面的内容
;一个描述符的格式如下
;低32位
;|0~15			|16~31			|
;|段界限0~15	|段地址0~15		|
;高32位
;|0~7			|8~11|12|13~14	|15|16~19 		|20 |21|22	|23|24~31		|
;|段地址16~23	|TYPE|S	|DPL	|P |段界限16~19 |AVL|L |D/B	|G |段地址24~31	|
;接下来分析一下代码段和数据段的格式吧
gdt_table:
	;0:void
	dd		0x00000000
	dd		0x00000000
	;1:4GB(flat-mode) code segment 0
	;低32位
	;|0~15			|16~31			|
	;|段界限0~15	|段地址0~15		|
	;|0xffff		|0x0000			|
	;高32位
	;|0~7			|8~11 	|12 |13~14	|15	|16~19 			|20 |21|22	|23|24~31		|
	;|段地址16~23	|TYPE	|S	|DPL	|P 		|段界限16~19 	|AVL|L |D/B	|G |段地址24~31	|
	;|0x00			|1010b	|1b |00b	|1b		|1111b 			|0  |0 |1	|1 |0x0000		|
	;=================
	;通过对数据的解析，我们可以知道他的段地址0~31位都是0，段界限0~19位都是1，并且发现G是1，
	;也就是说他是一个基地址为0x00000000,段界限为0xffffffff(因为G为1，所以粒度是4KB，也就是
	;段界限值0xfffff*4kb = 0xffffffff)的一个段。
	;S为1，说明是一个非系统段（不是说不能给系统用，只是说系统有自己特别类型的段，已经存在的）。
	;TYPE类型是1010b，也就是说他是一个可执行的、一致性代码段
	;DPL是0，也就是说他是一个特权级为0的段，正好，我们的系统的特权级就是0。
	;P表示是否存在，这个地方是1，表示存在
	;AVL ，AVaiLable，可用的。不过这是对用户来说的，也就是操作系统可以随意用此位。
	;对硬件来说，它没有专门的用途
	;L 为 1 表示 64 位代码段，否则表示 32位代码段。我们是32位操作系统，所以它是0
	;D/B用来指定是16位指令还是32位指令，为1是32位指令，为0是16位指令，这个地方是1
	;=================
	;根据解析，我们可以知道他是一个32位的，可执行的，基地址为0，界限为0xffffffff的代码段
	dd		0x0000ffff
	dd		0x00cf9A00
	;2:4GB(flat-mode) data segment 0
	;低32位
	;|0~15			|16~31			|
	;|段界限0~15	|段地址0~15		|
	;|0xffff		|0x0000			|
	;高32位
	;|0~7			|8~11 	|12 |13~14	|15	|16~19 			|20 |21|22	|23|24~31		|
	;|段地址16~23	|TYPE	|S	|DPL	|P 		|段界限16~19 	|AVL|L |D/B	|G |段地址24~31	|
	;|0x00			|0010b	|1b |00b	|1b		|1111b 			|0  |0 |1	|1 |0x0000		|
	;=================
	;发现这个描述符的和上一个的差别就是类型不一样。
	;TYPE类型是0010b，通过查表可知，也就是说他是可读可写的数据段
	;=================
	;根据解析，我们可以知道他是一个32位的，可执行的，基地址为0，界限为0xffffffff的数据段
	dd		0x0000ffff
	dd		0x00cf9200
	
gdt_length equ $ - gdt_table	
gdt_reg:
	dw	(gdt_length-1)
	dd	gdt_table

[bits 32]
align 32
flush:
	mov ax, 0x10	;the data 
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov gs, ax 
	mov ss, ax 
	mov esp, LOADER_STACK_TOP
	
	jmp 0x08: SETUP_ADDR

	jmp $

;fill it with 4kb
times (4096-($-$$)) db 0
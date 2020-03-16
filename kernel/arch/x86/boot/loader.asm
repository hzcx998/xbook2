
%include "const.inc"
%include "config.inc"

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

    ; load kernel file from disk
	call load_kernel_file

    ;我们不再使用软盘，所以这里关闭软盘驱动
	call kill_floppy_motor	
    
    ; get memory info
	call get_memory_info

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
	jmp	dword 0x08:protect_mode_entry
	
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



;在这个地方把elf格式的内核加载到一个内存，elf文件不能从头执行，
;必须把它的代码和数据部分解析出来，这个操作是进入保护模式之后进行的
load_kernel_file:
	;load kernel
	;first block 128 sectors
	;把内核文件加载到段为KERNEL_SEG（0x1000）的地方，也就是物理内存
	;为0x10000的地方，一次性加载BLOCK_SIZE（128）个扇区
	;写入参数
	mov ax, KERNEL_SEG
	mov si, KERNEL_OFF
	mov cx, BLOCK_SIZE
	;调用读取一整个块的扇区数据函数，其实也就是循环读取128个扇区，只是
	;把它做成函数，方便调用
	call load_file_block
	
	;second block 128 sectors
	;当读取完128个扇区后，我们的缓冲区位置要改变，也就是增加128*512=0x10000
	;的空间，由于ax会给es，所以这个地方用改变段的位置，所以就是0x1000,
	;扇区的位置是保留在si中的，上一次调用后，si递增了128，所以这里我们不对
	;si操作
	add ax, 0x1000
	mov cx, BLOCK_SIZE
	call load_file_block
	
	;third block 128 sectors
	;这个地方和上面同理
	add ax, 0x1000
	mov cx, BLOCK_SIZE
	call load_file_block
    
    ;third block 128 sectors
	;这个地方和上面同理
	add ax, 0x1000
	mov cx, BLOCK_SIZE
	call load_file_block
    
    ;third block 128 sectors
	;这个地方和上面同理
	add ax, 0x1000
	mov cx, BLOCK_SIZE
	call load_file_block
    
    ;third block 128 sectors
	;这个地方和上面同理
	add ax, 0x1000
	mov cx, BLOCK_SIZE
	call load_file_block
    
    ;third block 128 sectors
	;这个地方和上面同理
	add ax, 0x1000
	mov cx, BLOCK_SIZE
	call load_file_block
    
    ;third block 128 sectors
	;这个地方和上面同理
	add ax, 0x1000
	mov cx, BLOCK_SIZE
	call load_file_block
    
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
;切换到保护模式后会跳转到这里
protect_mode_entry:
	;init all segment registeres
	;初始化保护模式下的段描述符，此时访问内存方式已经改变
	
	;这个地方的0x10是选择子，不是段
	;选择子的格式是这样的
	;|3~15      |2 |0~1		|
	;|描述符索引 |TI|RPL	 |
	;0x10的解析是
	;|2         |0b|00b     |
	;也及时说RPL为0，及要访问的段的特权级为0
	;TI为0，也就是说在GDT中获取描述符，TI为1时，是在IDT中获取描述符。
	;索引为2，也就是第三个描述符，根据GDT可知，他是一个数据段
	;=============
	;也就是说，我们使用了数据段段
	mov ax, 0x10	;the data 
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov gs, ax 
	mov ss, ax 
	mov esp, LOADER_STACK_TOP
	
	;put 'P'
	mov byte [0xb8000+160*2+0], 'P'
	mov byte [0xb8000+160*2+1], 0X07
	mov byte [0xb8000+160*2+2], 'R'
	mov byte [0xb8000+160*2+3], 0X07
	mov byte [0xb8000+160*2+4], 'O'
	mov byte [0xb8000+160*2+5], 0X07
	mov byte [0xb8000+160*2+6], 'T'
	mov byte [0xb8000+160*2+7], 0X07
	mov byte [0xb8000+160*2+8], 'E'
	mov byte [0xb8000+160*2+9], 0X07
	mov byte [0xb8000+160*2+10], 'C'
	mov byte [0xb8000+160*2+11], 0X07
	mov byte [0xb8000+160*2+12], 'T'
	mov byte [0xb8000+160*2+13], 0X07
	
	call setup_page

	mov byte [0x800b8000+160*3+0], 'P'
	mov byte [0x800b8000+160*3+1], 0X07
	mov byte [0x800b8000+160*3+2], 'A'
	mov byte [0x800b8000+160*3+3], 0X07
	mov byte [0x800b8000+160*3+4], 'G'
	mov byte [0x800b8000+160*3+5], 0X07
	mov byte [0x800b8000+160*3+6], 'E'
	mov byte [0x800b8000+160*3+7], 0X07

	;从elf内核文件中读取内核的代码段和数据段到1M的位置
	call load_kernel

	;jmp $

	;这个时候，我们就可以跳转到1M这个地方去执行我们的内核了
	;由于在makefile中我们制定了-e _start,所以，就会把kernel/_start.asm中的
	;_start标签最为整个内核程序的入口地址，也就是说_start实在代码的最前面，
	;这个地方跳转过去，就是相当于跳转到_start执行
	
    ; 这个地方最好是自动获取地址
    jmp 0X08:KERNEL_START_ADDR
	
	;push eax
	;jmp $

; 遍历每一个 Program Header，根据 Program Header 中的信息来确定把什么放进内存，放到什么位置，以及放多少。
;把内核的代码段和数据段从elf文件中读取到1个对应的内存地址中
load_kernel:
	xor	esi, esi
	mov	cx, word [KERNEL_PHY_ADDR + 2Ch]
	movzx	ecx, cx					
	mov	esi, [KERNEL_PHY_ADDR + 1Ch]
	add	esi, KERNEL_PHY_ADDR
.begin:
	mov	eax, [esi + 0]
	cmp	eax, 0
	jz	.unaction
	push	dword [esi + 010h]
	mov	eax, [esi + 04h]	
	add	eax, KERNEL_PHY_ADDR
    push	eax				
	push	dword [esi + 08h]
	call	memcpy
	add	esp, 12	
.unaction:
	add	esi, 020h
	dec	ecx
	jnz	.begin
	ret
	
memcpy:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]
	mov	esi, [ebp + 12]
	mov	ecx, [ebp + 16]
.1:
	cmp	ecx, 0
	jz	.2

	mov	al, [ds:esi]
	inc	esi			
					
	mov	byte [es:edi], al
	inc	edi

	dec	ecx
	jmp	.1
.2:
	mov	eax, [ebp + 8]

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回	

setup_page:  
    mov ecx,1024                       ;1024个目录项
    mov ebx,PAGE_DIR_PHY_ADDR                 ;页目录的物理地址
    xor esi,esi
.clean_page_dir_table:
    mov dword [ebx+esi],0x00000000  ;页目录表项清零 
    add esi,4
    loop .clean_page_dir_table

    mov edi, PAGE_TBL_PHY_ADDR
    mov ebx, PAGE_DIR_PHY_ADDR

	; 第一个页表
    mov dword [ebx], PAGE_TBL_PHY_ADDR | KERNEL_PAGE_ATTR 
    mov dword [ebx+512*4], PAGE_TBL_PHY_ADDR | KERNEL_PAGE_ATTR    
	; 第二个页表
    mov dword [ebx+4], (PAGE_TBL_PHY_ADDR+0X1000) | KERNEL_PAGE_ATTR
    mov dword [ebx+513*4], (PAGE_TBL_PHY_ADDR+0X1000) | KERNEL_PAGE_ATTR    

	; 页目录自己
	mov dword [ebx+1023*4], PAGE_DIR_PHY_ADDR | KERNEL_PAGE_ATTR
   	
	;低端8M内存直接对应，可以直接访问到
   	mov cx, 1024*2
    mov esi, 0 | KERNEL_PAGE_ATTR
    
.map_kernel_page_table:	;kernel page table
    mov [edi], esi
    add esi, 0x1000
    add edi,4
    loop .map_kernel_page_table
   	
.switch_paging_mode:
   	mov eax , PAGE_DIR_PHY_ADDR
    mov cr3,eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ret	

;fill it with 4kb
times (4096-($-$$)) db 0
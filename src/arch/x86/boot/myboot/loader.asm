
%include "const.inc"
%include "config.inc"

; 配置图形模式
;%define CONFIG_GRAPHIC
; 启动图形模式
;%define CONFIG_GRAPHIC_ENABLE

org 0x90000
[bits 16]
align 16

entry:
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

	call load_setup_file

	call load_kernel_file
    
    %ifdef CONFIG_BOOT_FLOPPY
	call stop_floppy_motor	
    %endif
	call get_memory_info

    %ifdef CONFIG_GRAPHIC
    call get_vbe_info
    %endif

    jmp set_protect_mode
;保护模式设置的步骤为
;1.关闭中断，防止中间发生中断，因为保护模式中断和实模式中断的方式不一样
;2.加载gdt，保护模式进行内存访问需要用到gdt里面定义的数据结构
;3.打开A20总线，使得可以访问1MB以上的内存空间
;4.设置cr0的最低1位位1，就是切换成保护模式
;5.执行一个远跳转，清空cpu流水线
set_protect_mode:
	
	cli ;close the interruption
	lgdt	[gdt_reg]
	
	;enable A20 line
	in		al,0x92
	or		al,2
	out		0x92,al
	; enable protect mode
	mov		eax,cr0
	or		eax,1
	mov		cr0,eax
	
	;far jump:to clean the flush line
	jmp	dword 0x08:flush

load_setup_file:
	mov ax, SETUP_SEG
    mov dx, 0
	mov si, SETUP_OFF
	mov cx, SETUP_CNTS
    xor bx, bx 
	call read_sectors
	ret

load_kernel_file:
	mov ax, KERNEL_SEG
	mov si, KERNEL_OFF
    mov dx, 0
	mov cx, DISK_BLOCK_SIZE
    xor bx, bx 

    mov di, 16           ; 内核占用512kb，每次加载64扇区（32kb），因此需要加载16次
.replay:
	call read_sectors
	add ax, 0x800
    add si, DISK_BLOCK_SIZE

    dec di
    cmp di, 0
    ja .replay
	ret

%ifdef CONFIG_BOOT_FLOPPY
; function: read a sector data from floppy
; @input:
;       es: dx -> buffer seg: off
;       si     -> lba
floppy_read_sector:
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
%endif

%ifdef CONFIG_BOOT_HARDDISK
align 4
DAP:    ; disk address packet
    db 0x10 ; [0]: packet size in bytes
    db 0    ; [1]: reserved, must be 0
    db 0    ; [2]: nr of blocks to transfer (0~127)
    db 0    ; [3]: reserved, must be 0
    dw 0    ; [4]: buf addr(offset)
    dw 0    ; [6]: buf addr(seg)
    dd 0    ; [8]: lba. low 32-bit
    dd 0    ; [12]: lba. high 32-bit

; function: read a sector data from harddisk
; @input:
;       ax: dx  -> buffer seg: off
;       si     -> lba low 32 bits
harddisk_read_sector:
    push ax
    push bx
    push cx
    push dx
    push si

    mov word [DAP + 2], 1       ; count
    mov word [DAP + 4], dx      ; offset
    mov word [DAP + 6], ax      ; segment
    mov word [DAP + 8], si      ; lba low 32 bits
    mov dword [DAP + 12], 0     ; lba high 32 bits
    
    xor bx, bx
    mov ah, 0x42
    mov dl, 0x80
    mov si, DAP
    int 0x13
    
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret
%endif

read_sectors:
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    push es

.reply:
    %ifdef CONFIG_BOOT_HARDDISK
    call harddisk_read_sector
    add ax, 0x20    ; next buffer
    %endif
    
    %ifdef CONFIG_BOOT_FLOPPY
    mov es, ax
    call floppy_read_sector
    add bx, 512     ; next buffer
    %endif

    inc si          ; next lba
    loop .reply

    pop es
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret

%ifdef CONFIG_BOOT_FLOPPY 
stop_floppy_motor:
	push dx
	mov	dx, 03F2h
	mov	al, 0
	out	dx, al
	pop	dx
	ret
%endif

get_memory_info:
	mov ax, ARDS_SEG
	mov es, ax

	mov	ebx, 0			; ebx = 后续值, 开始时需为 0
	mov	di, 4		    ; es:di 指向一个地址范围描述符结构（Address Range Descriptor Structure）
.mem_loop:
	mov	eax, 0E820h		; eax = 0000E820h
	mov	ecx, 20			; ecx = 地址范围描述符结构的大小
	mov	edx, 0534D4150h	; edx = 'SMAP'
	int	15h			    
	jc	.mem_failed
	add	di, 20
	inc	dword [es:0]
	cmp	ebx, 0
	jne	.mem_loop
	jmp	.mem_ok
.mem_failed:
	mov	dword [es:0], 0
.mem_ok:

	ret 

%ifdef CONFIG_GRAPHIC

; 使用的分辨率
;VBE_MODE	EQU	VMODE_800_600_32
;VBE_MODE	EQU	VMODE_1920_1080_16
VBE_MODE	EQU	VMODE_1024_768_16
;VBE_MODE	EQU	VMODE_800_600_32

get_vbe_info:
    push ds
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
	mov	ax, 0x4f01	    ;获取画面模式功能，指定ax=0x4f01
	int	0x10

	cmp	ax, 0x004f	    ;ax=0x004f 指定的这种模式可以使用
	jne	.vbe_error

%ifdef CONFIG_GRAPHIC_ENABLE
    ;切换到指定的模式
	mov	bx, VBE_MODE + 0x4000	;bx=模式号和属性
	mov	ax, 0x4f02	            ;切换模式模式功能，指定ax=0x4f01
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
    pop ds
    ret
%endif

;Global Descriptor Table(GDT)
gdt_table:
	dd		0x00000000
	dd		0x00000000
	dd		0x0000ffff
	dd		0x00cf9A00
	dd		0x0000ffff
	dd		0x00cf9200
	
gdt_length equ $ - gdt_table	
gdt_reg:
	dw	(gdt_length-1)
	dd	gdt_table

[bits 32]
align 32
flush:
	mov ax, 0x10	;the data selector
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov gs, ax 
	mov ss, ax 
	mov esp, LOADER_STACK_TOP
	
	jmp 0x08: SETUP_ADDR

;pad loader to 4kb
times (4096-($-$$)) db 0

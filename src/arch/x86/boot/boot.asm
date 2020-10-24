%include "const.inc"
%include "config.inc"

org 0x7c00
[bits 16]
align 16

entry:
	mov ax, cs
	mov ds, ax
	mov ss, ax
	mov sp, 0
	mov ax, 0xb800
	mov es, ax

    call clean_screen

	;show 'BOOT'
	mov byte [es:0],'B'
	mov byte [es:1],0x07
	mov byte [es:2],'O'
	mov byte [es:3],0x07
	mov byte [es:4],'O'
	mov byte [es:5],0x07
	mov byte [es:6],'T'
	mov byte [es:7],0x07

    mov ax, LOADER_SEG
    mov dx, 0
    mov si, LOADER_OFF
    mov cx, LOADER_CNTS
    xor bx, bx
    call read_sectors

	jmp LOADER_SEG:0

;clean screan
clean_screen:
	mov ax, 0x02
	int 0x10
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

times 510-($-$$) db 0
dw 0xaa55   ; boot sector flags
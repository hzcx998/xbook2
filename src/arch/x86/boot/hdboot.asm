%include "const.inc"

org 0x7c00
[bits 16]
align 16

entry:
    jmp boot_start

DAP:    ; disk address packet
    db 0x10 ; [0]: packet size in bytes
    db 0    ; [1]: reserved, must be 0
    db 1    ; [2]: nr of blocks to transfer (0~127)
    db 0    ; [3]: reserved, must be 0
    dw 0    ; [4]: buf addr(offset)
    dw 0    ; [6]: buf addr(seg)
    dd 0    ; [8]: lba. low 32-bit
    dd 0    ; [12]: lba. high 32-bit

boot_dot: dw 0

boot_start:
	mov ax, cs
	mov ds, ax
	mov ss, ax
	mov sp, 0
	mov ax, 0xb800
	mov es, ax

;clean screan
clean_screen:
	mov ax, 0x02
	int 0x10
	;show 'BOOT'
	mov byte [es:0],'B'
	mov byte [es:1],0x07
	mov byte [es:2],'O'
	mov byte [es:3],0x07
	mov byte [es:4],'O'
	mov byte [es:5],0x07
	mov byte [es:6],'T'
	mov byte [es:7],0x07

    mov word [boot_dot], 8

    ; buffer
    mov word [DAP + 6], LOADER_SEG
    mov word [DAP + 4], 0
    ; lba
    mov dword [DAP + 8], LOADER_OFF
    mov cx, LOADER_CNTS

read_loader_from_disk:	
    ; show dot '.'
    mov si, [boot_dot]
	mov byte [es:si],'.'
	mov byte [es:si + 1],0x07
    add word [boot_dot], 2

	call harddisk_read_sectors
    
    add word [DAP + 4], 512     ; buffer off += 512
    inc dword [DAP + 8]         ; lba off += 1
	loop read_loader_from_disk

	jmp LOADER_SEG:0

	;we read load from sector 1(0 is first) width 8 sectors to 0x10000
	;es:dx=buffer address
	;mov ax, LOADER_SEG
	;mov es, ax 
	;xor bx, bx 
	;mov si, LOADER_OFF
	;mov cx, LOADER_CNTS

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
.RP:
	mov al, 0x01
	mov ah, 0x02 
	int 0x13 
	jc .RP 
	pop dx
	pop cx 
	pop ax
	ret

harddisk_read_sectors:
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    
    xor bx, bx

    mov ah, 0x42
    mov dl, 0x80
    mov si, DAP
    int 0x13
    
    
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret

times 510-($-$$) db 0
dw 0xaa55   ; boot sector flags

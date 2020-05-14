%include "const.inc"

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


	;we read load from sector 1(0 is first) width 8 sectors to 0x10000
	;es:dx=buffer address
	mov ax, LOADER_SEG
	mov es, ax 
	xor bx, bx 
	mov si, LOADER_OFF
	mov cx, LOADER_CNTS
read_loader_from_disk:	
	call floppy_read_sectors
	add bx, 512
	inc si 
	loop read_loader_from_disk

	jmp LOADER_SEG:0
	
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
	
times 510-($-$$) db 0
dw 0xaa55   ; boot sector flags

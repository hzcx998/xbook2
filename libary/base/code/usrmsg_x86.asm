[bits 32]
[section .text]

USRMSG_INT	EQU 0x40

; int usrmsg(umsg_t *msg);
global usrmsg
usrmsg:
	push ebx
	mov ebx, [esp + 8]	; ebx = msg
	int USRMSG_INT		; raise a int to USRMSG_INT
	pop ebx
	ret
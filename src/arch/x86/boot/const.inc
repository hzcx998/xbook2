;----
;file:      arch/x86/include/boot/const.inc
;auther:    Jason Hu
;time:      2019/6/2
;copyright:	(C) 2018-2020 by Book OS developers. All rights reserved.
;----

;----boot const----
;we read loader.bin from disk sector 2
LOADER_OFF EQU 2	

;we assume loader.bin size is 4kb(8 sectors)	
LOADER_CNTS EQU 8		

;we read loader.bin to memory 0x90000 for 4kb
;so the segment is 0x9000, we are in real mode now.
LOADER_SEG EQU 0x9000	

;we read setup.bin from disk sector 10
SETUP_OFF EQU 10	

;we assume setup.bin size is 36kb(72 sectors)	
SETUP_CNTS EQU 90		

;we read setup.bin to memory 0x91000 for 36kb
;so the segment is 0x9100, we are in real mode now.
SETUP_SEG EQU 0x9100	

SETUP_ADDR EQU 0x91000	

;---loader const----
;we read kernel.elf from disk sector 100 
KERNEL_OFF EQU 100	

;----we assume that kernel max is 256 kb (0x40000 byte)----
; 内核文件的物理内存范围是0x10000~0x50000

;we read kernel.elf to memory 0x10000
;so the segment is 0x1000, we are in real mode now.
KERNEL_SEG EQU 0x1000

;kernel.elf phy addr is KERNEL_SEG*16, 
;0x1000*16 = 0x10000
KERNEL_PHY_ADDR EQU KERNEL_SEG*16

;we read filesrv from disk sector 700 
FILESRV_OFF EQU 700	

;----we assume that filesrv max is 256 kb (0x40000 byte)----
; 内核文件的物理内存范围是0x50000~0x90000

;we read filesrv to memory 0x50000
;so the segment is 0x5000, we are in real mode now.
FILESRV_SEG EQU 0x5000

;filesrv phy addr is FILESRV_SEG*16, 
;0x5000*16 = 0x50000
FILESRV_PHY_ADDR EQU FILESRV_SEG*16

;we will read kernel from kerne.elf to 1M after we jmp to protect mode,
;so this is our kernel finally addr
KERNEL_START_ADDR EQU 0x80100000

;loader stack top is 0x9a000 ~ 0x9c000
LOADER_STACK_TOP EQU 0x9c000

;we can read sector less than 128 sector. so I set block size,
;every time we load 128 sectors
BLOCK_SIZE EQU 128

ARDS_SEG	EQU		0x100	;ards start segment

;分页机制
;内核页目录表地址
PAGE_DIR_PHY_ADDR EQU   0x201000
;内核页表地址
PAGE_TBL_PHY_ADDR EQU   0x202000

;显存页表地址
VRAM_PT_PHY_ADDR    EQU 0x203000

PAGE_P_1	EQU  	1	; 0001 exist in memory
PAGE_P_0	EQU  	0	; 0000 not exist in memory
PAGE_RW_R  	EQU     0	; 0000 R/W read/execute
PAGE_RW_W  	EQU     2	; 0010 R/W read/write/execute
PAGE_US_S  	EQU     0	; 0000 U/S system level, cpl0,1,2
PAGE_US_U  	EQU     4	; 0100 U/S user level, cpl3

KERNEL_PAGE_ATTR    EQU (PAGE_US_S | PAGE_RW_W | PAGE_P_1)

;VBE支持
VBE_INFO_SEG	EQU		0x110	; 物理地址是0x1100
VBE_INFO_SIZE	EQU		0x200	; vbe2.0+ 的信息块大小是512 字节

VBE_MODE_SEG	EQU		0x130	; 物理地址是0x1300
VBE_MODE_SIZE	EQU		0x100	; 固定大小256字节

VBE_CMD_VBEINFO  EQU     0x4f00  ; 返回VBE控制器的信息

; 配置vbe驱动，注意也要修改内核中的图形配置
%define CONFIG_GRAPHIC      1

; 启动图形模式
%define CONFIG_GRAPHIC_SWITCH   1
; 注意！！！ 配置图形只检测图形模式，如果要切换到图形模式，则需要打开图形开关

%if CONFIG_GRAPHIC == 1
;图形模式列表：
; 16位
VMODE_640_480_16     EQU 0x111
VMODE_800_600_16     EQU 0x114
VMODE_1024_768_16    EQU 0x117
VMODE_1280_1024_16   EQU 0x11A

; 24位
; 24bit 不能再vmware中使用，不知道为何，可能是不支持
VMODE_640_480_24            EQU 0x112
VMODE_800_600_24            EQU 0x115
VMODE_1024_768_24           EQU 0x118
VMODE_1280_1024_24          EQU 0x11B
VMODE_VMWARE_800_600_24     EQU	0x140

; 使用的模式
VBE_MODE	EQU	VMODE_640_480_16

%endif  ; CONFIG_GRAPHIC


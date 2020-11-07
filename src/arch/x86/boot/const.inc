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

;we assume setup.bin size is 32kb(64 sectors)	
SETUP_CNTS EQU 64		

;we read setup.bin to memory 0x91000 for 32kb
;so the segment is 0x9100, we are in real mode now.
SETUP_SEG EQU 0x9100	

SETUP_ADDR EQU 0x91000	

;---loader const----
;we read kernel.elf from disk sector 100 
KERNEL_OFF EQU 100	

;----we assume that kernel max is 512 kb (0x80000 byte)----
; 内核文件的物理内存范围是0x10000~0x50000

;we read kernel.elf to memory 0x10000
;so the segment is 0x1000, we are in real mode now.
KERNEL_SEG EQU 0x1000

;kernel.elf phy addr is KERNEL_SEG*16, 
;0x1000*16 = 0x10000
KERNEL_PHY_ADDR EQU KERNEL_SEG*16

;we will read kernel from kerne.elf to 1M after we jmp to protect mode,
;so this is our kernel finally addr
KERNEL_START_ADDR EQU 0x80100000

;loader stack top is 0x9a000 ~ 0x9c000
LOADER_STACK_TOP EQU 0x9c000

;we can read sector less than 128 sector. so I set block size,
;every time we load 128 sectors
BLOCK_SIZE EQU 64

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

; 8位
VMODE_1600_1200_8           EQU 0x11C
VMODE_320_200_8             EQU 0x146
VMODE_1152_864_8            EQU 0x148

; 15位
VMODE_800_600_15            EQU 0x113
VMODE_1024_768_15           EQU 0x116
VMODE_1280_1024_15          EQU 0x119
VMODE_1600_1200_15          EQU 0x11d
VMODE_1152_864_15           EQU 0x149

; 16位
VMODE_640_480_16            EQU 0x111
VMODE_800_600_16            EQU 0x114
VMODE_1024_768_16           EQU 0x117
VMODE_1280_1024_16          EQU 0x11A
VMODE_1600_1200_16          EQU 0x11e
VMODE_1152_864_16           EQU 0x14a
VMODE_1280_768_16           EQU 0x175
VMODE_1280_800_16           EQU 0x178
VMODE_1280_960_16           EQU 0x17b
VMODE_1400_900_16           EQU 0x17e
VMODE_1400_1050_16          EQU 0x181
VMODE_1680_1050_16          EQU 0x184
VMODE_1920_1200_16          EQU 0x187
VMODE_2560_1600_16          EQU 0x18a
VMODE_1280_720_16           EQU 0x18d
VMODE_1920_1080_16          EQU 0x190

; 24位
; 24bit 不能再vmware中使用，不知道为何，可能是不支持
VMODE_VMWARE_800_600_24     EQU	0x140
VMODE_640_480_24            EQU 0x112
VMODE_800_600_24            EQU 0x115
VMODE_1024_768_24           EQU 0x118
VMODE_1280_1024_24          EQU 0x11B
VMODE_1600_1200_24          EQU 0x11f
VMODE_1152_864_24           EQU 0x14b
VMODE_1280_768_24           EQU 0x173
VMODE_1280_800_24           EQU 0x179
VMODE_1280_960_24           EQU 0x17c
VMODE_1400_900_24           EQU 0x17f
VMODE_1400_1050_24          EQU 0x182
VMODE_1680_1050_24          EQU 0x185
VMODE_1920_1200_24          EQU 0x188
VMODE_2560_1600_24          EQU 0x18b
VMODE_1280_720_24           EQU 0x18e
VMODE_1920_1080_24          EQU 0x191

; 32位
VMODE_320_200_32            EQU 0x140
VMODE_640_400_32            EQU 0x141
VMODE_640_480_32            EQU 0x142
VMODE_800_600_32            EQU 0x143
VMODE_1024_768_32           EQU 0x144
VMODE_1280_1024_32          EQU 0x145
VMODE_1600_1200_32          EQU 0x147
VMODE_1152_864_32           EQU 0x14c
VMODE_1280_768_32           EQU 0x177
VMODE_1280_800_32           EQU 0x17a
VMODE_1280_960_32           EQU 0x17d
VMODE_1400_900_32           EQU 0x180
VMODE_1400_1050_32          EQU 0x183
VMODE_1680_1050_32          EQU 0x186
VMODE_1920_1200_32          EQU 0x189
VMODE_2560_1600_32          EQU 0x18c
VMODE_1280_720_32           EQU 0x18f
VMODE_1920_1080_32          EQU 0x192

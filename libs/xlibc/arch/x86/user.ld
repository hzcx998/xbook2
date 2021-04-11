/* Simple linker script for the xbook user crt
    See the GNU ld 'info' manual ("info ld") to learn the syntax. */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start) /* _start is at xlibc/arch/x86/start.asm */

SECTIONS {
    /* 
    if use lower addr: . = 0x00001000;  
    or not use high addr: . = 0x80000000;
    */
    . = 0x80000000;
    
    /* code segment */
    .text : {
        . = ALIGN(4);   /* 4 bytes align */
        *(.text)
    }

    /* data segment */
    .data : {
        . = ALIGN(4);   /* 4 bytes align */
        *(.data)
    }

    /* bss segement */
    .bss : {
        . = ALIGN(4);   /* 4 bytes align */
        *(.bss)
    }
}
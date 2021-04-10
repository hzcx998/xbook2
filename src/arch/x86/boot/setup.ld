/* Simple linker script for the xbook kernel boot-setup
    See the GNU ld 'info' manual ("info ld") to learn the syntax. */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start) /* _start is at start.asm */

SECTIONS {
    . = 0x91000;

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
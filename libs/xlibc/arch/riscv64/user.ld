/* Simple linker script for the xbook user crt
    See the GNU ld 'info' manual ("info ld") to learn the syntax. */

OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS {
    . = 0x00001000;
    
    /* code segment */
    .text : {
        . = ALIGN(4);   /* 4 bytes align */
        *(.text.entry)
        *(.text )
    }

    /* data segment */
    .data : {
        . = ALIGN(4);   /* 4 bytes align */
        *(.data )
    }

    /* bss segement */
    .bss : {
        . = ALIGN(4);   /* 4 bytes align */
        *(.bss )
    }
}
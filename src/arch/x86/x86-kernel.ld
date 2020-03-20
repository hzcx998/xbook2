/* Simple linker script for the xbook kernel
    See the GNU ld 'info' manual ("info ld") to learn the syntax. */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(kernel_start) /* kernel_start is at kernel/arch/x86/entry.asm */

SECTIONS {
    /* Load the kernel at this address: "." means the current address */
    . = 0x80100000;

    /* code segment */
    .text : {
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
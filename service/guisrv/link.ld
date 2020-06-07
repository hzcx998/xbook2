/* Simple linker script for the xbook kernel
    See the GNU ld 'info' manual ("info ld") to learn the syntax. */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS {
    /* Load the kernel at this address: "." means the current address */
    . = 0x00001000;
    /* code segment */
    .text : {
        _stext = .;           /* Provide the name for the start of this section */
        *(.text)
        *(.text.*)              /*  cpp namespace function      */

        . = ALIGN(4);           /* Align the start of the rodata part */
        *(.rodata)              /*  read-only data (constants)  */
        *(.rodata*)
    }
    /* .data 段数据初始化内容放在这里 */
    . = ALIGN(16);
    _etext = . ;

    /* data segment */
    .data : {
        . = ALIGN(4);        /* Align the start of the section */
        _sdata = .;          /* Provide the name for the start of this section */

        *(.data)
        *(.data.*)

        . = ALIGN(4);        /* Align the end of the section */
    }
    _edata = .;             /* Provide the name for the end of this section */
   
    /* bss segement */
    .bss : {
        . = ALIGN(4);        /* Align the start of the section */
        _sbss = .;           /* Provide the name for the start of this section */
      
        *(.bss)
        *(.bss.*)
        . = ALIGN(512);
    }
    _ebss = .;             /* Provide the name for the end of this section */
}
; i386 C RunTime Library i, crti.asm

[bits 32]
[section .init]

global _init
_init:
    sub esp, 12

[bits 32]
[section .fini]

global _fini
_fini:
    sub esp, 12

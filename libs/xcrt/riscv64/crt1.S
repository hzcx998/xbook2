.extern __libc_start_main
.extern __libc_csu_fini	
.extern __libc_csu_init	
.extern __start_main
.extern main	

.section .text.entry
.global _start
_start:
    mv a0, sp
    tail __start_main
loop:
    j loop
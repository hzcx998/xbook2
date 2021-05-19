.extern __libc_start_main
.extern __libc_csu_fini	
.extern __libc_csu_init	
.extern main	

.section .text.entry
.global _start
_start:
    call main
loop:
    j loop
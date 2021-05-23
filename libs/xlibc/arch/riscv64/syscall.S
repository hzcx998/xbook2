.section .text
.global __syscalln
__syscalln:
    # a0: syscall number
    # a1: arg1
    # a2: arg2
    # a3: arg3
    # a4: arg4
    ecall
    ret
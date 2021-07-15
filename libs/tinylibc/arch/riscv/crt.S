
.section .sdata,"aw"
.text
.global _start
.type _start ,%function
_start:
.weak __global_pointer
.hidden __global_pointer
.option push
.option norelax
lla gp, __global_pointer
.option pop
mv a0, sp
.weak _DYNAMIC
.hidden _DYNAMIC
lla a1, _DYNAMIC
andi sp, sp, -16
tail __start_main
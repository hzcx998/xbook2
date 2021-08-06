    .section .text
    .global fpu_register_init
fpu_register_init:
#if __riscv_flen == 32
	fssr x0
	fmv.s.x f0, x0
	fmv.s.x f1, x0
	fmv.s.x f2, x0
	fmv.s.x f3, x0
	fmv.s.x f4, x0
	fmv.s.x f5, x0
	fmv.s.x f6, x0
	fmv.s.x f7, x0
	fmv.s.x f8, x0
	fmv.s.x f9, x0
	fmv.s.x f10, x0
	fmv.s.x f11, x0
	fmv.s.x f12, x0
	fmv.s.x f13, x0
	fmv.s.x f14, x0
	fmv.s.x f15, x0
	fmv.s.x f16, x0
	fmv.s.x f17, x0
	fmv.s.x f18, x0
	fmv.s.x f19, x0
	fmv.s.x f20, x0
	fmv.s.x f21, x0
	fmv.s.x f22, x0
	fmv.s.x f23, x0
	fmv.s.x f24, x0
	fmv.s.x f25, x0
	fmv.s.x f26, x0
	fmv.s.x f27, x0
	fmv.s.x f28, x0
	fmv.s.x f29, x0
	fmv.s.x f30, x0
	fmv.s.x f31, x0
#elif __riscv_flen == 64
	fssr x0
	fmv.d.x f0, x0
	fmv.d.x f1, x0
	fmv.d.x f2, x0
	fmv.d.x f3, x0
	fmv.d.x f4, x0
	fmv.d.x f5, x0
	fmv.d.x f6, x0
	fmv.d.x f7, x0
	fmv.d.x f8, x0
	fmv.d.x f9, x0
	fmv.d.x f10, x0
	fmv.d.x f11, x0
	fmv.d.x f12, x0
	fmv.d.x f13, x0
	fmv.d.x f14, x0
	fmv.d.x f15, x0
	fmv.d.x f16, x0
	fmv.d.x f17, x0
	fmv.d.x f18, x0
	fmv.d.x f19, x0
	fmv.d.x f20, x0
	fmv.d.x f21, x0
	fmv.d.x f22, x0
	fmv.d.x f23, x0
	fmv.d.x f24, x0
	fmv.d.x f25, x0
	fmv.d.x f26, x0
	fmv.d.x f27, x0
	fmv.d.x f28, x0
	fmv.d.x f29, x0
	fmv.d.x f30, x0
	fmv.d.x f31, x0
#endif
    ret


    .global fpu_save_regs
fpu_save_regs:
#if __riscv_flen == 64
	/* store fs0 - fs11 */
	fsd fs0, 0x00(a0)
	fsd fs1, 0x08(a0)
	fsd fs2, 0x10(a0)
	fsd fs3, 0x18(a0)
	fsd fs4, 0x20(a0)
	fsd fs5, 0x28(a0)
	fsd fs6, 0x30(a0)
	fsd fs7, 0x38(a0)
	fsd fs8, 0x40(a0)
	fsd fs9, 0x48(a0)
	fsd fs10, 0x50(a0)
	fsd fs11, 0x58(a0)
#endif
    ret

    .global fpu_restore_regs
fpu_restore_regs:
	/* load fs0 - fs11 */
#if __riscv_flen == 64
	fld fs0, 0x00(a0)
	fld fs1, 0x08(a0)
	fld fs2, 0x10(a0)
	fld fs3, 0x18(a0)
	fld fs4, 0x20(a0)
	fld fs5, 0x28(a0)
	fld fs6, 0x30(a0)
	fld fs7, 0x38(a0)
	fld fs8, 0x40(a0)
	fld fs9, 0x48(a0)
	fld fs10, 0x50(a0)
	fld fs11, 0x58(a0)
#endif
    ret
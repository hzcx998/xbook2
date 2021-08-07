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
	fsd f0, 0x00(a0)
	fsd f1, 0x08(a0)
	fsd f2, 0x10(a0)
	fsd f3, 0x18(a0)
	fsd f4, 0x20(a0)
	fsd f5, 0x28(a0)
	fsd f6, 0x30(a0)
	fsd f7, 0x38(a0)
	fsd f8, 0x40(a0)
	fsd f9, 0x48(a0)
	fsd f10, 0x50(a0)
	fsd f11, 0x58(a0)
	fsd f12, 0x60(a0)
	fsd f13, 0x68(a0)
	fsd f14, 0x70(a0)
	fsd f15, 0x78(a0)
	fsd f16, 0x80(a0)
	fsd f17, 0x88(a0)
	fsd f18, 0x90(a0)
	fsd f19, 0x98(a0)
	fsd f20, 0xa0(a0)
	fsd f21, 0xa8(a0)
	fsd f22, 0xb0(a0)
	fsd f23, 0xb8(a0)
	fsd f24, 0xc0(a0)
	fsd f25, 0xc8(a0)
	fsd f26, 0xd0(a0)
	fsd f27, 0xd8(a0)
	fsd f28, 0xe0(a0)
	fsd f29, 0xe8(a0)
	fsd f30, 0xf0(a0)
	fsd f31, 0xf8(a0)
#endif
    ret

    .global fpu_restore_regs
fpu_restore_regs:
	/* load fs0 - fs11 */
#if __riscv_flen == 64
	fld f0, 0x00(a0)
	fld f1, 0x08(a0)
	fld f2, 0x10(a0)
	fld f3, 0x18(a0)
	fld f4, 0x20(a0)
	fld f5, 0x28(a0)
	fld f6, 0x30(a0)
	fld f7, 0x38(a0)
	fld f8, 0x40(a0)
	fld f9, 0x48(a0)
	fld f10, 0x50(a0)
	fld f11, 0x58(a0)
	fld f12, 0x60(a0)
	fld f13, 0x68(a0)
	fld f14, 0x70(a0)
	fld f15, 0x78(a0)
	fld f16, 0x80(a0)
	fld f17, 0x88(a0)
	fld f18, 0x90(a0)
	fld f19, 0x98(a0)
	fld f20, 0xa0(a0)
	fld f21, 0xa8(a0)
	fld f22, 0xb0(a0)
	fld f23, 0xb8(a0)
	fld f24, 0xc0(a0)
	fld f25, 0xc8(a0)
	fld f26, 0xd0(a0)
	fld f27, 0xd8(a0)
	fld f28, 0xe0(a0)
	fld f29, 0xe8(a0)
	fld f30, 0xf0(a0)
	fld f31, 0xf8(a0)
#endif
    ret
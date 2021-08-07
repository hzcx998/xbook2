#ifndef _RISCV64_FPU_H
#define _RISCV64_FPU_H

#include <string.h>
#include <xbook/debug.h>
#include "riscv.h"

#define FLOAT_REGISTER_NR   32

typedef struct  {
    double fs[FLOAT_REGISTER_NR];
} fpu_storage_t;

typedef struct  {
    fpu_storage_t storage;
} fpu_t;

extern void fpu_register_init();
extern void fpu_save_regs(fpu_storage_t *storage);
extern void fpu_restore_regs(fpu_storage_t *storage);

static inline void fpu_init(fpu_t *fpu, int reg)
{
    /* 需要初始化寄存器才调用 */
    if (reg) {
        fpu_register_init();
    }
    memset(&fpu->storage, 0, sizeof(fpu_storage_t));
}

static inline void fpu_save(fpu_t *fpu)
{
    fpu_save_regs(&fpu->storage);
}

static inline void fpu_restore(fpu_t *fpu)
{
    fpu_restore_regs(&fpu->storage);
}

#endif  /* _RISCV64_FPU_H */

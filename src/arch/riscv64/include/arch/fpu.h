#ifndef _RISCV64_FPU_H
#define _RISCV64_FPU_H

#include <string.h>
#include "riscv.h"

typedef struct  {
    double fs0;
    double fs1;
    double fs2;
    double fs3;
    double fs4;
    double fs5;
    double fs6;
    double fs7;
    double fs8;
    double fs9;
    double fs10;
    double fs11;
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

#ifndef _RISCV64_FPU_H
#define _RISCV64_FPU_H

#include <string.h>

typedef struct  {
    long unused;
} fpu_storage_t;

typedef struct  {
    fpu_storage_t storage;
} fpu_t;

static inline void fpu_init(fpu_t *fpu, int reg)
{
    //if (reg)    /* 需要初始化寄存器才调用 */
        //__asm__ __volatile__ ("fninit");
    memset(&fpu->storage, 0, sizeof(fpu_storage_t));
}

static inline void fpu_save(fpu_t *fpu)
{
    //__asm__ __volatile__ ( "fnsave %0 ; fwait" : "=m" (fpu->storage));
}

static inline void fpu_restore(fpu_t *fpu)
{
    //__asm__ __volatile__ ("frstor %0	\n\t": :"m" (fpu->storage));
}
#endif  /* _RISCV64_FPU_H */

#ifndef _X86_FPU_H
#define _X86_FPU_H

#include <string.h>

/* i387 fsave */
typedef struct  {
    long cwd;
    long swd;
    long twd;
    long fip;
    long fcs;
    long foo;
    long fos;
    long st_space[20]; /* 8*10 bytes for each FP-reg = 80 bytes */
    long status; /* software status information */
} fpu_storage_t;

typedef struct  {
    fpu_storage_t storage;
} fpu_t;

static inline void fpu_init(fpu_t *fpu, int reg)
{
    if (reg)    /* 需要初始化寄存器才调用 */
        __asm__ __volatile__ ("fninit");
    memset(&fpu->storage, 0, sizeof(fpu_storage_t));
}

static inline void fpu_save(fpu_t *fpu)
{
    __asm__ __volatile__ ( "fnsave %0 ; fwait" : "=m" (fpu->storage));
}

static inline void fpu_restore(fpu_t *fpu)
{
    __asm__ __volatile__ ("frstor %0	\n\t": :"m" (fpu->storage));
}
#endif  /* _X86_FPU_H */

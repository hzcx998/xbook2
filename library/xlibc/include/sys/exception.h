#ifndef _SYS_EXCEPTION_H
#define _SYS_EXCEPTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <stdint.h>

enum exception_code {
    EXP_CODE_UNKNOWN = 0,
    EXP_CODE_USER,
    EXP_CODE_INT,
    EXP_CODE_ILL,
    EXP_CODE_TRAP,
    EXP_CODE_ABORT,
    EXP_CODE_BUS,
    EXP_CODE_SEGV,
    EXP_CODE_FPE,
    EXP_CODE_FINALHIT,
    EXP_CODE_PIPE,
    EXP_CODE_STKFLT,
    EXP_CODE_ALRM,
    EXP_CODE_TERM,
    EXP_CODE_CHLD,
    EXP_CODE_CONT,
    EXP_CODE_STOP,
    EXP_CODE_TTIN,
    EXP_CODE_TTOU,
    EXP_CODE_SYS,
    EXP_CODE_MAX_NR
};

typedef void (*exp_hander_t) (uint32_t, uint32_t);

int expsend(pid_t pid, uint32_t code, uint32_t arg);
int expraise(uint32_t code, uint32_t arg);
int expcatch(uint32_t code, exp_hander_t handler);
int expblock(uint32_t code);
int expunblock(uint32_t code);
int expcheck();

#ifdef __cplusplus
}
#endif

#endif   /* _SYS_EXCEPTION_H */
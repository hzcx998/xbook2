#ifndef _XBOOK_SOFT_EXCEPTION_H
#define _XBOOK_SOFT_EXCEPTION_H

#include <stdint.h>
#include <types.h>
#include <xbook/list.h>
#include <xbook/spinlock.h>

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

#define EXCEPTION_SETS_SIZE   (EXP_CODE_MAX_NR / 32 + 1)

typedef struct {
    list_t list;        // 单个任务的异常构成一个列表
    uint32_t code;
    uint32_t flags;
    pid_t source;       // 异常来源
    uint32_t arg;
} exception_t;

typedef struct {
    list_t exception_list;
    list_t catch_list;
    uint32_t exception_number;
    uint32_t catch_number;
    spinlock_t manager_lock;    // 管理器使用的锁
    uint32_t exception_block[EXCEPTION_SETS_SIZE];    // 异常的阻塞情况    
    uint32_t exception_catch[EXCEPTION_SETS_SIZE];    // 异常的捕捉情况    
} exception_manager_t;

void exception_manager_init(exception_manager_t *exception_manager);
void exception_manager_exit(exception_manager_t *exception_manager);
int exception_send(pid_t pid, uint32_t code, uint32_t arg);
int exception_force(pid_t pid, uint32_t code, uint32_t arg);
int exception_copy(exception_manager_t *dest, exception_manager_t *src);

int sys_expsend(pid_t pid, uint32_t code, uint32_t arg);
int sys_expchkpoint(uint32_t *code, uint32_t *arg);
int sys_expcatch(uint32_t code, uint32_t state);
int sys_expblock(uint32_t code, uint32_t state);

#endif   /* _XBOOK_SOFT_EXCEPTION_H */

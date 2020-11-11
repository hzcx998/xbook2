#ifndef _XBOOK_SOFT_EXCEPTION_H
#define _XBOOK_SOFT_EXCEPTION_H

#include <stdint.h>
#include <types.h>
#include <xbook/list.h>
#include <xbook/spinlock.h>
#include <arch/interrupt.h>

enum exception_code {
    EXP_CODE_UNKNOWN = 0,
    EXP_CODE_SEGMENT,
    EXP_CODE_USER,
    EXP_CODE_INTR,
    EXP_CODE_MAX_NR
};

#define EXCEPTION_BLOCK_SIZE   (EXP_CODE_MAX_NR / 32 + 1)

typedef struct {
    list_t list;        // 单个任务的异常构成一个列表
    uint32_t code;
    uint32_t flags;
    pid_t source;       // 异常来源
    uint32_t arg;
} exception_t;

typedef struct {
    list_t exception_list;
    uint32_t exception_number;
    spinlock_t manager_lock;    // 管理器使用的锁
    uint32_t exception_block[EXCEPTION_BLOCK_SIZE];    // 异常的阻塞情况    
} exception_manager_t;

void exception_manager_init(exception_manager_t *exception_manager);
void exception_manager_exit(exception_manager_t *exception_manager);
int exception_send(pid_t pid, uint32_t code, uint32_t arg);
int exception_check(trap_frame_t *frame);
int exception_copy(exception_manager_t *dest, exception_manager_t *src);

#endif   /* _XBOOK_SOFT_EXCEPTION_H */

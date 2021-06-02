#ifndef _XBOOK_SOFT_EXCEPTION_H
#define _XBOOK_SOFT_EXCEPTION_H

#include <stdint.h>
#include <types.h>
#include <xbook/list.h>
#include <xbook/spinlock.h>
#include <xbook/waitqueue.h>

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
    EXP_CODE_DEVICE,
    EXP_CODE_MAX_NR
};

#define EXCEPTION_SETS_SIZE   (EXP_CODE_MAX_NR / 32 + 1)

typedef void (*exception_handler_t) (uint32_t);

typedef struct {
    list_t list;        // 单个任务的异常构成一个列表
    uint32_t code;
    uint32_t flags;
    pid_t source;       // 异常来源
} exception_t;

typedef struct {
	char *ret_addr;                 /* 记录返回地址 */
	unsigned long code;             /* 异常号 */
    trap_frame_t trap_frame;        /* 保存原来的栈框 */
	char ret_code[8];               /* 构建返回的系统调用代码 */
} exception_frame_t;

typedef struct {
    list_t exception_list;
    list_t catch_list;
    uint32_t exception_number;
    uint32_t catch_number;
    int in_user_mode;
    wait_queue_t wakeup_queue;      /* 需要唤醒的任务的队列 */
    spinlock_t manager_lock;    // 管理器使用的锁
    uint32_t exception_block[EXCEPTION_SETS_SIZE];    // 异常的阻塞情况    
    uint32_t exception_catch[EXCEPTION_SETS_SIZE];    // 异常的捕捉情况    
    exception_handler_t handlers[EXP_CODE_MAX_NR];
} exception_manager_t;

void exception_manager_init(exception_manager_t *exception_manager);
void exception_manager_exit(exception_manager_t *exception_manager);
int exception_send(pid_t pid, uint32_t code);
int exception_send_group(pid_t pgid, uint32_t code);
int exception_force(pid_t pid, uint32_t code);
int exception_copy(exception_manager_t *dest, exception_manager_t *src);
int exception_force_self(uint32_t code);
int exception_raise(uint32_t code);
bool exception_cause_exit(exception_manager_t *exception_manager);
bool exception_cause_exit_when_wait(exception_manager_t *exception_manager);
void exception_frame_build(uint32_t code, exception_handler_t handler, trap_frame_t *frame);
int exception_return(trap_frame_t *frame);

void exception_enable_block(exception_manager_t *exception_manager, uint32_t code);
void exception_disable_block(exception_manager_t *exception_manager, uint32_t code);

int sys_expsend(pid_t pid, uint32_t code);
int sys_expcatch(uint32_t code, exception_handler_t handler);
int sys_expblock(uint32_t code, uint32_t state);
int sys_excetion_return(unsigned int ebx, unsigned int ecx, unsigned int edx, 
    unsigned int esi, unsigned int edi, trap_frame_t *frame);
int sys_expmask(uint32_t *mask);
void *sys_exphandler(uint32_t code);

int exception_check_kernel(trap_frame_t *frame);

#endif   /* _XBOOK_SOFT_EXCEPTION_H */

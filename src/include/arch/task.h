#ifndef _ARCH_TASK_H
#define _ARCH_TASK_H

#include "general.h"

#define current_task_addr __current_task_addr

#define init_task_lock_stack __init_task_lock_stack

#define switch_to(prev, next) __switch_to((unsigned long)prev, (unsigned long)next)

#define user_trap_frame_init(frame) __user_trap_frame_init(frame)

/* 往栈框写入入口 */
#define user_entry_point(frame, entry) __user_entry_point(frame, entry) 

/* 从内核态切换到用户态 */
#define switch_to_user(frame)      __switch_to_user(frame)

#endif  /* _ARCH_TASK_H */

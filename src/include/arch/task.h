#ifndef _ARCH_TASK_H
#define _ARCH_TASK_H

#include "general.h"

#define current_task_addr __current_task_addr

#define init_task_lock_stack __init_task_lock_stack

#define switch_to(prev, next) __switch_to((unsigned long)prev, (unsigned long)next)

#endif  /* _ARCH_TASK_H */

#ifndef _XBOOK_TYPES_H
#define _XBOOK_TYPES_H


#include "config.h"

typedef long pid_t;
typedef unsigned long flags_t;
typedef unsigned long register_t;    //寄存器
typedef unsigned long off_t;         //偏移类型的变量
typedef unsigned long sector_t;
typedef unsigned char mode_t;
typedef unsigned long dev_t;
typedef unsigned long blksize_t;
typedef unsigned long blkcnt_t;
typedef unsigned long clock_t;

/* 在线程中作为形参 */
typedef void task_func_t(void *);

#endif  /* _XBOOK_TYPES_H */

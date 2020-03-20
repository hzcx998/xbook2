#ifndef _XBOOK_TYPES_H
#define _XBOOK_TYPES_H


#include "config.h"

#ifdef CONFIG_32BIT 

typedef int pid_t;
typedef unsigned int flags_t;
typedef unsigned int register_t;    //寄存器
typedef unsigned int address_t;     //地址类型的变量
typedef unsigned int off_t;         //偏移类型的变量
typedef unsigned int sector_t;
typedef unsigned char mode_t;
typedef unsigned int dev_t;
typedef unsigned int ino_t;
typedef unsigned int blksize_t;
typedef unsigned int blkcnt_t;
typedef unsigned long clock_t;

#endif

#ifdef CONFIG_64BIT

typedef long pid_t;
typedef unsigned long flags_t;
typedef unsigned long register_t;    //寄存器
typedef unsigned long address_t;     //地址类型的变量
typedef unsigned long off_t;         //偏移类型的变量
typedef unsigned long sector_t;
typedef unsigned char mode_t;
typedef unsigned long dev_t;
typedef unsigned long ino_t;
typedef unsigned long blksize_t;
typedef unsigned long blkcnt_t;

#endif 

#endif  /* _XBOOK_TYPES_H */

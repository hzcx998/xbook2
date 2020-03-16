#ifndef _XBOOK_TYPES_H
#define _XBOOK_TYPES_H


#include "config.h"

#ifdef CONFIG_32BIT 

typedef int pid_t;
typedef unsigned int flags_t;
typedef unsigned int register_t;    //寄存器
typedef unsigned int address_t;     //地址类型的变量
typedef unsigned int off_t;         //偏移类型的变量
typedef unsigned long sector_t;
typedef unsigned char mode_t;
typedef unsigned int dev_t;
typedef unsigned int ino_t;
typedef unsigned int blksize_t;
typedef unsigned int blkcnt_t;

#endif


#ifdef CONFIG_64BIT

#endif 

#endif  /* _XBOOK_TYPES_H */

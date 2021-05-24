#ifndef _XBOOK_SAFETY_H
#define _XBOOK_SAFETY_H

#include <stdint.h>

#define SAFETY_TINY

/* 不同平台需要实现对应的函数 */
int do_copy_from_user(void *dest, void *src, unsigned long nbytes);
int do_copy_to_user(void *dest, void *src, unsigned long nbytes);


/* 权限：资源（设备，ipc）访问、路径访问、 */

int safety_check_range(void *src, unsigned long nbytes);
int mem_copy_from_user(void *dest, void *src, unsigned long nbytes);
int mem_copy_to_user(void *dest, void *src, unsigned long nbytes);

#endif /* _XBOOK_SAFETY_H */

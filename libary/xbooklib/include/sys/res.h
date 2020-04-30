#ifndef _SYS_RES_H
#define _SYS_RES_H

#include <types.h>

/* 资源类型 */
#define RES_DEV    0x1000000
#define RES_IPC    0x2000000

#define RES_STDINNO   0
#define RES_STDOUTNO   1
#define RES_STDERRNO   2



int res_open(char *name, unsigned long flags, unsigned long arg);
int res_close(int res);
int res_write(int res, off_t off, void *buffer, size_t size);
int res_read(int res, off_t off, void *buffer, size_t size);
int res_ioctl(int res, unsigned int cmd, unsigned long arg);

#endif   /* _SYS_RES_H */
#ifndef _SYS_WAITQUE_H
#define _SYS_WAITQUE_H

/* 附带变量操作 */
#define WAITQUE_ADD     1
#define WAITQUE_SUB     2
#define WAITQUE_SET     3

int waitque_create();
int waitque_destroy(int handle);
int waitque_wait(int handle, int *addr, int wqflags, int value);
int waitque_wake(int handle, int *addr, int wqflags, int value);

#endif   /* _SYS_WAITQUE_H */
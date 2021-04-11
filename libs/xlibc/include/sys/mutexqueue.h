#ifndef _SYS_MUTEX_QUEUE_H
#define _SYS_MUTEX_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

/* 附带变量操作 */
#define MUTEX_QUEUE_ADD     1   /* 增加操作 */
#define MUTEX_QUEUE_SUB     2   /* 减少操作 */
#define MUTEX_QUEUE_SET     3   /* 设置变量值 */
#define MUTEX_QUEUE_ZERO    4   /* 置0 */
#define MUTEX_QUEUE_ONE     5   /* 置1 */
#define MUTEX_QUEUE_INC     6   /* +1 */
#define MUTEX_QUEUE_DEC     7   /* -1 */


/* 标志位 */
#define MUTEX_QUEUE_ALL     (1 << 16)  /* 唤醒所有等待者 */
#define MUTEX_QUEUE_TIMED   (1 << 17)  /* 有时间的等待 */

#define MUTEX_QUEUE_OPMASK    0xffff  /* 操作遮罩，低16位 */

int mutex_queue_alloc();
int mutex_queue_free(int handle);
int mutex_queue_wait(int handle, void *addr, unsigned int wqflags, unsigned long value);
int mutex_queue_wake(int handle, void *addr, unsigned int wqflags, unsigned long value);

#ifdef __cplusplus
}
#endif

#endif   /* _SYS_MUTEX_QUEUE_H */
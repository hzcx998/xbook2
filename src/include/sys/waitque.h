#ifndef _SYS_WAITQUE_H
#define _SYS_WAITQUE_H

/* 附带变量操作 */
#define WAITQUE_ADD     1   /* 增加操作 */
#define WAITQUE_SUB     2   /* 减少操作 */
#define WAITQUE_SET     3   /* 设置变量值 */
#define WAITQUE_ZERO    4   /* 置0 */
#define WAITQUE_ONE     5   /* 置1 */
#define WAITQUE_INC     6   /* +1 */
#define WAITQUE_DEC     7   /* -1 */

/* 标志位 */
#define WAITQUE_ALL     (1 << 16)  /* 唤醒所有等待者 */
#define WAITQUE_TIMED    (1 << 17)  /* 有时间的等待 */

#define WAITQUE_OPMASK    0xffff  /* 操作遮罩，低16位 */

#endif   /* _SYS_WAITQUE_H */
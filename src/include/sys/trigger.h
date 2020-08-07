#ifndef _SYS_TRIGGER_H
#define _SYS_TRIGGER_H

/* 触发器集 */
typedef unsigned long trigset_t; 

/* 触发器处理函数 */
typedef void (*trighandler_t) (int);

#define TRIG_NR         9   /* 触发器的数量 */

#define TRIGSYS         1   /* system trigger，系统触发器 */
#define TRIGDBG         2   /* debug trigger，调试触发器 */
#define TRIGPAUSE       3   /* pause trigger，暂停触发器 */
#define TRIGRESUM       4   /* resume trigger，恢复触发器 */
#define TRIGHSOFT       5   /* heavy software trigger，重软件触发器 */
#define TRIGLSOFT       6   /* light software trigger，轻软件触发器 */
#define TRIGUSR0        7  /* user trigger 0，用户自定义触发器 */
#define TRIGUSR1        8  /* user trigger 1，用户自定义触发器 */
#define TRIGALARM       9  /* user alarm trigger 用户闹钟触发器 */

#define TRIGMAX         TRIGALARM  /* 最大的触发器 */

/* 信号处理函数 */
#define TRIG_ERR     ((trighandler_t) -1)       /* 错误触发器 */
#define TRIG_DFL     ((trighandler_t) 0)        /* 默认触发器处理方式 */
#define TRIG_IGN     ((trighandler_t) 1)        /* 忽略触发器 */

/* 检测是否是错误的触发器 */
#define IS_BAD_TRIGGER(trig) \
    (trig < 1 || trig > TRIGMAX)
  
#define TA_ONCSHOT          (1 << 0)    /* 只执行一次 */
#define TA_NOMASK           (1 << 1)    /* 执行期间没有屏蔽 */
#define TA_NODEFFER         TA_NOMASK

/* trigprocmask的how参数值 */
#define TRIG_BLOCK   1 //在阻塞触发器集中加上给定的触发器集
#define TRIG_UNBLOCK 2 //从阻塞触发器集中删除指定的触发器集
#define TRIG_SETMASK 3 //设置阻塞触发器集(触发器屏蔽码)

/* 触发器行为 */
typedef struct {
    trighandler_t handler;      /* 行为处理函数 */
    unsigned long flags;        /* 行为标志 */
    trigset_t mask;             /* 屏蔽位 */
} trig_action_t;

int trigaddset(trigset_t *set, int trig);
int trigdelset(trigset_t *set, int trig);

int trigemptyset(trigset_t *set);
int trigfillset(trigset_t *set);

int trigismember(trigset_t *set, int trig);
int trigisfull(trigset_t *set);
int trigisempty(trigset_t *set);

int trigorset(trigset_t *set, trigset_t *setb);
int trigmask(int trig);

#endif   /* _SYS_TRIGGER_H */
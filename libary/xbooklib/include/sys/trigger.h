#ifndef _SYS_TRIGGER_H
#define _SYS_TRIGGER_H

#include <types.h>

/* 触发器集 */
typedef unsigned long trigset_t; 

/* 触发器处理函数 */
typedef void (*trighandler_t) (int);

#define TRIG_NR         8   /* 触发器的数量 */

#define TRIGHW          1   /* hardware trigger，硬件触发器 */
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
    (trig < 1 || trig > TRIGUSR1)
  
#define TA_ONCE          (1 << 0)    /* 只执行一次 */

/* 触发器行为 */
typedef struct {
    trighandler_t handler;      /* 行为处理函数 */
    unsigned long flags;        /* 行为标志 */
} trig_action_t;

int trigaddset(trigset_t *set, int trig);
int trigdelset(trigset_t *set, int trig);

int trigemptyset(trigset_t *set);
int trigfillset(trigset_t *set);

int trigismember(trigset_t *set, int trig);
int trigisfull(trigset_t *set);
int trigisempty(trigset_t *set);

int trigger(int trig, trighandler_t handler);
int trigger_action(int trig, trig_action_t *act, trig_action_t *oldact);
int triggeron(int trig, pid_t pid);


#endif   /* _SYS_TRIGGER_H */
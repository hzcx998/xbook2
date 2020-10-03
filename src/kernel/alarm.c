#include <xbook/alarm.h>
#include <xbook/schedule.h>
#include <xbook/clock.h>
#include <xbook/debug.h>
#include <xbook/trigger.h>

// #define DEBUG_ALARM

unsigned long sys_alarm(unsigned long second)
{
    task_t *cur = current_task;

    /* 需要返回上次剩余的秒数 */
    unsigned long old_second = cur->alarm.second;

    if (second == 0) {
        cur->alarm.flags = 0;       /* 取消闹钟 */
    } else {
        cur->alarm.flags = 1;       /* 定时器生效 */
        cur->alarm.second = second; /* 设定秒数 */
        cur->alarm.ticks = HZ;      /* 设置ticks数 */
#ifdef DEBUG_ALARM
        printk(KERN_DEBUG "sys_alarm: alarm seconds %d ticks %d\n", second, cur->alarm.ticks);
#endif    
    }
    return old_second; /* 返回上次剩余时间 */
}

void update_alarms()
{
    task_t *task;
    unsigned long flags; /* 遍历期间需要关闭中断 */
    save_intr(flags);
    list_for_each_owner (task, &task_global_list, global_list) {
        if (task->alarm.flags) {
            task->alarm.ticks--;
            if (!task->alarm.ticks) {
                task->alarm.second--;
                /* alarmTicks和时钟相关 */
                task->alarm.ticks = HZ; 
                /* 如果时间结束，那么就发送SIGALRM信号 */
                if (!task->alarm.second) {
#ifdef DEBUG_ALARM
                    printk(KERN_DEBUG "update_alarms: task %d do alarm.\n", task->pid);
#endif
                    sys_trigger_active(TRIGALARM, task->pid);
                    task->alarm.flags = 0;
                }
            }
        }
    }
    restore_intr(flags);
}
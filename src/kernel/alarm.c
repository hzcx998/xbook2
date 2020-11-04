#include <xbook/alarm.h>
#include <xbook/schedule.h>
#include <xbook/clock.h>
#include <xbook/debug.h>
#include <xbook/trigger.h>

unsigned long sys_alarm(unsigned long second)
{
    task_t *cur = current_task;
    unsigned long old_second = cur->alarm.second;
    if (second == 0) {
        cur->alarm.flags = 0;
    } else {
        cur->alarm.flags = 1;
        cur->alarm.second = second;
        cur->alarm.ticks = HZ;
    }
    return old_second;
}

void alarm_update_ticks()
{
    task_t *task;
    unsigned long flags;
    interrupt_save_state(flags);
    list_for_each_owner (task, &task_global_list, global_list) {
        if (task->alarm.flags) {
            task->alarm.ticks--;
            if (!task->alarm.ticks) {
                task->alarm.second--;
                task->alarm.ticks = HZ;
                if (!task->alarm.second) {
                    sys_trigger_active(TRIGALARM, task->pid);
                    task->alarm.flags = 0;
                }
            }
        }
    }
    interrupt_restore_state(flags);
}
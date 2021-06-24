#include <xbook/alarm.h>
#include <xbook/schedule.h>
#include <xbook/clock.h>
#include <xbook/debug.h>
#include <xbook/signal.h>
#include <xbook/safety.h>
#include <errno.h>

unsigned long sys_alarm(unsigned long second)
{
    task_t *cur = task_current;
    unsigned long old_second = TICKS_to_MSEC(cur->alarm.ticks) / 1000;
    if (second == 0) {
        cur->alarm.flags = ALARM_INACITVE;
        cur->alarm.ticks = 0;
    } else {
        cur->alarm.flags = ALARM_ACITVE;
        cur->alarm.ticks = MSEC_TO_TICKS(second * 1000);
    }
    cur->alarm.which = ITIMER_REAL;
    return old_second;
}

void alarm_update_ticks()
{
    task_t *cur = task_current;
    alarm_t *alarm;
    
    task_t *task;
    unsigned long flags;
    interrupt_save_and_disable(flags);
    list_for_each_owner (task, &task_global_list, global_list) {
        if (task->state != TASK_STOPPED && task->state != TASK_HANGING && task->state != TASK_ZOMBIE) {
            alarm = &task->alarm;
            if (alarm->flags > 0) {
                alarm->ticks--;
                if (alarm->ticks <= 0) {
#ifdef CONFIG_SIGNAL
                    switch (alarm->which) {
                    case ITIMER_REAL:
                        do_send_signal(task->pid, SIGALRM, cur->pid);
                        break;
                    case ITIMER_VIRTUAL:
                        /* FIXME: 对用户态进行计时 */
                        do_send_signal(task->pid, SIGVTALRM, cur->pid);
                        break;
                    case ITIMER_PROF:
                        /* FIXME: 对用户态和内核态进行计时 */
                        do_send_signal(task->pid, SIGPROF, cur->pid);
                        break;
                    default:
                        errprint("[alarm] unknown itimer type %d\n", alarm->which);                    
                        break;
                    }
#else
                    exception_send(task->pid, EXP_CODE_ALRM);
#endif
                    /* 触发完信号后，再判断是否有间隔，如果有间隔的话，就重新设置ticks的值 */
                    unsigned long ms = alarm->itimer.it_interval.tv_sec * 1000 + alarm->itimer.it_interval.tv_usec / 1000;
                    if (ms > 0) {
                        alarm->ticks = MSEC_TO_TICKS(ms);
                    } else {
                        alarm->flags = ALARM_INACITVE;  /* stop alarm */
                        alarm->ticks = 0;
                    }
                }
            }
        }
    }
    interrupt_restore_state(flags);
}

int sys_getitimer(int which, struct itimerval *value)
{
    if (!value)
        return -EFAULT;
    task_t *cur = task_current;
    alarm_t *alarm = &cur->alarm;
    switch (which) {
    case ITIMER_REAL:
    case ITIMER_VIRTUAL:
    case ITIMER_PROF:
        if (mem_copy_to_user(value, &alarm->itimer, sizeof(struct itimerval)) < 0)
            return -EFAULT;
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

int sys_setitimer(int which, const struct itimerval *new_value,
    struct itimerval *old_value)
{
    if (!new_value)
        return -EFAULT;
    int err = 0;
    if (old_value) {
        err = sys_getitimer(which, old_value);
        if (err < 0)
            return err;
    }
    task_t *cur = task_current;
    alarm_t *alarm = &cur->alarm;
    
    switch (which) {
    case ITIMER_REAL:
    case ITIMER_VIRTUAL:
    case ITIMER_PROF:
        if (mem_copy_from_user(&alarm->itimer, (void *)new_value, sizeof(struct itimerval)) < 0)
            return -EFAULT;

        /* 检测是否为停止闹钟 */
        if (alarm->itimer.it_value.tv_sec == 0 && alarm->itimer.it_value.tv_usec == 0 &&
            alarm->itimer.it_interval.tv_sec == 0 && alarm->itimer.it_interval.tv_usec == 0) {
            alarm->flags = ALARM_INACITVE;   /* stop alarm */
            alarm->ticks = 0;
        } else {
            alarm->flags = ALARM_ACITVE;
            unsigned long ms = alarm->itimer.it_value.tv_sec * 1000 + alarm->itimer.it_value.tv_usec / 1000;
            alarm->ticks = MSEC_TO_TICKS(ms);
        }
        break;
    default:
        return -EINVAL;
    }
    alarm->which = which;
    return 0;
}
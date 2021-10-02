#include <dwin/dwin.h>
#include <dwin/hal.h>

/* xbook kernel header */
#include <xbook/task.h>
#include <xbook/schedule.h>

static int thread_start(struct dwin_thread *thread, void (*entry)(void *), void *arg)
{
    task_t *task = task_create(DWIN_THREAD_NAME, TASK_PRIO_LEVEL_NORMAL, entry, arg);
    if (task == NULL)
    {
        return -1;
    }
    return 0;
}

static int thread_stop(struct dwin_thread *thread, void *arg)
{
    task_exit((int)arg);    
    return 0;
}

struct dwin_hal_thread __kdevice_thread_hal = {
    .start = thread_start,
    .stop = thread_stop,
    .extension = NULL,
};

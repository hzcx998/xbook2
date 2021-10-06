#include <dwin/dwin.h>
#include <dwin/hal.h>

/* xbook kernel header */
#include <xbook/task.h>
#include <xbook/schedule.h>

static struct dwin_thread *start(void (*entry)(void *), void *arg)
{
    struct dwin_thread *thread = mem_alloc(sizeof(struct dwin_thread));
    if (thread == NULL)
    {
        return NULL;
    }

    task_t *task = task_create(DWIN_THREAD_NAME, TASK_PRIO_LEVEL_NORMAL, entry, arg);
    if (task == NULL)
    {
        mem_free(thread);
        return NULL;
    }
    thread->object = thread;
    return thread;
}

static int stop(struct dwin_thread *thread, void *arg)
{
    if (thread->object == NULL)
    {
        return -1;
    }
    
    thread->object = NULL;
    mem_free(thread);

    task_exit((int)arg);
    return 0;
}

struct dwin_hal_thread __kdevice_thread_hal = {
    .start = start,
    .stop = stop,
    .extension = NULL,
};

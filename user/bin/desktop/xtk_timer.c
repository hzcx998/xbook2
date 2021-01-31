#include "xtk_timer.h"
#include <stdlib.h>

xtk_timer_t *xtk_timer_create(uint32_t interval, xtk_timer_callback_t function, void *data)
{
    xtk_timer_t *timer = malloc(sizeof(xtk_timer_t));
    if (!timer)
        return NULL;
    timer->interval = interval;
    timer->callback = function;
    timer->calldata = data;
    list_init(&timer->list);
    timer->timer_id = 0;
    return timer;
}

int xtk_timer_destroy(xtk_timer_t *timer)
{
    if (!timer)
        return -1;
    free(timer);
    return 0;
}

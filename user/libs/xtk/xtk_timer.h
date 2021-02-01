#ifndef _LIB_XTK_TIMER_H
#define _LIB_XTK_TIMER_H

#include <stdbool.h>
#include <stdint.h>
#include "xtk_spirit.h"

/*bool timeout(xtk_spirit_t *spirit, uint32_t id, void *data) */
typedef bool (*xtk_timer_callback_t) (xtk_spirit_t *, uint32_t, void *);

typedef struct {
    list_t list;                    /* 定时器组成的链表 */
    uint32_t timer_id;              /* 定时器id */
    uint32_t interval;              /* 定时器的时间间隔（ms为单位） */
    xtk_timer_callback_t callback;
    xtk_timer_callback_t calldata;
} xtk_timer_t;

xtk_timer_t *xtk_timer_create(uint32_t interval, xtk_timer_callback_t function, void *data);
int xtk_timer_destroy(xtk_timer_t *timer);

#endif /* _LIB_XTK_TIMER_H */
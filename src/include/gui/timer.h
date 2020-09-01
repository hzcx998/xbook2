#ifndef _GUI_TIMER_H
#define _GUI_TIMER_H

#include <stdint.h>
#include <xbook/timer.h>
#include <list.h>

typedef struct {
    list_t list;    /* 定时器链表 */
    int layer;      /* 定时器对应的图层 */
    timer_t timer;    /* 定时器 */
} gui_timer_t;

uint32_t sys_gui_new_timer(uint32_t msec, int arg);
int sys_gui_modify_timer(uint32_t timer, uint32_t msec);
int sys_gui_del_timer(uint32_t timer);
int gui_timer_del_by_layer(int layer_id);

#endif  /* _GUI_TIMER_H */

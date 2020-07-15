#include <malloc.h>
#include <stddef.h>

#include <event/event.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>

gui_event_pool *event_pool;

/* 添加一个事件 */
int gui_event_add(gui_event *ev)
{
    /* 检查事件池满 */
    int next = (event_pool->head + 1) % GUI_EVENT_NR;
    if (next == event_pool->tail) { /* 事件池满，丢弃 */
        return -1;
    }
    /* 放入事件 */
    gui_event *tmp = &event_pool->events[event_pool->head];
    *tmp = *ev;
    event_pool->head = (event_pool->head + 1) % GUI_EVENT_NR;
    return 0;
}

/* 添加一个事件 */
int gui_event_del(gui_event *ev)
{
    /* 判断事件池情况 */
    if (GUI_EVENT_POOL_EMPTY(event_pool)) {
        return -1;
    }
    /* 放入事件 */
    gui_event *tmp = &event_pool->events[event_pool->tail];
    *ev = *tmp;
    event_pool->tail = (event_pool->tail + 1) % GUI_EVENT_NR;
    return 0;
}

/* 轮询事件 */
int gui_event_poll(gui_event *ev)
{
    /* 读取事件 */
    drv_keyboard.read();
    drv_mouse.read();
    
    /* 从事件池中删除一个事件，并传输 */
    return gui_event_del(ev);
}

int init_event()
{
    event_pool = malloc(sizeof(gui_event_pool));
    if (event_pool == NULL) {
        return -1;
    }
    event_pool->head = event_pool->tail = 0;
    return 0;
}
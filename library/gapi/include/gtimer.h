#ifndef _GAPI_TIMER_H
#define _GAPI_TIMER_H

#include <stddef.h>
#include <stdint.h>
#include <sys/list.h>

#include "gmessage.h"
// 定时器间隔的最大值和最小值
#define G_TIMER_MINIMUM 0x0000000A
#define G_TIMER_MAXIMUM 0x7FFFFFFF

/**
 * @layer: 定时器所在的图层句柄
 * @msg: 定时器消息
 * @id: 定时器id
 * @time: 系统启动以来的毫秒数 
 * 
 */
typedef void (*g_timer_handler_t) (int, uint32_t, uint32_t, uint32_t);

typedef struct {
    list_t list;    /* 定时器链表 */
    uint32_t id;    /* 定时器id */
    uint32_t timer; /* 内核定时器 */
    int layer;      /* 和定时器绑定的图层（窗口） */
    g_timer_handler_t handler;  /* 回调函数 */
} g_timer_t;

/**
 * g_set_timer - 设置定时器
 * @layer: 图层句柄
 * @id: 定时器id
 * @msec: 定时间隔（毫秒为单位）
 * @handler: 定时器超时的回调函数
 * 
 * 如果layer为-1，并且id为0，那么就返回一个系统默认的定时器id。
 * 如果layer为正，并且id不为0，并成功创建，返回对应的id值
 * 如果创建失败，那么返回0
 */
uint32_t g_set_timer(int layer, uint32_t id, uint32_t msec, g_timer_handler_t handler);

/**
 * g_del_timer - 删除定时器
 * @layer: 图层句柄
 * @id: 定时器id
 * 
 * 成功返回0，失败返回-1
 */
int g_del_timer(int layer, uint32_t id);

int g_del_timer_all();

g_timer_t *g_timer_find_by_timer(uint32_t timer);
#endif /* _GAPI_TIMER_H */
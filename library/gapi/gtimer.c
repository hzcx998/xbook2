#include <gtimer.h>
#include <malloc.h>
#include <sys/syscall.h>

/* 从1开始，0是无效的id */
uint32_t _g_timer_next_id = 1; 

LIST_HEAD(_g_timer_list_head); /* 定时器链表头 */

uint32_t syscall_new_timer(uint32_t msec, int layer)
{
    return syscall2(uint32_t, SYS_GNEWTIMER, msec, layer);
}

int syscall_modify_timer(uint32_t timer, uint32_t msec)
{
    return syscall2(int, SYS_GMODIFYTIMER, timer, msec);
}

int syscall_del_timer(uint32_t timer)
{
    return syscall1(int, SYS_GDELTIMER, timer);
}

/**
 * 通过id查找定时器
 * 找到返回定时器结构指针，没找到返回NULL
 */
static g_timer_t *__g_timer_find_by_id(uint32_t id)
{
    g_timer_t *tmr;
    list_for_each_owner (tmr, &_g_timer_list_head, list) {
        if (tmr->id == id)
            return tmr;
    }
    return NULL;
}

/**
 * 通过内核timer查找定时器
 * 找到返回定时器结构指针，没找到返回NULL
 */
g_timer_t *g_timer_find_by_timer(uint32_t timer)
{
    g_timer_t *tmr;
    list_for_each_owner (tmr, &_g_timer_list_head, list) {
        if (tmr->timer == timer)
            return tmr;
    }
    return NULL;
}



g_timer_t *g_new_timer(int layer, uint32_t id, uint32_t msec, g_timer_handler_t handler)
{
    g_timer_t *tmr;
    tmr = malloc(sizeof(g_timer_t));
    if (tmr == NULL) {
        return NULL;
    }
    tmr->id = id;
    tmr->layer = layer;
    tmr->handler = handler;
    tmr->timer = syscall_new_timer(msec, layer);
    if (!tmr->timer) {
        free(tmr);
        return NULL;
    }
    list_add_tail(&tmr->list, &_g_timer_list_head);
    return tmr;
}

uint32_t g_set_timer(int layer, uint32_t id, uint32_t msec, g_timer_handler_t handler)
{
    if (layer < 0) /* 没有指定图层，设置失败 */
        return 0;
    g_timer_t *tmr;
    uint32_t tmrid;
    
    /* 对定时器的间隔进行修复 */
    if (msec < G_TIMER_MINIMUM)
        msec = G_TIMER_MINIMUM;
    if (msec > G_TIMER_MAXIMUM)
        msec = G_TIMER_MAXIMUM;

    if (!id) {   /* 分配一个默认的定时器 */
        do {    
            /* 检测是否有重复的定时器id */
            tmrid = _g_timer_next_id;
            tmr = __g_timer_find_by_id(tmrid);
            if (tmr) {  /* 找到了定时器，就需要更新默认的定时器id值 */
                _g_timer_next_id++;
            }
        } while (tmr != NULL);
    } else {    /* 分配一个指定id的定时器 */
        tmrid = id;
        tmr = __g_timer_find_by_id(tmrid);
        if (tmr) {  /* 找到了定时器，就是重新设置定时器超时和回调函数 */
            tmr->handler = handler;
            if (syscall_modify_timer(tmr->timer, msec) < 0) /* 修改超时间隔 */
                return 0;
            return tmr->id;
        }
    }
    /* 现在找到一个可用的定时器id，创建新的定时器 */
    tmr = g_new_timer(layer, tmrid, msec, handler);
    if (tmr == NULL) {
        return 0;   /* 创建失败 */
    }
    return tmr->id;
}

/**
 * 根据图层和id删除定时器
 * 成功返回0，失败返回-1
 */
int g_del_timer(int layer, uint32_t id)
{
    if (layer < 0 || !id)
        return -1;

    g_timer_t *tmr;
    tmr = __g_timer_find_by_id(id);
    if (tmr == NULL)
        return -1;
    if (tmr->id == id && tmr->layer == layer) {
        if (!syscall_del_timer(tmr->timer)) {
            list_del(&tmr->list);
            free(tmr);
            return 0;
        }
    }
    return -1;
}

int g_del_timer_all()
{
    g_timer_t *tmr, *next;
    list_for_each_owner_safe (tmr, next, &_g_timer_list_head, list) {
        if (!syscall_del_timer(tmr->timer)) {
            list_del(&tmr->list);
            free(tmr);
        } else {
            return -1;
        }
    }
    return 0;
}
#include <gui/timer.h>
#include <gui/message.h>
#include <gui/layer.h>
#include <xbook/kmalloc.h>
#include <xbook/clock.h>
#include <xbook/debug.h>

static LIST_HEAD(gui_timer_list_head);

void gui_timer_handler(unsigned long arg)
{
    /* 向图层发送GM_TIMER消息 */
    //printk("[gui]: timer occur, arg=%x\n", arg);
    gui_timer_t *gtmr = (gui_timer_t *) arg;
    if (gtmr == NULL)
        return;

    layer_t *layer = layer_find_by_id_without_lock(gtmr->layer);
    if (layer == NULL)
        return;

    task_t *task = (task_t *) layer->extension;
    if (task) {       
        g_msg_t m;
        m.id = GM_TIMER;
        m.target = (int) gtmr->layer;
        m.data0 = gtmr->timer.id;           /* 定时器id */ 
        m.data1 = TICKS_to_MSEC(sys_get_ticks()); /* 系统启动以来经过的的毫秒数 */
        msgpool_try_push(task->gmsgpool, &m);
    }
}

/**
 * 窗口一个新的定时器
 * 
 * 成功返回定时器id值，失败返回0
 */
uint32_t sys_gui_new_timer(uint32_t msec, int arg)
{
    gui_timer_t *gtmr = kmalloc(sizeof(gui_timer_t));
    if (gtmr == NULL)
        return 0;
    gtmr->layer = arg;
    list_add(&gtmr->list, &gui_timer_list_head);
    /* 将毫秒值转换成定时器ticks */
    clock_t ticks = MSEC_TO_TICKS(msec);
    if (ticks < 3)  /* 至少得有3个ticks */
        ticks = 3;
    timer_init(&gtmr->timer, ticks, (unsigned long) gtmr, gui_timer_handler);
    timer_add(&gtmr->timer);
    //printk("set timer %d ticks %d\n", gtmr->timer.id, ticks);
    return gtmr->timer.id;
}

gui_timer_t *gui_timer_find_by_id(uint32_t timer)
{
    gui_timer_t *gtmr;
    list_for_each_owner (gtmr, &gui_timer_list_head, list) {
        if (gtmr->timer.id == timer)
            return gtmr;
    }
    return NULL;
}

int gui_timer_del_by_layer(int layer_id)
{
    gui_timer_t *gtmr, *next;
    list_for_each_owner_safe (gtmr, next, &gui_timer_list_head, list) {
        if (gtmr->layer == layer_id) {
            sys_gui_del_timer(gtmr->timer.id);
        }
    }
    return 0;
}


int sys_gui_modify_timer(uint32_t timer, uint32_t msec)
{
    gui_timer_t *gtmr;
    gtmr = gui_timer_find_by_id(timer);
    if (gtmr == NULL)
        return -1;
    clock_t ticks = MSEC_TO_TICKS(msec);
    timer_mod(&gtmr->timer, ticks);
    if (!timer_alive(&gtmr->timer)) { /* 修改后，如果没在定时器链表中，就添加进去 */
        timer_add(&gtmr->timer);
    }
    return 0;
}

int sys_gui_del_timer(uint32_t timer)
{
    gui_timer_t *gtmr;
    gtmr = gui_timer_find_by_id(timer);
    if (gtmr == NULL)
        return -1;
    if (timer_cancel(&gtmr->timer) < 0)
        return -1;
    list_del(&gtmr->list);
    kfree(gtmr);
    return 0;
}

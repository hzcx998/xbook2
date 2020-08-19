#include <xbook/msgpool.h>
#include <xbook/task.h>
#include <gui/message.h>
#include <gui/message.h>

msgpool_t *gui_msgpool = NULL;

int gui_init_msg()
{
    gui_msgpool = msgpool_create(sizeof(g_msg_t), GUI_MSG_NR);
    if (gui_msgpool == NULL)
        return -1;
    return 0;
}

int gui_push_msg(g_msg_t *msg)
{
    if (msgpool_full(gui_msgpool) > 0)
        return -1;
    return msgpool_push(gui_msgpool, msg);
}

int gui_pop_msg(g_msg_t *msg)
{
    if (msgpool_empty(gui_msgpool) > 0)
        return -1;
    return msgpool_pop(gui_msgpool, msg);
}

/**
 * 获取消息，如果没有就阻塞等待
 * 
 */
int sys_g_get_msg(g_msg_t *msg)
{
    task_t *cur = current_task;
    if (!cur->gmsgpool)
        return -1;
    return msgpool_pop(cur->gmsgpool, msg);
}

/**
 * 尝试获取消息，如果没有就返回-1
 * 
 */
int sys_g_try_get_msg(g_msg_t *msg)
{
    task_t *cur = current_task;
    if (!cur->gmsgpool)
        return -1;
    return msgpool_try_pop(cur->gmsgpool, msg);
}

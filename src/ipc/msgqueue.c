#include <xbook/msgqueue.h>
#include <xbook/debug.h>
#include <xbook/safety.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sys/ipc.h>

msg_queue_t *msg_queue_table;
DEFINE_SEMAPHORE(msg_queue_mutex, 1);

static msg_queue_t *msg_queue_find_by_name(char *name)
{
    msg_queue_t *msgq;
    int i;
    for (i = 0; i < MSGQ_MAX_NR; i++) {
        msgq = &msg_queue_table[i];
        if (msgq->name[0] != '\0') { 
            if (!strcmp(msgq->name, name)) {
                return msgq;
            }
        }
    }
    return NULL;
}

static msg_queue_t *msg_queue_find_by_id(int msgid)
{
    msg_queue_t *msgq;
    int i;
    for (i = 0; i < MSGQ_MAX_NR; i++) {
        msgq = &msg_queue_table[i];
        if (msgq->id == msgid && msgq->name[0] != '\0') { 
            return msgq;
        }
    }
    return NULL;
}

msg_queue_t *msg_queue_alloc(char *name)
{
    msg_queue_t *msgq;
    int i;
    for (i = 0; i < MSGQ_MAX_NR; i++) {
        msgq = &msg_queue_table[i];
        if (msgq->name[0] == '\0') {
            memcpy(msgq->name, name, MSGQ_NAME_LEN);
            msgq->name[MSGQ_NAME_LEN - 1] = '\0';
            INIT_LIST_HEAD(&msgq->msg_list);
            msgq->msgs = 0;
            msgq->msgsz = MSG_MAX_LEN;
            wait_queue_init(&msgq->senders);
            wait_queue_init(&msgq->receivers);
            return msgq;
        }
    }
    return NULL;
}

int msg_queue_free(msg_queue_t *msgq)
{
    memset(msgq->name, 0, MSGQ_NAME_LEN);
    return 0;
}

/**
 * @flags: 获取标志
 *         IPC_CREAT: 如果消息队列不存在，则创建一个新的消息队列，否则就打开
 *         IPC_EXCL:  和CREAT一起使用，则要求创建一个新的消息队列，若已存在，就返回-1。
 *                    相当于在CREAT上面加了一个必须不存在的限定。
 * @return: 成功返回消息队列id，失败返回-1
 */
int msg_queue_get(char *name, unsigned long flags)
{
    if (name == NULL)
        return -1;
    char craete_new = 0;
    msg_queue_t *msgq;
    int retval = -1;
    semaphore_down(&msg_queue_mutex);
    if (flags & IPC_CREAT) {
        if (flags & IPC_EXCL) {
            craete_new = 1;
        }
        msgq = msg_queue_find_by_name(name);
        if (msgq) {
            if (craete_new) {
                goto err;
            }
            retval = msgq->id;
        } else {
            msgq = msg_queue_alloc(name);
            if (msgq == NULL) {
                goto err;
            }
            retval = msgq->id;
        }
    }
err:
    semaphore_up(&msg_queue_mutex);
    return retval;
}

int msg_queue_put(int msgid)
{
    msg_queue_t *msgq;
    semaphore_down(&msg_queue_mutex);
    msgq = msg_queue_find_by_id(msgid);
    if (msgq) {
        msg_queue_free(msgq);
        semaphore_up(&msg_queue_mutex);        
        return 0;
    }
    semaphore_up(&msg_queue_mutex);
    return -1;
}

void msg_init(msg_t *msg, long type, void *text, size_t length)
{
	INIT_LIST_HEAD(&msg->list);
	msg->type = type;
	msg->length = length;
	memcpy(msg->buf, text, length);
}

/**
 * @size: 消息大小，不包括long int 的type。
 * @msgflg: 消息标志，IPC_NOWAIT表示队列满不等待，否则就要等待。
 * @return: 成功返回0，失败返回-1
 */
int msg_queue_send(int msgid, void *msgbuf, size_t size, int msgflg)
{
    msg_queue_t *msgq;
    semaphore_down(&msg_queue_mutex);
    msgq = msg_queue_find_by_id(msgid);
    if (msgq == NULL) {
        semaphore_up(&msg_queue_mutex);
        printk(KERN_ERR "msg_queue_send: not found message queue!\n");
        return -1;
    }
    semaphore_up(&msg_queue_mutex);
    semaphore_down(&msgq->mutex);
    if (size > msgq->msgsz) {
        size = msgq->msgsz;
    }
    if (msgq->msgs > MSGQ_MAX_MSGS) {
        if (msgflg & IPC_NOWAIT) {
            semaphore_up(&msgq->mutex);
            return -1;
        }
        wait_queue_add(&msgq->senders, task_current);
        semaphore_up(&msgq->mutex);
        TASK_ENTER_WAITLIST(task_current);
        task_block(TASK_BLOCKED);
        semaphore_down(&msgq->mutex);
    }
    msg_t *msg = mem_alloc(sizeof(msg_t) + size);
    if (msg == NULL) {
        semaphore_up(&msgq->mutex);
        return -1;
    }
    long *msg_header = (long *) msgbuf;
    msg_init(msg, *msg_header, msg_header + 1, size);
    list_add(&msg->list, &msgq->msg_list);
    msgq->msgs++;
    wait_queue_wakeup(&msgq->receivers);
    semaphore_up(&msgq->mutex);
    return 0;
}

/**
 * @msgsz: 消息大小，不包括long int 的type。
 * @msgtype: 消息类型，实现接收优先级
 *          =0：表示返回队列里的第一条消息
 *          >0：返回队列第一条类型等于msgtype的消息。
 *          <0：返回队列第一条类型小于等于msgtype绝对值的消息。
 * @msgflg: 消息标志：
 *          IPC_NOWAIT：队列没有可读消息不等待，返回错误
 *          IPC_NOERROR：消息大小超过size时被截断
 *          msgtype>0且msgflg=IPC_EXCEPT：接收类型不等于msgtype的第一条消息
 * @return: 成功返回实际接收的数据量，失败返回-1
 */
int msg_queue_recv(int msgid, void *msgbuf, size_t msgsz, long msgtype, int msgflg)
{
    msg_queue_t *msgq;
    semaphore_down(&msg_queue_mutex);
    msgq = msg_queue_find_by_id(msgid);
    if (msgq == NULL) {
        semaphore_up(&msg_queue_mutex);    
        printk(KERN_DEBUG "msg_queue_recv: not found message queue!\n");
        return -1;
    }   
    semaphore_up(&msg_queue_mutex);
    semaphore_down(&msgq->mutex);
    if (!msgq->msgs) {
        if (msgflg & IPC_NOWAIT) {
            semaphore_up(&msgq->mutex); 
            return -1;
        }
        wait_queue_add(&msgq->receivers, task_current);
        semaphore_up(&msgq->mutex);
        TASK_ENTER_WAITLIST(task_current);
        task_block(TASK_BLOCKED);
        semaphore_down(&msgq->mutex);
    }
    msg_t *msg = NULL, *tmp;
    if (msgtype > 0) {
        if (msgflg & IPC_EXCEPT) {
            list_for_each_owner (tmp, &msgq->msg_list, list) {
                if (tmp->type != msgtype) {
                    msg = tmp;
                    break;
                }
            }
        } else {
            list_for_each_owner (tmp, &msgq->msg_list, list) {
                if (tmp->type == msgtype) {
                    msg = tmp;
                    break;
                }
            }
        }
    } else if (msgtype == 0) {
        msg = list_first_owner_or_null(&msgq->msg_list, msg_t, list);
    } else {
        msgtype = ABS(msgtype);
        list_for_each_owner (tmp, &msgq->msg_list, list) {
            if (tmp->type <= msgtype) {
                msg = tmp;
                break;
            }
        }
    }
    if (msg == NULL) {
        semaphore_up(&msgq->mutex);
        printk(KERN_ERR "msg_queue_recv: message not found!\n");
        return -1;
    }
    list_del(&msg->list);
    msgq->msgs--;
    unsigned short len = msg->length;
    if (msgflg & IPC_NOERROR) { 
        if (len > msgsz) {
            len = msgsz;
        }
    }
    long *msg_header = (long *) msgbuf;
    *msg_header = msg->type;
    memcpy((void *)(msg_header + 1), msg->buf, len);
    mem_free(msg);
    wait_queue_wakeup(&msgq->senders);
    semaphore_up(&msgq->mutex);
    return len;
}

int sys_msgque_get(char *name, unsigned long flags)
{
    if (!name)
        return -EINVAL;
    if (mem_copy_from_user(NULL, name, MSGQ_NAME_LEN) < 0)
        return -EINVAL;
    return msg_queue_get(name, flags);
}

int sys_msgque_put(int msgid)
{
    return msg_queue_put(msgid);
}

int sys_msgque_send(int msgid, void *msgbuf, size_t size, int msgflg)
{
    if (!msgbuf)
        return -EINVAL;
    if (mem_copy_from_user(NULL, msgbuf, size) < 0)
        return -EINVAL;
    return msg_queue_send(msgid, msgbuf, size, msgflg);
}

int sys_msgque_recv(int msgid, void *msgbuf, size_t msgsz, int msgflg)
{
    if (!msgbuf)
        return -EINVAL;
    if (mem_copy_to_user(msgbuf, NULL, msgsz) < 0)
        return -EINVAL;
    long *msgtype = (long *) msgbuf;
    return msg_queue_recv(msgid, msgbuf, msgsz, *msgtype, msgflg);
}

void msg_queue_init()
{
    msg_queue_table = (msg_queue_t *)mem_alloc(sizeof(msg_queue_t) * MSGQ_MAX_NR);
    if (msg_queue_table == NULL)
        panic(KERN_EMERG "msg_queue_init: alloc mem for msg_queue_table failed! :(\n");
    int i;
    for (i = 0; i < MSGQ_MAX_NR; i++) {
        msg_queue_table[i].id = 1 + i + i * 2;
        INIT_LIST_HEAD(&msg_queue_table[i].msg_list);
        msg_queue_table[i].msgs = 0;
        semaphore_init(&msg_queue_table[i].mutex, 1);
        memset(msg_queue_table[i].name, 0, MSGQ_NAME_LEN);
    }
}

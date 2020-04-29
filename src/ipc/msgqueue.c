#include <xbook/msgqueue.h>
#include <xbook/debug.h>
#include <xbook/string.h>
#include <xbook/math.h>
#include <sys/ipc.h>

/* debug msgq : 1 enable, 0 disable */
#define DEBUG_MSGQ 0

/* 消息队列表 */ 
msg_queue_t *msg_queue_table;

/* 保护消息队列的分配与释放 */
DEFINE_SEMAPHORE(msg_queue_mutex, 1);

/**
 * msg_queue_find_by_name - 通过名字查找消息队列  
 * @name: 消息队列的名字
 * 
 * @return: 如果消息队列已经在消息队列表中，就返回消息队列指针，
 *          没有则返回NULL
 */
static msg_queue_t *msg_queue_find_by_name(char *name)
{
    msg_queue_t *msgq;
    int i;
    for (i = 0; i < MSGQ_MAX_NR; i++) {
        msgq = &msg_queue_table[i];
        if (msgq->name[0] != '\0') { /* 有名字才进行比较 */
            if (!strcmp(msgq->name, name)) {
                return msgq;
            }
        }
    }
    return NULL;
}

/**
 * msg_queue_find_by_id - 通过id查找消息队列
 * @id: 消息队列的id
 * 
 * @return: 如果消息队列已经在消息队列表中，就返回消息队列指针，
 *          没有则返回NULL
 */
static msg_queue_t *msg_queue_find_by_id(int msgid)
{
    msg_queue_t *msgq;
    int i;
    for (i = 0; i < MSGQ_MAX_NR; i++) {
        msgq = &msg_queue_table[i];
        /* id相同并且正在使用，才找到 */
        if (msgq->id == msgid && msgq->name[0] != '\0') { 
            return msgq;
        }
    }
    return NULL;
}

/**
 * msg_queue_alloc - 分配一个消息队列
 * @name: 名字
 * @flags: 标志
 * 
 * 从消息队列表中分配一个消息队列
 * 
 * @return: 成功返回消息队列结构的地址，失败返flags回NULL
 */
msg_queue_t *msg_queue_alloc(char *name)
{
    msg_queue_t *msgq;
    int i;
    for (i = 0; i < MSGQ_MAX_NR; i++) {
        msgq = &msg_queue_table[i];
        if (msgq->name[0] == '\0') { /* 没有名字才使用 */
            /* 设置消息队列名字 */
            memcpy(msgq->name, name, MSGQ_NAME_LEN);
            msgq->name[MSGQ_NAME_LEN - 1] = '\0';
            INIT_LIST_HEAD(&msgq->msg_list);
            msgq->msgs = 0;
            msgq->msgsz = MSG_MAX_LEN;  /* 默认消息长度 */
            /* 发送者和接受者等待队列 */
            wait_queue_init(&msgq->senders);
            wait_queue_init(&msgq->receivers);
#if DEBUG_MSGQ == 1
            printk(KERN_DEBUG "msg_queue_alloc: msgq id=%d\n", msgq->id);
#endif
            return msgq; /* 返回消息队列 */
        }
    }
    return NULL;
}

/**
 * msg_queue_free - 释放一个消息队列
 * @msgq: 消息队列
 * 
 * @return: 成功返回0，失败返回-1
 */
int msg_queue_free(msg_queue_t *msgq)
{
#if DEBUG_MSGQ == 1    
    printk(KERN_DEBUG "msg_queue_free: msgq id=%d msgs=%d\n", msgq->id, msgq->msgs);
#endif
    memset(msgq->name, 0, MSGQ_NAME_LEN);

    return 0;
}

/**
 * msg_queue_get - 获取一个消息队列
 * 
 * @name: 消息队列名
 * @flags: 获取标志
 *         IPC_CREAT: 如果消息队列不存在，则创建一个新的消息队列，否则就打开
 *         IPC_EXCL:  和CREAT一起使用，则要求创建一个新的消息队列，若已存在，就返回-1。
 *                    相当于在CREAT上面加了一个必须不存在的限定。
 * 
 * @return: 成功返回消息队列id，失败返回-1
 */
int msg_queue_get(char *name, unsigned long flags)
{
    /* 检测参数 */
    if (name == NULL)
        return -1;
    char craete_new = 0; /* 是否需要创建一个新的消息队列 */
    msg_queue_t *msgq;
    int retval = -1;
    /* get msg queue table access */
    semaphore_down(&msg_queue_mutex);
    /* 有创建标志 */
    if (flags & IPC_CREAT) { /* 创建一个新的消息队列 */
        if (flags & IPC_EXCL) { /* 必须不存在才行 */
            craete_new = 1; /* 需要创建一个新的消息队列 */
        }
        msgq = msg_queue_find_by_name(name);

        if (msgq) {  /* 消息队列已经存在 */
            /* 必须创建一个新的，不能是已经存在的，故错误 */
            if (craete_new) {
                goto err;
            }
            retval = msgq->id; /* 已经存在，那么就返回已经存在的消息队列的id */
        } else { /* 不存在则创建一个新的 */
            
            msgq = msg_queue_alloc(name);
            if (msgq == NULL) {
                goto err;
            }
            retval = msgq->id; /* 返回消息队列id */
        }
    }
err:
    semaphore_up(&msg_queue_mutex);
    /* 没有创建标志，直接返回错误 */
    return retval;
}

/**
 * msg_queue_put - 释放一个消息队列
 * 
 * @msgid: 消息队列id
 * 
 * @return: 成功返回0，失败返回-1
 */
int msg_queue_put(int msgid)
{
    msg_queue_t *msgq;
    /* get msg queue table access */
    semaphore_down(&msg_queue_mutex);
    msgq = msg_queue_find_by_id(msgid);

    if (msgq) {  /* 消息队列存在 */
#if DEBUG_msgq == 1
        printk(KERN_INFO "msgq msgs %d.\n", msgq->msgs);
#endif
        msg_queue_free(msgq);
        semaphore_up(&msg_queue_mutex);        
        return 0;
    }
    
    semaphore_up(&msg_queue_mutex);
    /* 没有找到消息队列 */
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
 * msg_queue_send - 把一条消息添加到消息队列
 * @msgid: 消息队列id
 * @msgbuf: 消息数据
 * @size: 消息大小，不包括long int 的type。
 * @msgflg: 消息标志，IPC_NOWAIT表示队列满不等待，否则就要等待。
 * 
 * @return: 成功返回0，失败返回-1
 */
int msg_queue_send(int msgid, void *msgbuf, size_t size, int msgflg)
{
    msg_queue_t *msgq;
    /* get msg queue table access */
    semaphore_down(&msg_queue_mutex);
    msgq = msg_queue_find_by_id(msgid);
    if (msgq == NULL) {  /* not found message queue */
        semaphore_up(&msg_queue_mutex);
        printk(KERN_DEBUG "msg_queue_send: not found message queue!\n");
        return -1;
    }   
    semaphore_up(&msg_queue_mutex);
    
    //task_t *cur = current_task;
    /* get msg queue */
    semaphore_down(&msgq->mutex);
    if (size > msgq->msgsz) {
        size = msgq->msgsz; /* 截断消息 */
#if DEBUG_MSGQ == 1
        printk(KERN_NOTICE "msg_queue_send: message too long, truncated!\n");
#endif
    }

    /* 如果消息队列已经满了，就根据标志来判断进行什么样的操作 */
    if (msgq->msgs > MSGQ_MAX_MSGS) {
        if (msgflg & IPC_NOWAIT) {  /* 有不等待标志，直接返回 */
            semaphore_up(&msgq->mutex);
            printk(KERN_DEBUG "msg_queue_send: message queue full, no wait!\n");
            return -1;
        }
#if DEBUG_MSGQ == 1
        printk(KERN_NOTICE ">>> sender wait.\n\n");
#endif        
        /* 没有不等待标志，就阻塞 */
        wait_queue_add(&msgq->senders, current_task);
        semaphore_up(&msgq->mutex);
        task_block(TASK_BLOCKED); /* 阻塞自己 */
        semaphore_down(&msgq->mutex); /* 唤醒后再次获取 */
    }

    /* 分配一个消息 */
    msg_t *msg = kmalloc(sizeof(msg_t) + size);
    if (msg == NULL) {
        semaphore_up(&msgq->mutex);
        return -1;
    }
        
    long *msg_header = (long *) msgbuf;

    /* 填写消息 */
    msg_init(msg, *msg_header, msg_header + 1, size);

    /* 把消息添加到队列 */
    list_add(&msg->list, &msgq->msg_list);
    msgq->msgs++;
    /* 发送完后，检测是否有等待接收中的任务，并将它唤醒 */
    wait_queue_wakeup(&msgq->receivers);
    semaphore_up(&msgq->mutex);
#if DEBUG_MSGQ == 1    
    printk(KERN_DEBUG "msg_queue_send: send msg type=%d len:%d ok!\n", msg->type, msg->length);
#endif
    return 0;
}

/**
 * msg_queue_recv - 从一个消息队列中获取消息
 * @msgid: 消息队列id
 * @msgbuf: 消息数据
 * @msgsz: 消息大小，不包括long int 的type。
 * @msgtype: 消息类型，实现接收优先级
 *          =0：表示返回队列里的第一条消息
 *          >0：返回队列第一条类型等于msgtype的消息。
 *          <0：返回队列第一条类型小于等于msgtype绝对值的消息。
 *            
 * @msgflg: 消息标志：
 *          IPC_NOWAIT：队列没有可读消息不等待，返回错误
 *          IPC_NOERROR：消息大小超过size时被截断
 *          msgtype>0且msgflg=IPC_EXCEPT：接收类型不等于msgtype的第一条消息
 * 
 * @return: 成功返回实际接收的数据量，失败返回-1
 */
int msg_queue_recv(int msgid, void *msgbuf, size_t msgsz, long msgtype, int msgflg)
{
    msg_queue_t *msgq;
    /* get msg queue table access */
    semaphore_down(&msg_queue_mutex);
    msgq = msg_queue_find_by_id(msgid);
    if (msgq == NULL) {  /* not found message queue */
        semaphore_up(&msg_queue_mutex);    
        printk(KERN_DEBUG "msg_queue_recv: not found message queue!\n");
        
        return -1;
    }   
    semaphore_up(&msg_queue_mutex);
    
    /* get msg queue access */
    semaphore_down(&msgq->mutex);
    /* 如果消息队列为空，就根据标志来判断进行什么样的操作 */
    if (!msgq->msgs) {
        if (msgflg & IPC_NOWAIT) {  /* 有不等待标志，直接返回 */
            semaphore_up(&msgq->mutex);
            printk(KERN_DEBUG "msg_queue_recv: message queue empty, no wait!\n");
            
            return -1;
        }
#if DEBUG_MSGQ == 1
        printk(KERN_NOTICE "<<< receiver wait.\n");
#endif
        /* 没有不等待标志，就阻塞 */
        wait_queue_add(&msgq->receivers, current_task);
        semaphore_up(&msgq->mutex);
        task_block(TASK_BLOCKED); /* 阻塞自己 */
        semaphore_down(&msgq->mutex); /* 唤醒后再次获取 */
    }

    /* 根据msgtype获取一个消息 */
    msg_t *msg = NULL, *tmp;
    if (msgtype > 0) {  /* 第一个分支 */
        if (msgflg & IPC_EXCEPT) {  /* 获取第一个非msgtype类型消息 */
            list_for_each_owner (tmp, &msgq->msg_list, list) {
                if (tmp->type != msgtype) { /* 类型不相等，是我们需要的消息 */
                    msg = tmp;
                    break;
                }
            }
        } else { /* 获取第一个类型相等的消息 */
            list_for_each_owner (tmp, &msgq->msg_list, list) {
                if (tmp->type == msgtype) { /* 类型不相等，是我们需要的消息 */
                    msg = tmp;
                    break;
                }
            }
        }
    } else if (msgtype == 0) {  /* 获取第一个消息 */
        /* 获取链表第一个消息 */
        msg = list_first_owner_or_null(&msgq->msg_list, msg_t, list);
    } else {    /* 获取第一个小于等于msgtype绝对值的消息 */
        msgtype = ABS(msgtype);
        list_for_each_owner (tmp, &msgq->msg_list, list) {
            if (tmp->type <= msgtype) {
                msg = tmp;
                break;
            }
        }
    }
    if (msg == NULL) { /* 没找到相匹配的消息 */
        semaphore_up(&msgq->mutex);
        printk(KERN_DEBUG "msg_queue_recv: message not found!\n");
            
        return -1;
    }   
        
    /* 找到消息，从链表删除 */
    list_del(&msg->list);
    msgq->msgs--;   /* 消息数减少1 */

    unsigned short len = msg->length;
    /* 复制数据到缓冲区 */
    if (msgflg & IPC_NOERROR) { 
        if (len > msgsz) {  /* 超过参数传递的大小，截断 */
            len = msgsz;
        }
    }
    long *msg_header = (long *) msgbuf;
    *msg_header = msg->type;
    memcpy((void *)(msg_header + 1), msg->buf, len);
#if DEBUG_MSGQ == 1    
    printk(KERN_DEBUG "msg_queue_recv: recv msg type=%d len:%d ok!\n", msg->type, msg->length);
#endif
    /* 释放消息 */
    kfree(msg);
    /* 接收完后，检测是否有等待发送中的任务，并将它唤醒 */
    wait_queue_wakeup(&msgq->senders);
    semaphore_up(&msgq->mutex);

    return len;
}

/**
 * init_msg_queue - 初始化消息队列
 */
void init_msg_queue()
{
    msg_queue_table = (msg_queue_t *)kmalloc(sizeof(msg_queue_t) * MSGQ_MAX_NR);
    if (msg_queue_table == NULL) /* must be ok! */
        panic(KERN_EMERG "init_msg_queue: alloc mem for msg_queue_table failed! :(\n");
    //printk(KERN_DEBUG "init_msg_queue: alloc mem table at %x\n", msg_queue_table);   
    int i;
    for (i = 0; i < MSGQ_MAX_NR; i++) {
        msg_queue_table[i].id = 1 + i + i * 2;
        /* init msg list in each msg queue */
        INIT_LIST_HEAD(&msg_queue_table[i].msg_list);
        msg_queue_table[i].msgs = 0;
        semaphore_init(&msg_queue_table[i].mutex, 1); /* 初始化为1，表示有没有持有锁 */ 
        memset(msg_queue_table[i].name, 0, MSGQ_NAME_LEN);
    }
#if 0
    int msgid = msg_queue_get("test", IPC_CREAT);
    if (msgid < 0) {
        printk(KERN_DEBUG "get msgq failed!\n");
    }
    printk(KERN_DEBUG "get msgq %d.\n", msgid);
    msgid = msg_queue_get("test", IPC_CREAT | IPC_EXCL);
    if (msgid < 0) {
        printk(KERN_DEBUG "get msgq failed!\n");
    }
    printk(KERN_DEBUG "get msgq %d.\n", msgid);
    if (msg_queue_put(msgid) < 0) {
        printk(KERN_DEBUG "put msgq %d failed!\n", msgid);
    }
#endif
}
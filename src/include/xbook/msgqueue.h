#ifndef _XBOOK_MSG_QUEUE_H
#define _XBOOK_MSG_QUEUE_H

#include "list.h"
#include "waitqueue.h"
#include "semaphore.h"

/* 消息队列名字长度 */
#define MSGQ_NAME_LEN      24

/* 单个消息最大长度, 8kb */
#define MSG_MAX_LEN			1000 

/* 最多允许多少个消息队列 */
#define MSGQ_MAX_NR			128

/* 消息队列上最多允许多少个消息 */
#define MSGQ_MAX_MSGS		64

typedef struct msg {
	list_t list;				/* 消息链表，添加到消息队列中去 */
	long type;					/* 消息类型 */
	unsigned short length;		/* 消息长度 */
	char buf[1];				/* 消息数据缓冲区 */		
} msg_t;

/* 消息队列结构 */
typedef struct {
	unsigned short id;					/* 消息队列id */
	list_t msg_list;					/* 消息链表，所属的消息都在此链表上 */
	unsigned char msgs;					/* 消息数量 */
	unsigned short msgsz;				/* 消息最大大小，可调节 */
	wait_queue_t senders;				/* 发送者等待队列 */
	wait_queue_t receivers;				/* 接受者等待队列 */
	semaphore_t mutex;					/* 保护队列操作 */
	char name[MSGQ_NAME_LEN];			/* 名字 */
} msg_queue_t;

msg_queue_t *msg_queue_alloc(char *name);
int msg_queue_free(msg_queue_t *msgq);
int msg_queue_get(char *name, unsigned long flags);
int msg_queue_put(int msgid);
int msg_queue_send(int msgid, void *msgbuf, size_t size, int msgflg);
int msg_queue_recv(int msgid, void *msgbuf, size_t msgsz, long msgtype, int msgflg);

void init_msg_queue();


#endif   /* _XBOOK_MSG_QUEUE_H */

#ifndef _XBOOK_PIPE_H
#define _XBOOK_PIPE_H

#include "mutexlock.h"
#include "fifobuf.h"
#include "task.h"

/* 管道名字长度 */
#define PIPE_NAME_LEN      24

/* 管道大小 */
#define PIPE_SIZE      		4096

/* 支持的管道数量 */
#define PIPE_NR			128

/* 管道标志 */
#define PIPE_IN_READ	0x01
#define PIPE_IN_WRITE	0x02




/* 管道结构 */
typedef struct {
	unsigned short id;					/* 管道id */
	fifo_buf_t *fifo;					/* 先入先出缓冲区 */
	task_t *reader, *writer;			/* 读者与写者 */
	/* 标志位：低16位是公用标志，16-23位是读端位，24-31是写端位 */
    unsigned int flags;				
    atomic_t readref, writeref;              /* 读写引用计数 */
	mutexlock_t mutex;					/* 保证同一个时刻要么是读，要么是写 */
	char name[PIPE_NAME_LEN];			/* 名字 */
} pipe_t;

pipe_t *pipe_alloc(char *name);
int pipe_free(pipe_t *pipe);
int pipe_get(char *name, unsigned long flags);
int pipe_put(int pipeid);
int pipe_write(int pipeid, void *buffer, size_t size, int pipeflg);
int pipe_read(int pipeid, void *buffer, size_t size, int pipeflg);
int pipe_ctl(int pipeid, unsigned int cmd, unsigned long arg);
int pipe_grow(int pipeid);
void init_pipe();

#endif   /* _XBOOK_PIPE_H */

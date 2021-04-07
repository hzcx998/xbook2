#ifndef _XBOOK_FIFO_H
#define _XBOOK_FIFO_H

#include "mutexlock.h"
#include "fifobuf.h"
#include "task.h"

/* 管道名字长度 */
#define FIFO_NAME_LEN      24

/* 管道大小 */
#define FIFO_SIZE      		4096

/* 支持的管道数量 */
#define FIFO_NR			128

/* 管道标志 */
#define FIFO_IN_READ	0x01
#define FIFO_IN_WRITE	0x02

#define NAMEED_PIPE_PATH	"/pipe/"

/* 管道结构 */
typedef struct {
	unsigned short id;					/* 管道id */
	fifo_buf_t *fifo;					/* 先入先出缓冲区 */
	task_t *reader, *writer;			/* 读者与写者 */
	/* 标志位：低16位是公用标志，16-23位是读端位，24-31是写端位 */
    unsigned int flags;				
    atomic_t readref, writeref;              /* 读写引用计数 */
	mutexlock_t mutex;					/* 保证同一个时刻要么是读，要么是写 */
	char name[FIFO_NAME_LEN];			/* 名字 */
} fifo_t;

fifo_t *fifo_alloc(char *name);
int fifo_free(fifo_t *FIFO);
int fifo_get(char *name, unsigned long flags);
int fifo_put(int fifoid);
int fifo_write(int fifoid, void *buffer, size_t size);
int fifo_read(int fifoid, void *buffer, size_t size);
int fifo_ctl(int fifoid, unsigned int cmd, unsigned long arg);
int fifo_incref(int fifoid);
int fifo_decref(int fifoid);
void fifo_init();
int fifo_make(char *name, mode_t mode);
int sys_mkfifo(const char *pathname, mode_t mode);

#endif   /* _XBOOK_FIFO_H */

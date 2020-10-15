#ifndef _XBOOK_PIPE_H
#define _XBOOK_PIPE_H

#include "mutexlock.h"
#include "fifobuf.h"
#include "waitqueue.h"
#include <list.h>
#include <stdint.h>
#include <types.h>

#define PIPE_SIZE   4096

/* pipe flags */
enum {
    PIPE_NOWAIT = 0x01,
};


/* 管道结构 */
typedef struct {
    list_t list;                /* 链表 */
    kobjid_t id;                /* id号 */
	fifo_buf_t *fifo;           /* 数据缓冲区 */
	uint16_t flags;		        /* 管道标志 */
    uint8_t rdflags;		    /* 读端标志 */
    uint8_t wrflags;		    /* 写端标志 */
    atomic_t read_count;        /* 读引用计数 */
    atomic_t write_count;       /* 写引用计数 */
	mutexlock_t mutex;          /* 读写互斥 */
    wait_queue_t wait_queue;    /* 等待队列 */
} pipe_t;

pipe_t *create_pipe();
int destroy_pipe(pipe_t *pipe);
int pipe_read(kobjid_t pipeid, void *buffer, size_t bytes);
int pipe_write(kobjid_t pipeid, void *buffer, size_t bytes);
int pipe_close(kobjid_t pipeid, int rw);
int pipe_ioctl(kobjid_t pipeid, unsigned int cmd, unsigned long arg, int rw);
int pipe_grow(kobjid_t pipeid, int rw);
int pipe_clear(pipe_t *pipe);

#endif  /* _XBOOK_PIPE_H */

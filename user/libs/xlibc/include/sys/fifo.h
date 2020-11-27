#ifndef _SYS_FIFO_H
#define _SYS_FIFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "spinlock.h"

struct fifo_t {
	unsigned char * buffer;
	unsigned int size;
	unsigned int in;
	unsigned int out;
	spinlock_t lock;
};

void __fifo_reset(struct fifo_t * f);
unsigned int __fifo_len(struct fifo_t * f);
unsigned int __fifo_put(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int __fifo_get(struct fifo_t * f, unsigned char * buf, unsigned int len);

struct fifo_t * fifo_alloc(unsigned int size);
void fifo_free(struct fifo_t * f);
void fifo_reset(struct fifo_t * f);
unsigned int fifo_len(struct fifo_t * f);
unsigned int fifo_put(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int fifo_get(struct fifo_t * f, unsigned char * buf, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_FIFO_H */

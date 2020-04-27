#ifndef _XBOOK_RAW_BLOCK_H
#define _XBOOK_RAW_BLOCK_H

#include "driver.h"
#include "list.h"

#define RAW_BLOCK_NAME_LEN 24

#define RB_SEEK_SET 0
#define RB_SEEK_CUR 1
#define RB_SEEK_END 2

#define RB_BLOCK_NR 256

#define RB_DEVICE   "vfloppy"

//#define RB_DEVICE   "ide0"

typedef struct raw_block {
    list_t list;                /* 所有块构成一个链表 */
    dev_t handle;                /* 设备号 */
    
    unsigned long offset;       /* 设备中的偏移 */
    unsigned long count;        /* 占用块的数量 */
    
    unsigned char *vaddr;        /* 所在的虚拟地址 */
    unsigned long memsz;        /* 占用内存大小 */
    
    unsigned long pos;          /* 数据访问位置 */
    char name[RAW_BLOCK_NAME_LEN]; /* 块名字标识 */
} raw_block_t;

raw_block_t *raw_block_alloc(handle_t handle, char *name);
void raw_block_free(raw_block_t *block);
raw_block_t *raw_block_get_by_name(char *name);

int raw_block_init(raw_block_t *block, unsigned long off, unsigned long count,
    unsigned long memsz);

int raw_block_upload(raw_block_t *block);
int raw_block_download(raw_block_t *block);

void raw_block_seek(raw_block_t *block, unsigned long pos, unsigned char seek);
long raw_block_read(raw_block_t *block, void *buffer, unsigned long size);
long raw_block_write(raw_block_t *block, void *buffer, unsigned long size);

int raw_block_read_off(raw_block_t *rb, void *buffer, unsigned long offset, unsigned long size);
int raw_block_write_off(raw_block_t *rb, void *buffer, unsigned long offset, unsigned long size);

int raw_block_tmp_add(raw_block_t *block, unsigned char *buf, unsigned long size);
void raw_block_tmp_del(raw_block_t *block);

void init_raw_block();


#endif  /*_XBOOK_RAW_BLOCK_H*/


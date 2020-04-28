#include <xbook/rawblock.h>
#include <xbook/string.h>
#include <xbook/math.h>
#include <xbook/debug.h>
#include <xbook/driver.h>

#define DEBUG_LOCAL 0

LIST_HEAD(raw_block_list);

raw_block_t *raw_block_alloc(handle_t handle, char *name)
{
    raw_block_t *block = kmalloc(sizeof(raw_block_t));
    if (block == NULL)
        return NULL;

    block->handle = handle;
    memset(block->name, 0, RAW_BLOCK_NAME_LEN);
    strcpy(block->name, name);

    block->offset = 0;
    block->count = 0;
    block->vaddr = NULL;
    block->memsz = 0;
    block->pos = 0;

    return block;
}

void raw_block_free(raw_block_t *block)
{
    list_del_init(&block->list);
    kfree(block);
}

int raw_block_init(raw_block_t *block, unsigned long off, unsigned long count,
    unsigned long memsz)
{
    block->offset = off;
    block->count = count;
    block->memsz = memsz;
    block->pos = 0;

    block->vaddr = kmalloc(block->memsz);
    if (block->vaddr == NULL)
        return -1;

    list_add_tail(&block->list, &raw_block_list);
    return 0;
}

/**
 * raw_block_tmp_add - 添加一个临时原始块
 * 
 * @buf: 数据缓冲区
 * @size: 数据大小
 * 
 * 一般用于proc_exec_file中
 * 
 */
int raw_block_tmp_add(raw_block_t *block, unsigned char *buf, unsigned long size)
{
    block->offset = 0;
    block->count = 0;
    block->memsz = size;
    block->pos = 0;
    block->vaddr = NULL;
    /* 根据缓冲区大小创建一个新的缓冲区，并把数据复制进去 */
    if (block->memsz <= MAX_MEM_CACHE_SIZE) { /* 分配一个小块内存 */
        block->vaddr = kmalloc(block->memsz);
    } else { /* 添加一个新的高速缓存 */
        printk(KERN_NOTICE "raw_block_tmp_add: need a large buffer!\n");
        return -1;
    }

    if (block->vaddr == NULL)
        return -1;

    /* 复制数据 */
    memcpy(block->vaddr, buf, size);

    return 0;
}

void raw_block_tmp_del(raw_block_t *block)
{
    if (block->memsz <= MAX_MEM_CACHE_SIZE) {
        kfree(block->vaddr);
    } else {
        printk(KERN_NOTICE "raw_block_tmp_del: free a large buffer!\n");
    }
}


raw_block_t *raw_block_get_by_name(char *name)
{
    raw_block_t *rb;
    list_for_each_owner (rb, &raw_block_list, list) {
        if (!strcmp(rb->name, name))
            return rb;
    }
    return NULL;
}

/* 从磁盘上传到内存 */
int raw_block_upload(raw_block_t *block)
{
    int count = block->count;
    long off = block->offset;
    
    /* 小于1个块 */
    if (count < RB_BLOCK_NR) {
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "raw_block_upload: count=%d\n", count);
#endif
        if (device_read(block->handle, block->vaddr, count * SECTOR_SIZE, off) < 0) 
            return -1;
    } else {
        /* 处理小于DATA_BLOCK个块 */
        int chunk = count & 0xff;   /* 取256以下的数据数量 */
        unsigned char *p = block->vaddr;
        while (count > 0) {
            if (device_read(block->handle, p, chunk * SECTOR_SIZE, off) < 0)
                return -1;
            
            off += chunk;
            count -= chunk;
            p += chunk * SECTOR_SIZE;
            /* 每次处理BLOCK个 */
            chunk = RB_BLOCK_NR;
        }
    }
    return 0;
}

/* 从内存下载到磁盘 */
int raw_block_download(raw_block_t *block)
{
    int count = block->count;
    long off = block->offset;
    
    /* 小于1个块 */
    if (count < RB_BLOCK_NR) {
        if (device_write(block->handle, block->vaddr, count * SECTOR_SIZE, off) < 0) 
            return -1;
    } else {
        /* 处理小于DATA_BLOCK个块 */
        int chunk = count & 0xff;   /* 取256以下的数据数量 */
        unsigned char *p = block->vaddr;
        while (count > 0) {
            if (device_write(block->handle, p, chunk  * SECTOR_SIZE, off) < 0)
                return -1;
            
            off += chunk;
            count -= chunk;
            p += chunk * SECTOR_SIZE;
            /* 每次处理BLOCK个 */
            chunk = RB_BLOCK_NR;
        }
    }
    return 0;
}

void raw_block_seek(raw_block_t *block, unsigned long pos, unsigned char seek)
{
    switch (seek)
    {
    case RB_SEEK_SET:
        block->pos = pos;
        break;
    case RB_SEEK_CUR:
        block->pos += pos;    
        break;
    case RB_SEEK_END:
        block->pos = block->count + pos;    
        break;
    default:
        break;
    }

    if (block->pos < 0)
        block->pos = 0;

    if (block->pos >= block->memsz)
        block->pos = block->memsz - 1;    
}

/* 成功返回读取的字节数，失败返回0 */
long raw_block_read(raw_block_t *block, void *buffer, unsigned long size)
{
    if (block->pos >= block->memsz)
        return 0;
    unsigned char *buf = (unsigned char *)buffer;
    unsigned char *p = block->vaddr + block->pos;
    unsigned long len = MIN(size, block->memsz - block->pos);
    memcpy(buf, p, len);
    block->pos += size;
    return len; /* return read byte */
}

/* 成功返回写入的字节数，失败返回0 */
long raw_block_write(raw_block_t *block, void *buffer, unsigned long size)
{
    if (block->pos >= block->memsz)
        return 0;
    unsigned char *buf = (unsigned char *)buffer;
    unsigned char *p = block->vaddr + block->pos;
    unsigned long len = MIN(size, block->memsz - block->pos);
    memcpy(p, buf, len);
    block->pos += size;
    return len; /* return read byte */
}

/**
 * raw_block_read_off - 从某个偏移位置读取数据
 * @rb: 原始块
 * @buffer: 读取到的buffer
 * @offset: 偏移
 * @size: 要读取的数据数量
 */
int raw_block_read_off(raw_block_t *rb, void *buffer, unsigned long offset, unsigned long size)
{
    raw_block_seek(rb, offset, RB_SEEK_SET);
    if (!raw_block_read(rb, buffer, size)) {
        printk(KERN_ERR "raw_block_read_off: read %d failed!\n", size);
        return -1;
    }
    return 0;
}

/**
 * raw_block_write_off - 往某个偏移位置写入数据
 * @rb: 原始块
 * @buffer: 读取到的buffer
 * @offset: 偏移
 * @size: 要读取的数据数量
 */
int raw_block_write_off(raw_block_t *rb, void *buffer, unsigned long offset, unsigned long size)
{
    raw_block_seek(rb, offset, RB_SEEK_SET);
    if (!raw_block_write(rb, buffer, size)) {
        printk(KERN_ERR "raw_block_write_off: write %d failed!\n", size);
        return -1;
    }
    return 0;
}
typedef struct {
    char *name;
    unsigned long off;
    unsigned long cnt;
    unsigned long size;
} raw_block_info_t;

#define MAX_RBI_NR  2
/* raw block info table
需要添加新原始块时，就添加到这个表中即可。
最大文件大小依据MAX_MEM_CACHE_SIZE而定
 */
raw_block_info_t rbi_table[MAX_RBI_NR] = {
    {"init", 0, 200, 1024*100},
    {"bin", 200, 200, 1024*100},
};

void init_raw_block()
{
    handle_t rbdev = device_open(RB_DEVICE, 0);
    if (rbdev < 0)
        panic("init_raw_block: open raw block device failed!\n");
        
    raw_block_info_t *rbi;
    /* 构建原始块信息 */
    int n;
    for (n = 0; n < MAX_RBI_NR; n++) {
        rbi = &rbi_table[n];
        printk(KERN_INFO "raw block name:%s\n", rbi->name);
        raw_block_t *rb = raw_block_alloc(rbdev, rbi->name);
        if (rb == NULL)
            panic(KERN_EMERG "raw block alloc for %s failed!\n", rbi->name);
        if (raw_block_init(rb, rbi->off, rbi->cnt, rbi->size))
            panic(KERN_EMERG "raw block init for %s failed!\n", rbi->name);
        if (raw_block_upload(rb))
            panic(KERN_EMERG "raw block upload for %s failed!\n", rbi->name);
        #if 0
        unsigned int *buf = kmalloc(SECTOR_SIZE);
        if (buf == NULL)
            panic(KERN_EMERG "kmalloc for buf faled!\n");
        int read = raw_block_read(rb, buf, SECTOR_SIZE);
        printk("read bytes:%d\n", read);
        int i;
        for (i = 0; i < SECTOR_SIZE / 4; i++) {
            printk("%x ", buf[i]);
        }
        #endif
    }

}
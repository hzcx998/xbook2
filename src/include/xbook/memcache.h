#ifndef _XBOOK_MEMCACHE_H
#define _XBOOK_MEMCACHE_H

#include <types.h>
#include <stddef.h>
#include <xbook/config.h>
#include <xbook/bitmap.h>
#include <xbook/list.h>
#include <xbook/mutexlock.h>
#include <const.h>

/*
当内存对象大小小于1024时，储存在一个页中。
+-----------+
| group     | 
| bitmap    |
| objects   |
+-----------+
当内存对象大于1024时，对象和纪律信息分开存放。记录信息存放在一个页中，
对象存放在其他页中。
+-----------+
| group     | 
| bitmap    |
+-----------+
| objects   |
+-----------+
*/

/* 最大的mem的对象的大小 */
#ifdef CONFIG_LARGE_ALLOCS 
	#define MAX_MEM_CACHE_SIZE (2*1024*1024)
#else
	#define MAX_MEM_CACHE_SIZE (128*1024)
#endif

#define MAX_MEM_OBJECT_SIZE     (24 * MB)

typedef struct mem_group {
    list_t list;           // 指向cache中的某个链表（full, partial, free）
    bitmap_t map;          // 管理对象分配状态的位图
    unsigned char *objects;     // 指向对象群的指针
    unsigned long using_count;    // 正在使用中的对象数量
    unsigned long free_count;     // 空闲的对象数量
    unsigned long flags;         // group的标志
} mem_group_t;

#define SIZEOF_MEM_GROUP sizeof(mem_group_t)

#define MEM_CACHE_NAME_LEN 24

typedef struct mem_cache {
    list_t full_groups;      // group对象都被使用了，就放在这个链表
    list_t partial_groups;   // group对象一部分被使用了，就放在这个链表
    list_t free_groups;      // group对象都未被使用了，就放在这个链表

    unsigned long object_size;    // group中每个对象的大小
    flags_t flags;              // cache的标志位
    unsigned long object_count;  // 每个group中有多少个对象
    mutexlock_t mutex;
    char name[MEM_CACHE_NAME_LEN];     // cache的名字
} mem_cache_t;

typedef struct cache_size {
    unsigned long cache_size;         // 描述cache的大小
    mem_cache_t *mem_cache;      // 指向对应cache的指针
} cache_size_t;

typedef struct {
    list_t list;    /* 链表 */
    size_t size;    /* 内存大小 */
    void *addr;     /* 虚拟地址 */
} large_mem_object_t;

int mem_caches_init();

void *mem_alloc(size_t size);
void *mem_realloc(void *ptr, size_t size);
void *mem_zalloc(size_t size);
void mem_free(void *object);
int ksharink();

void *mem_alloc_align(size_t size, int align);
#define mem_free_align(ptr) mem_free(ptr)

int mem_cache_init(mem_cache_t *cache, char *name, size_t size, flags_t flags);
void *mem_cache_alloc_object(mem_cache_t *cache);
void mem_cache_free_object(mem_cache_t *cache, void *object);

#endif   /* _XBOOK_MEMCACHE_H */

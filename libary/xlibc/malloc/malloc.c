#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>

//#define DEBUG_MEMOBJ

#define MEMOBJ_NR   1024

#define MEMOBJ_UNUSED   0   /* 未使用 */
#define MEMOBJ_USING    1   /* 使用中 */
#define MEMOBJ_FREE     2   /* 空闲，可重复利用 */

typedef struct _memobject {
    size_t size;
    void *addr;
    int flags;
} memobject_t;

static memobject_t mobjs[MEMOBJ_NR]; 

static int is_memobj_init = 0;

#define ALIGN(p,alignbytes) ((void*)(((unsigned long)p+alignbytes-1)&~(alignbytes-1)))

static size_t szalign(size_t size){
    if(size % 8 ==0) return size;
    return ((size >> 3) + 1) <<3;
}

static void memobj_init() {
    int i;
    for (i = 0; i < MEMOBJ_NR; i++) {
        mobjs[i].addr = NULL;
        mobjs[i].size = 0;
        mobjs[i].flags = 0;
    }
    is_memobj_init = 1;
}

static int memobj_add(void *addr, size_t size) {
    int i;
    for (i = 0; i < MEMOBJ_NR; i++) {
        if (mobjs[i].flags == MEMOBJ_UNUSED) {
            mobjs[i].addr = addr;
            mobjs[i].size = size;
            mobjs[i].flags = MEMOBJ_USING;
            #ifdef DEBUG_MEMOBJ
            printf("memobj add %d\n", i);
            #endif
            return 0;
        }
    }
    return -1;    
}

static memobject_t *memobj_getobj(void *addr) {
    int i;
    for (i = 0; i < MEMOBJ_NR; i++) {
        if (mobjs[i].addr == addr && mobjs[i].size > 0 && mobjs[i].flags != MEMOBJ_UNUSED) {
            return &mobjs[i];
        }
    }
    return NULL;  
}

static int memobj_del(void *addr) {
    int i;
    for (i = 0; i < MEMOBJ_NR; i++) {
        if (mobjs[i].addr == addr && mobjs[i].size > 0 && mobjs[i].flags != MEMOBJ_UNUSED) {
            /*mobjs[i].size = 0;
            mobjs[i].addr = NULL;*/
            mobjs[i].flags = MEMOBJ_FREE;
            #ifdef DEBUG_MEMOBJ
            printf("memobj del %d\n", i);
            #endif
            return 0;
        }
    }
    return -1;    
}

static void *memobj_match(size_t size) {
    int i;
    for (i = 0; i < MEMOBJ_NR; i++) {
        if (mobjs[i].flags == MEMOBJ_FREE) {
            if (size <= mobjs[i].size) {
                mobjs[i].flags = MEMOBJ_USING;
                #ifdef DEBUG_MEMOBJ
                printf("memobj match %d\n", i);
                #endif
                return mobjs[i].addr;
            }
        }
    }
    return NULL;    
}


static void *malloc_intenal(size_t size, int _align){
    size = szalign(size);

    /* 匹配空闲对象 */
    void *p;
    p = memobj_match(size);
    if (p)
        return p;

    p = sbrk(0);
    if (sbrk(size) == (void *) -1)
        return NULL;
    
    /* 地址对齐 */
    void *addr = ALIGN(p,_align);
    unsigned long off = addr - p;
    if (sbrk(off) == (void *) -1)
        return NULL;
    #ifdef DEBUG_MEMOBJ
    printf("malloc: old %p, new %p, off %x size %x\n", p, addr, off, size);
    #endif
    /* 添加到内存记录 */
    if (!is_memobj_init)
        memobj_init();

    if (memobj_add(addr, size) < 0) {
        sbrk(-(off + size));
        return NULL;
    }
    return (void *) addr;
}

void *malloc(size_t size){
    return malloc_intenal(size, 8);
}

void *memalign (size_t boundary, size_t size) {
    return malloc_intenal(size, boundary);
}

void *calloc(int n,size_t size){
    void *p = malloc_intenal(size * n, 8);
    if (p)
        memset(p, 0, size * n);
    return p;
}

/*free 函数实现*/
void free(void *p){
    if (!p)
        return;

    /* 获取原来的内存的长度 */
    memobject_t *obj = memobj_getobj(p);
    if (!obj)
        return;
    #ifdef DEBUG_MEMOBJ
    printf("free: addr %p, size %x\n", obj->addr, obj->size);
    #endif
    if (memobj_del(p) < 0) {
        return;
    }
}

void *realloc(void *p, size_t size){
    if (p == NULL && !size)
        return NULL;
    if (p == NULL)
        return malloc(size);
    if (!size) {
        free(p);
        return NULL;
    }
        
    /* 获取原来的内存的长度 */
    memobject_t *obj = memobj_getobj(p);
    if (!obj)
        return NULL;

    if (obj->size >= size) { /* 缩小 */
        #ifdef DEBUG_MEMOBJ
        printf("realloc: shrink addr %x size %x -> %x\n", obj->addr, obj->size, size);
        #endif
        obj->size = size;
        return obj->addr;
    }
    /* 扩大内存 */
    void *q = malloc(size);
    if (q == NULL) {
        //printf("realloc2: alloc new failed!\n");
        return NULL;
    }
        
    memcpy(q, obj->addr, obj->size);
    #ifdef DEBUG_MEMOBJ
    printf("realloc: addr %x -> %x size %x -> %x\n", obj->addr, q, obj->size, size);
    #endif
    free(obj->addr);
    return q;
}

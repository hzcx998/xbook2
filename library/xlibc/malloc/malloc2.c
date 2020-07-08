/**
 * 这种内存分配方式，只会扩展堆，而不会对堆进行缩小。
 * 1.使用显示双向链表，按地址顺序维护内存块。
 * 2.使用循环首次适应算法进行空闲块的选择。
 */

#include <stddef.h>
#include <types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define __HAS_NOT_USED 	0
#define __HAS_USED		1

#define ALIGN8(size) ((size + 8) & (~7))
#define ALIGN4(size) ((size + 4) & (~3))

struct __memory_block
{
	int used;	// 是否被使用了
	int size;			// 所在块的大小

	/* 指向前驱和后驱的指针 */
	struct __memory_block *prev;
	struct __memory_block *next;

};

#define MEM_BLOCK_SIZE sizeof(struct __memory_block)

/* 内存块头指针 */
struct __memory_block *head;
/* 内存块尾指针 */
struct __memory_block *tail;
/* 遍历内存块的标记 */
struct __memory_block *flag;

/* 最开始运行时，没有初始化，当第一次调用malloc时才初始化 */
static char __had_initialized = 0;

/**
 * __init_memory_alloctor - 初始化内存管理器
 * 
 * 成功返回0， 失败返回-1
 */
static int __init_memory_alloctor()
{
	head = sbrk(sizeof(struct __memory_block));
	tail = sbrk(sizeof(struct __memory_block));
	if (head == (void *)-1 || tail == (void *)-1) {
		return -1;
	}
	head->used = tail->used = __HAS_USED;

	head->prev = tail;
	tail->prev = head;
	head->next = tail;
	tail->next = head;
	/* 遍历指针指向头 */
	flag = head;
	/* 已经初始化了，后面就不会在调用初始化函数 */
	__had_initialized = 1;
	return 0;
}

/**
 * __copy_block_data - 复制内存块数据
 * @src: 源内存块
 * @dst: 目的内存块
 * 
 * 复制的数据量都小于等于源和目的内存块的大小
 */
static void __copy_block_data(struct __memory_block *src, struct __memory_block *dst)
{
    int *sdata, *ddata;
    size_t i;
	// 指向数据区域
    sdata = (int *)(src + 1);
    ddata = (int *)(dst + 1);

	/* 当双方大小都满足时才进行舒服复制 */
    for(i = 0; i*4 < src->size && i*4 < dst->size; ++i)
        ddata[i] = sdata[i];
}

/**
 * __get_block - 通过地址获取内存块结构
 * @ptr: 内存指针
 */
static struct __memory_block *__get_block(void *ptr)
{
	return (struct __memory_block *)(ptr - MEM_BLOCK_SIZE);
};



/**
 * __split_block - 分裂一个块
 * @current: 要分裂的块
 * @size: 块的大小
 * 
 * 分裂一个块，截取size作为当前块大小，把后面的空间做出一个新的块
 */
static void __split_block(struct __memory_block *current, size_t size)
{
	struct __memory_block *next, *new;

	next = current->next;
	/* alloc block 在current后面 */
	new = (void*)current + size + MEM_BLOCK_SIZE;
	new->used = __HAS_NOT_USED;
	new->size = current->size - size - MEM_BLOCK_SIZE;
	new->prev = current;
	new->next = next;
	current->next = new;
	current->size = size;
	next->prev = new;
} 

/**
 * malloc - 分配一块内存
 * @size: 要分配的内存的大小
 * 
 * 在堆中分配一块内存，如果没有空闲堆了，就调用sbrk扩展内存堆
 */
void *malloc(size_t size)
{
	struct __memory_block *current_block;
	struct __memory_block *prev_block;
	void *memory_location = NULL;
	unsigned int block_size = size + MEM_BLOCK_SIZE;
	struct __memory_block *alloc_block = NULL;

	size = ALIGN4(size);
    if(size <= 0)
		return NULL;

	/* 尝试初始化 */
	if(!__had_initialized){
		if (__init_memory_alloctor() == -1)
			return NULL;
	}
	/* 当前块指向flag后，就会循环每一个block */
	current_block = flag;

	while(current_block){
		if(!current_block->used){
			if(current_block->size >= size){       /*将current_block标记为已分配*/
				current_block->used = __HAS_USED;
				memory_location = (void*) current_block + MEM_BLOCK_SIZE;
				//printf("malloc: found a block %x\n", memory_location);
	
				/* 判断是否需要分割当前块，至少要有16字节区域，才尝试把block细分*/
				if(current_block->size - size > MEM_BLOCK_SIZE + 4){          
					__split_block(current_block, size);
				}
				break;
			}
		}
		current_block = current_block->next;
		if(current_block == flag)
			break;
	}
	flag = current_block;

	/*没有找到合适的块，申请新的空间*/
	/*这一步很关键，必须注意将tail块放置到最后才能保证链表中块的地址顺序*/
	if(!memory_location){
		prev_block = tail->prev;

		/* 先把tail占用的内存释放 */
		sbrk(-1*MEM_BLOCK_SIZE);
		/* 再分配新的内存给请求大小 */
		alloc_block = sbrk(block_size);

		if(alloc_block == (void *)-1){
			printf("sbrk failed!\n");
			return NULL;
		}
		/* 再分配新的内存给tail */
		tail = sbrk(MEM_BLOCK_SIZE);

		alloc_block->size = size;
		alloc_block->used = __HAS_USED;
		alloc_block->next = tail;
		alloc_block->prev = prev_block;
		prev_block->next = alloc_block;

		tail->prev = alloc_block;
		tail->next = head;
		tail->used = __HAS_USED;
		tail->size = 0;

		head->prev = tail;
		
		//printf("malloc: sbrk addr %x\n", alloc_block);
	
		memory_location = (void *) alloc_block + sizeof(struct __memory_block);
	}
	return memory_location;
}

static void __merge_prev(struct __memory_block *current)
{
	struct __memory_block *prev = current->prev;
	/* 要加上当前地址的size和结构体所占的size */
	prev->size = prev->size + current->size + MEM_BLOCK_SIZE;
	prev->next = current->next;
	current->next->prev = prev;
}

static void __merge_next(struct __memory_block *current)
{
	struct __memory_block *next = current->next;
	/* 要加上后一个地址的size和结构体所占的size */
	current->size = current->size + next->size + MEM_BLOCK_SIZE;
	current->next = next->next;
	next->next->prev = current;
}

/**
 * free - 释放一个内存地址
 * @ptr: 需要释放的内存地址
 * 
 * 释放内存并尝试和相邻的内存块合并，减小系统开销
 */
void free(void *ptr)
{
	if(!ptr)
		return;
	struct __memory_block *current = __get_block(ptr);
	/* 判断是否合并，四种情况，1.不合并。2.前合并。3.后合并。4.前后同时合并 */
	/* 边界上的问题已经通过初始化解决了，合并完后prev是一直在前面的，所以涉及到
	prev的都可以不用设置used状态，不然就需要设置未使用标志*/

	struct __memory_block *prev = current->prev;
	struct __memory_block *next = current->next;

	/* 前后都在使用，不合并 */
	if(prev->used && next->used){
		current->used = __HAS_NOT_USED;

	} else if(!prev->used && next->used){
		/* 和前面合并 */
		__merge_prev(current);
		/* 标记块 */
		flag = prev;
	} else if(prev->used && !next->used){
		/* 和后面合并 */

		/* 要设置成未使用 */
		current->used = __HAS_NOT_USED;
		__merge_next(current);

		/* 标记块 */
		flag = current;	
	} else if(!prev->used && !next->used){
		/* 前后都合并 */

		/* 先和后面的合并 */
		__merge_next(current);
		/* 在和前面的合并 */
		__merge_prev(current);
		/* 标记块 */
		flag = prev;
	}
}


/**
 * calloc - 分配内存并置0
 * numitems: 有多少项
 * size: 每项的大小
 * 
 * 分配内存后，置0
 */
void *calloc(int n,size_t size)
{
    size_t *new;
    size_t s, i;
    new = malloc(n * size);
    if(new)
    {
		//因为申请的内存总是4的倍数，所以这里我们以4字节为单位初始化
        s = ALIGN4(n * size) >> 2;
        for(i = 0; i < s; ++i)
            new[i] = 0;
    }
    return new;
}

/**
 * realloc - 重新分配内存，扩大或者缩小当前区域
 * @ptr: 指针
 * @size: 要重新获取的大小
 * 
 * 1.如果ptr为空，那么就相当于malloc，分配size大小内存
 * 2.如果ptr不为空，size为0，那么就相当于free
 * 3.如果size比原来的小就缩小内存，并尝试分裂
 * 4.如果size比原来的大，就尝试寻找附近block，看能否合并，
 * 	合并后能否满足size需求
 * 5.如果size比原来大，且不能合并，那么就分配一块新的内存，并且复制数据
 */
void *realloc(void *ptr, size_t size)
{
    
    size_t _size;
    struct __memory_block *block, *newBlock;
    void *newPtr;

	/* 如果ptr为NULL，size为0，就啥也不做，返回ptr */
	if (!ptr && !size)
		return ptr;

	/* 如果ptr为NULL */
    if (!ptr) {
		//printf("realloc: alloc memory\n");
		return malloc(size);
	}
        
	
	/* 如果size未0，并且ptr存在，那么就释放内存 */
	if (!size && ptr != NULL) {
		//printf("realloc: free memory %x\n");
		free(ptr);
		return ptr;
	}
		
	_size = ALIGN4(size);
	//得到对应的block
	block = __get_block(ptr);	
	
	//如果size变小了，考虑收缩内存
	if(block->size >= _size) {
		/* 如果block的大小比内存块结构体+4都大，说明可以分裂一个新的块出来 */
		if(block->size - _size >= (sizeof(struct __memory_block) + 4)) {
			//printf("realloc: shrik-> block size %d size %d\n", block->size, size);
		
			__split_block(block, _size);
			/* 标记块 */
			flag = block;
		}
		/* 收缩完后，还是返回原来的空间 */
	} else {	//如果当前block的数据区不能满足size

		/*如果前继是unused的，后继是used的，并且如果合并后大小满足size，考虑合并 */
		if(!block->prev->used && block->next->used &&
			(block->size + MEM_BLOCK_SIZE + block->prev->size) >= _size) {

			__merge_prev(block);
			
			/* 合并后block就消失了，但是还是可以使用它的结构，
			如果prev满足size，再看能不能split*/
			if(block->prev->size - _size >= (MEM_BLOCK_SIZE + 4))
				__split_block(block->prev, _size);

			/* 标记块 */
			flag = block->prev;

			/* 因为把block合并到prev中去了，所以要修改成prev的地址 */
			ptr = (struct __memory_block *)block->prev + 1;

			//printf("realloc: merge prev size %d size %d\n", block->prev->size, _size);
		
			/* 与此同时还需要复制数据 */
			__copy_block_data(block, block->prev);

			
		} else if(block->prev->used && !block->next->used &&
			(block->size + MEM_BLOCK_SIZE + block->next->size) >= _size) {
			/*如果前继是used的，后继是unused的，并且如果合并后大小满足size，考虑合并 */
		
			__merge_next(block);
			
			/* 合并后如果满足size，再看能不能split */
			if(block->size - _size >= (MEM_BLOCK_SIZE + 4))
				__split_block(block, _size);
		
			//printf("realloc: merge next size %d size %d\n", block->size, _size);
			/* 标记块 */
			flag = block;
			/* 因为block还在，所以不需要复制数据 */	
		} else if(!block->prev->used && !block->next->used &&
			(block->size + block->prev->size + block->next->size + MEM_BLOCK_SIZE*2) >= _size){
			
			/*如果前继和后继都是unused的，并且如果合并后大小满足size，考虑合并 */
		
			__merge_next(block);
			__merge_prev(block);

			/* 合并后block就消失了，但是还是可以使用它的结构，
			如果prev满足size，再看能不能split*/
			if(block->prev->size - _size >= (MEM_BLOCK_SIZE + 4))
				__split_block(block->prev, _size);

			/* 标记块 */
			flag = block->prev;

			/* 因为把block合并到prev中去了，所以要修改成prev的地址 */
			ptr = (struct __memory_block *)block->prev + 1;

			//printf("realloc: merge both size %d size %d\n", block->prev->size, _size);
		
			/* 与此同时还需要复制数据 */
			__copy_block_data(block, block->prev);
			
		} else { /* 以上都不满足，则malloc新区域 */
			newPtr = malloc(_size);
			if(!newPtr)
				return NULL;
			
			//内存复制
			newBlock = __get_block(newPtr);
			__copy_block_data(block, newBlock);
			
			free(ptr);//释放old 
			//printf("realloc: block size %d size %d\n", newBlock->size, _size);
			/* 标记块 */
			flag = newBlock;

			return newPtr;
		}
	}
	return ptr;//当前block数据区大于size时
}

/**
 * memory_state - 读取内存管理的状态
 */
void memory_state()
{
	struct __memory_block *current_block = head;
	while(current_block){
		printf("size: %x, used: %d,address: %x\n",current_block->size,current_block->used,current_block);
		current_block = current_block->next;
		if(current_block == head)
			break;
	}
	printf("\n");
}

/**
 * malloc_usable_size - 获取一块内存占用的大小
 * @ptr: 内存地址指针
 * 
 * 占用大小 = 内存地址 + 内存块结构体
 */
int malloc_usable_size(void *ptr)
{
	struct __memory_block *current_block = (struct __memory_block *)(ptr - sizeof(struct __memory_block));
	/* 返回块的大小和结构体的大小 */
	return current_block->size + sizeof(struct __memory_block);
}

size_t getmsz(void *ptr)
{
    if (!ptr) {
		return 0;
	}
	struct __memory_block *block;
	//得到对应的block
	block = __get_block(ptr);
    return block->size;
}

int setmsz(void *ptr, size_t size)
{
    if (!ptr || !size) {
		return -1;
	}
	struct __memory_block *block;
	//得到对应的block
	block = __get_block(ptr);
    block->size = size;
    return 0;
}

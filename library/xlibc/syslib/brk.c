
#include <unistd.h>
#include <stdint.h>
#include <types.h>
#include <stdio.h>
#include <sys/vmm.h>

/* This must be initialized data because commons can't have aliases.  */
static void *__curbrk = NULL;

/**
 * brk - 设置断点值
 * @addr: 设置的地址
 * 
 * 成功返回0，失败返回-1
 */
int brk(void *addr)
{
  	void *newbrk;

	/* 保存新的brk值 */
	__curbrk = newbrk = (void *) heap(addr);
	
    //printf("__brk: %x\n", __curbrk);
	/* 检测执行后的结果 */
	if (newbrk < addr) {
      	return -1;
    }

 	return 0;
}

/**
 * __sbrk - 让断点移动位置
 * @increment: 移动的距离，可正可负
 * 
 * 返回移动前的brk指针位置
 * 成功则返回brk地址，失败则返回-1指针，而不是NULL
 */
void *sbrk(int increment)
{
  	void *oldbrk;
  	/* If this is not part of the dynamic library or the library is used
	via dynamic loading in a statically linked program update
	__curbrk from the kernel's brk value.  That way two separate
	instances of brk and sbrk can lib the heap, returning
	interleaved pieces of it.  */
  	
	/* If have not Initialized */
	if (__curbrk == NULL)
    	if (brk (0) < 0)      /* Initialize the break.  */
      		return (void *) -1;
	
	/* If no increment, return current brk. */
  	if (increment == 0)
    	return __curbrk;

	/* Get brk */
  	oldbrk = __curbrk;

	/* If brk not right or __brk failed, return -1. */
  	if ((increment > 0 ?
    	((uint32_t) oldbrk + (uint32_t) increment < (uint32_t) oldbrk) :
       	((int32_t)oldbrk < -((int32_t)increment))) ||
      	brk ((void *)((int32_t)oldbrk + (int32_t)increment)) < 0)
    	return (void *) -1;
	
	// printf("__sbrk: %x\n", oldbrk);
	
	return oldbrk;
}
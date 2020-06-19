#include <sys/syscall.h>
#include <sys/vmm.h>

/**
 * heap() - memory heap operate
 * 
 * @heap: heap value
 * 
 * if heap = 0, return current heap value.
 * if heap > old heap, expand heap up.
 * if heap < old heap, shrink heap down.
 * if heap = old heap, do nothing.
 * 
 * @return: always return newest heap value
 */
void *heap(void *heap)
{
    return syscall1(void *, SYS_HEAP, heap);
}


/**
 * munmap() - ummap memory range
 * @addr: start addr
 * @length: memory length
 * 
 * @return: success is 0, failed is -1 
 */
int munmap(void *addr, size_t length)
{
    return syscall2(int , SYS_MUNMAP, addr, length);
}


/**
 * mstate() - get memory state
 * @ms: memory state
 * 
 * @return: success is 0, failed is -1 
 */
int mstate(mstate_t *ms)
{
    return syscall1(int , SYS_MSTATE, ms);
}

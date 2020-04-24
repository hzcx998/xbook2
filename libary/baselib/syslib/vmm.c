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

void *malloc(size_t size)
{
    return NULL;
}

void free(void *ptr)
{

}

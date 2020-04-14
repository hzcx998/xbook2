#include <xcore/syscall.h>
#include <xcore/xcore.h>

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
unsigned long heap(unsigned long heap)
{
    return syscall1(unsigned long, SYS_HEAP, heap);
}

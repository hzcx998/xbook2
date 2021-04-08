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

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    mmap_args_t args = {addr, length, prot, flags, fd, offset};
    return syscall1(void * , SYS_MMAP, &args);
}

/**
 * xmmap() - memory map for file for xbook
 * @fd: file descriptor
 * @length: file length
 * @flags: map flags
 * @return: success is 0, failed is -1 
 */
void *xmmap(int fd, size_t length, int flags)
{
    return mmap(NULL, length, 0, flags, fd, 0); // prot = PROT_READ | PROT_WRITE
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
 * xmunmap() - ummap memory range for xbook
 * @addr: start addr
 * @length: memory length
 * 
 * @return: success is 0, failed is -1 
 */
int xmunmap(void *addr, size_t length)
{
    return munmap(addr, length);
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

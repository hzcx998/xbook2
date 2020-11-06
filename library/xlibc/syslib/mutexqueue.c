#include <sys/syscall.h>
#include <sys/mutexqueue.h>

/**
 * mutex_queue_alloc - 创建一个等待队列
 * 
 * 返回一个等待队列句柄，成功返回>=0，失败返回-1
 */
int mutex_queue_alloc()
{
    return syscall0(int, SYS_MUTEX_QUEUE_CREATE);
}
/**
 * mutex_queue_free - 销毁一个等待队列
 * @handle: 句柄
 * 
 * 成功返回0，失败返回-1
 */
int mutex_queue_free(int handle)
{
    return syscall1(int, SYS_MUTEX_QUEUE_DESTROY, handle);
}

/**
 * mutex_queue_wait - 把自己添加到等待队列
 * @handle: 句柄
 * @value: 对地址里面的值进行操作
 * 
 * 成功返回0，失败返回-1
 */
int mutex_queue_wait(int handle, void *addr, unsigned int wqflags, unsigned long value)
{
    return syscall4(int, SYS_MUTEX_QUEUE_WAIT, handle, addr, wqflags, value);
}

/**
 * mutex_queue_wake - 从等待队列唤醒一个任务
 * @handle: 句柄
 * 
 * 成功返回0，失败返回-1
 */
int mutex_queue_wake(int handle, void *addr, unsigned int wqflags, unsigned long value)
{
    return syscall4(int, SYS_MUTEX_QUEUE_WAKE, handle, addr, wqflags, value);
}

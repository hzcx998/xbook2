#include <sys/syscall.h>
#include <sys/waitque.h>

/**
 * waitque_create - 创建一个等待队列
 * 
 * 返回一个等待队列句柄，成功返回>=0，失败返回-1
 */
int waitque_create()
{
    return syscall0(int, SYS_WAITQUE_CREATE);
}
/**
 * waitque_destroy - 销毁一个等待队列
 * @handle: 句柄
 * 
 * 成功返回0，失败返回-1
 */
int waitque_destroy(int handle)
{
    return syscall1(int, SYS_WAITQUE_DESTROY, handle);
}

/**
 * waitque_wait - 把自己添加到等待队列
 * @handle: 句柄
 * @value: 对地址里面的值进行操作
 * 
 * 成功返回0，失败返回-1
 */
int waitque_wait(int handle, void *addr, unsigned int wqflags, unsigned long value)
{
    return syscall4(int, SYS_WAITQUE_WAIT, handle, addr, wqflags, value);
}

/**
 * waitque_wake - 从等待队列唤醒一个任务
 * @handle: 句柄
 * 
 * 成功返回0，失败返回-1
 */
int waitque_wake(int handle, void *addr, unsigned int wqflags, unsigned long value)
{
    return syscall4(int, SYS_WAITQUE_WAKE, handle, addr, wqflags, value);
}

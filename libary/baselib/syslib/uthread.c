#include <sys/syscall.h>
#include <sys/proc.h>
#include <sys/uthread.h>
#include <sys/vmm.h>
#include <stdio.h>

/* 线程入口，启动线程后会先进入入口执行 */
void __uthread_entry();

int uthread_make_default_attr(uthread_attr_t *attr)
{
    attr->stacksize = UTHREAD_STACKSIZE_DEL;
    attr->stackaddr = heap(0); /* 分配空间 */
    heap(attr->stackaddr + attr->stacksize);
    attr->detachstate = UTHREAD_CREATE_JOINABLE;
    return 0;
}  

/**
 * uthread_create - 创建一个线程
 * 
 */
uthread_t uthread_create(
    uthread_attr_t *attr,
    void * (*start_routine) (void *),
    void *arg
) {
    uthread_attr_t default_attr;
    if (attr == NULL) {
        /* 自动分配属性 */
        if (uthread_make_default_attr(&default_attr) == -1)
            return -1;
        attr = &default_attr;
    }
    return syscall4(int, SYS_THREAD_CREATE, attr, start_routine, arg, __uthread_entry);
}

void uthread_exit(void *retval)
{
    syscall1(int, SYS_THREAD_EXIT, retval);
}

void __uthread_exit(void *retval)
{
    printf("uthread_exit: exit val %x\n", retval);
    uthread_exit(retval);
}

int uthread_detach(uthread_t thread)
{
    return syscall1(int, SYS_THREAD_DETACH, thread);    
}

int uthread_join(uthread_t thread, void **thread_return)
{
    return syscall2(int, SYS_THREAD_JOIN, thread, thread_return);        
}

uthread_t uthread_self(void)
{
    return (uthread_t) gettid();
}

/**
 * uthread_equal - 判断两个线程是否相等
 * @thread1: 线程1
 * @thread2: 线程2
 * 
 * @return: 相等返回1，不相等返回0
 */
int uthread_equal(uthread_t thread1, uthread_t thread2)
{
    return (thread1 == thread2);
}

/**
 * uthread_cancel - 取消线程
 * @thread: 线程
 * 
 * 发送终止信号给thread线程，如果成功则返回0，否则为非0值。发送成功并不意味着thread会终止。 
 */
int uthread_cancel(uthread_t thread)
{
    return syscall1(int , SYS_THREAD_CANCEL, thread);
}

/**
 * uthread_setcancelstate - 设置取消状态
 * @state: 状态：UTHREAD_CANCEL_ENABLE（缺省）, 收到信号后设为CANCLED状态
 *              UTHREAD_CANCEL_DISABLE, 忽略CANCEL信号继续运行             
 * @oldstate: 原来的状态,old_state如果不为NULL则存入原来的Cancel状态以便恢复。 
 * 
 * 成功返回0，失败返回-1
 */
int uthread_setcancelstate(int state, int *oldstate)
{
    return syscall2(int , SYS_THREAD_CANCELSTATE, state, oldstate);
}

/**
 * uthread_setcanceltype - 设置取消动作的执行时机
 * @type: 取消类型，2种结果：UTHREAD_CANCEL_DEFFERED，收到信号后继续运行至下一个取消点再退出
 *                          UTHREAD_CANCEL_ASYCHRONOUS，立即执行取消动作（退出）
 * @oldtype: oldtype如果不为NULL则存入原来的取消动作类型值。 
 * 
 * 成功返回0，失败返回-1
 */
int uthread_setcanceltype(int type, int *oldtype)
{
    return syscall2(int , SYS_THREAD_CANCELTYPE, type, oldtype);
}

/**
 * uthread_testcancel - 检测测试点
 * 
 * 检查本线程是否处于Canceld状态，如果是，则进行取消动作，否则直接返回。
 */
void uthread_testcancel(void)
{
    syscall0(int , SYS_THREAD_TESTCANCEL);
}

int uthread_attr_getdetachstate(const uthread_attr_t *attr, int *detachstate)
{
    *detachstate = attr->detachstate; 
    return 0;
}

int uthread_attr_setdetachstate(uthread_attr_t *attr, int detachstate)
{
    attr->detachstate = detachstate;
    return 0;
}

int uthread_attr_init(uthread_attr_t *attr)
{
    attr->stacksize = UTHREAD_STACKSIZE_DEL;
    attr->stackaddr = NULL;
    attr->detachstate = UTHREAD_CREATE_JOINABLE;
    return 0;
}

int uthread_attr_getstacksize(const uthread_attr_t *attr, size_t *stacksize)
{
    *stacksize = attr->stacksize; 
    return 0;
}

int uthread_attr_setstacksize(uthread_attr_t *attr, size_t stacksize)
{
    attr->stacksize = stacksize;
    return 0;
}

int uthread_attr_getstackaddr(const uthread_attr_t *attr, void **stackaddr)
{
    *stackaddr = attr->stackaddr; 
    return 0;
}

int uthread_attr_setstackaddr(uthread_attr_t *attr, void *stackaddr)
{
    attr->stackaddr = stackaddr;
    return 0;
}
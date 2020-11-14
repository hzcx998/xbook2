#include <sys/syscall.h>
#include <sys/proc.h>
#include <sys/vmm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

/* 线程入口，启动线程后会先进入入口执行 */
void __pthread_entry();

int pthread_make_default_attr(pthread_attr_t *attr)
{
    attr->stacksize = PTHREAD_STACKSIZE_DEL;
    attr->stackaddr = malloc(attr->stacksize); /* 分配空间 */
    if (attr->stackaddr == NULL)
        return -1;
    memset(attr->stackaddr, 0, attr->stacksize);
    attr->detachstate = PTHREAD_CREATE_JOINABLE;
    return 0;
}  

/**
 * pthread_create - 创建一个线程
 * 
 */
int pthread_create(
    pthread_t *thread,
    pthread_attr_t *attr,
    void * (*start_routine) (void *),
    void *arg
) {
    if (!thread || !start_routine)
        return EINVAL;

    pthread_attr_t default_attr;
    if (attr == NULL) {
        /* 自动分配属性 */
        if (pthread_make_default_attr(&default_attr) == -1)
            return -1;
        attr = &default_attr;
    }
    pid_t pid = syscall4(pid_t, SYS_THREAD_CREATE, attr, start_routine, arg, __pthread_entry);
    if (pid < 0)
        return -1;
    *thread = (pthread_t) pid;
    return 0;
}

void pthread_exit(void *retval)
{
    syscall1(int, SYS_THREAD_EXIT, retval);
}

void __pthread_exit(void *retval)
{
    //printf("pthread_exit: exit val %x\n", retval);
    pthread_exit(retval);
}

int pthread_detach(pthread_t thread)
{
    return syscall1(int, SYS_THREAD_DETACH, thread);    
}

int pthread_join(pthread_t thread, void **thread_return)
{
    return syscall2(int, SYS_THREAD_JOIN, thread, thread_return);        
}

pthread_t pthread_self(void)
{
    return (pthread_t) gettid();
}

/**
 * pthread_equal - 判断两个线程是否相等
 * @thread1: 线程1
 * @thread2: 线程2
 * 
 * @return: 相等返回1，不相等返回0
 */
int pthread_equal(pthread_t thread1, pthread_t thread2)
{
    return (thread1 == thread2);
}

/**
 * pthread_cancel - 取消线程
 * @thread: 线程
 * 
 * 发送终止信号给thread线程，如果成功则返回0，否则为非0值。发送成功并不意味着thread会终止。 
 */
int pthread_cancel(pthread_t thread)
{
    return syscall1(int , SYS_THREAD_CANCEL, thread);
}

/**
 * pthread_setcancelstate - 设置取消状态
 * @state: 状态：pthread_CANCEL_ENABLE（缺省）, 收到信号后设为CANCLED状态
 *              pthread_CANCEL_DISABLE, 忽略CANCEL信号继续运行             
 * @oldstate: 原来的状态,old_state如果不为NULL则存入原来的Cancel状态以便恢复。 
 * 
 * 成功返回0，失败返回-1
 */
int pthread_setcancelstate(int state, int *oldstate)
{
    return syscall2(int , SYS_THREAD_CANCELSTATE, state, oldstate);
}

/**
 * pthread_setcanceltype - 设置取消动作的执行时机
 * @type: 取消类型，2种结果：pthread_CANCEL_DEFFERED，收到信号后继续运行至下一个取消点再退出
 *                          pthread_CANCEL_ASYCHRONOUS，立即执行取消动作（退出）
 * @oldtype: oldtype如果不为NULL则存入原来的取消动作类型值。 
 * 
 * 成功返回0，失败返回-1
 */
int pthread_setcanceltype(int type, int *oldtype)
{
    return syscall2(int , SYS_THREAD_CANCELTYPE, type, oldtype);
}

/**
 * pthread_testcancel - 检测测试点
 * 
 * 检查本线程是否处于Canceld状态，如果是，则进行取消动作，否则直接返回。
 */
void pthread_testcancel(void)
{
    syscall0(int , SYS_THREAD_TESTCANCEL);
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
    if (!attr)
        return EINVAL;
    *detachstate = attr->detachstate; 
    return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
    if (!attr)
        return EINVAL;
    attr->detachstate = detachstate;
    return 0;
}

int pthread_attr_init(pthread_attr_t *attr)
{
    if (!attr)
        return EINVAL;
    attr->stacksize = PTHREAD_STACKSIZE_DEL;
    attr->stackaddr = NULL;
    attr->detachstate = PTHREAD_CREATE_JOINABLE;
    return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
    if (!attr)
        return EINVAL;
    if (attr->stackaddr)
        free(attr->stackaddr);
    attr->stackaddr = NULL;
    attr->stacksize = 0;
    attr->detachstate = 0;
    return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
    if (!attr)
        return EINVAL;
    if (stacksize)
        *stacksize = attr->stacksize; 
    return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
    if (!attr)
        return EINVAL;
    attr->stacksize = stacksize;
    return 0;
}

int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr)
{
    if (!attr)
        return EINVAL;
    if (stackaddr)
        *stackaddr = attr->stackaddr; 
    return 0;
}

int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr)
{
    if (!attr)
        return EINVAL;
        
    if (attr->stackaddr){
        free(attr->stackaddr);  /* free old stack */
    }
    attr->stackaddr = stackaddr;
    return 0;
}

void pthread_cleanup_push(void (*routine)(void *), void *arg)
{
    // TODO: add cleanup func here
}
void pthread_cleanup_pop(int execute)
{
    // TODO: do cleanup func
}

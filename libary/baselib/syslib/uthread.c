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
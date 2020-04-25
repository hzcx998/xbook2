#include <sys/res.h>
#include <sys/vmm.h>
#include <sys/proc.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/uthread.h>
#include <sys/trigger.h>
#include <sys/time.h>

#define TTY_NAME    "tty0"
#define DISK_NAME   "ide0"

/* login arg */
#define BIN_OFFSET      200
#define BIN_SECTORS     100
#define BIN_SIZE        (BIN_SECTORS*512)
#define BIN_NAME        "login"

void test();

int main(int argc, char *argv[])
{
    res_open(TTY_NAME, RES_DEV, 0);
    res_open(TTY_NAME, RES_DEV, 0);
    res_open(TTY_NAME, RES_DEV, 0);
    res_ioctl(RES_STDINNO, TTYIO_CLEAR, 0);
    printf("init: say, hello!\n");
    
    int pid = fork();
    if (pid < 0) {
        printf("init: fork failed! exit now!\n");
        return -1;
    }

    if (pid > 0) {  /* parent */
        //printf("init-parent: pid is %d, my child pid is %d.\n", getpid(), pid);
        while (1) { /* loop */
#if 0
            int status = 0;
            pid = wait(&status);    /* wait one child exit */
            if (pid > 1) {
                printf("init: my child pid %d exit with status %d\n", pid, status);
            }
#endif
            int status = 0;
            int _pid;
            _pid = waitpid(-1, &status, 0);    /* wait one child exit */
            if (_pid > 1) {
                printf("init: my child pid %d exit with status %d\n", _pid, status);
            }
        }
    } else {
        /* execute a process */
        printf("init-child: pid is %d, my parent pid is %d.\n", getpid(), getppid());
        //exit(123);
        test();
        /* open disk */
        int ide0 = res_open(DISK_NAME, RES_DEV, 0);
        if (ide0 < 0) {
            printf("init-child: open disk '%s' failed! exit now.", DISK_NAME);
            return -1;
        }
        /* alloc memory for file */
        unsigned char *heap_pos = heap(0);
        //printf("init-child: heap addr %x.\n", heap_pos);

        heap(heap_pos + BIN_SIZE); // 40 kb memory

        unsigned char *buf = heap_pos;
        memset(buf, 0, BIN_SIZE);
       // printf("init-child: alloc data at %x for 40 kb.\n", buf);
        
        /* read disk sector for file: offset=200, sectors=50 */
        if (res_read(ide0, BIN_OFFSET, buf, BIN_SIZE) < 0) {
            printf("init-child: read disk sectors failed! exit now\n");
            return -1;
        }
        //printf("init-child: load data success.\n");
        
        kfile_t file = {buf, BIN_SIZE};
        /*int i;
        for (i = 0; i < 32; i++) {
            printf("%x ", buf[i]);
        }*/
        //printf("\ninit-child: free resource.\n");
        res_close(ide0); /* close ide0 */

        char *_argv[4] = {BIN_NAME, "xbook", "1234", 0};
        exit(execfile(BIN_NAME, &file, _argv));
    }
    return 0;
}

void alarm_handle(int trig)
{
    printf("alarm: %d\n", trig);
    //exit(-1);
}

void thread_exit(void *arg)
{
    printf("thread_exit: exit val %x\n", arg);

}


void *thread_test2(void *arg)
{
    printf("hello, deep %d!\n%s\n", uthread_self(), arg);
    /*
    trigger(TRIGALARM, alarm_handle);
    alarm(1);*/
    //triggeron(TRIGALARM, getpid());
    sleep(1);
    //exit(123);
    //uthread_detach(uthread_self());
    //sleep(1);
    //uthread_detach(gettid());
    //printf("set alarm done!\n");
    /*void *status;
    uthread_t pid = (uthread_t ) arg;
    uthread_join(pid, status);
    printf("thread: join done! status=%d", status);*/
    return (void *)123;
}



void *thread_test(void *arg)
{
    printf("hello, uthread %d!\n%s\n", gettid(), arg);
    /*
    trigger(TRIGALARM, alarm_handle);
    alarm(1);*/
    //triggeron(TRIGALARM, getpid());
    sleep(1);


    uthread_t tid = uthread_create(NULL, thread_test2, "hello, test!");
    printf("init: create thread %d\n", tid);
    void *status;
    uthread_join(tid, NULL);

    //exit(123);
    //uthread_detach(uthread_self());
    //sleep(1);
    //uthread_detach(gettid());
    //printf("set alarm done!\n");
    /*void *status;
    uthread_t pid = (uthread_t ) arg;
    uthread_join(pid, status);
    printf("thread: join done! status=%d", status);*/
    return (void *)123;
}

void test()
{
    printf("init: testing...\n");

    unsigned char *stack_top;
    uthread_attr_t attr;
    /*int i;
    for (i = 0; i < 10; i++) {*/
    uthread_attr_init(&attr);
    uthread_attr_setstacksize(&attr, 4096);
    /* get heap start addr */
    stack_top = heap(0);
    /* expand heap */
    heap(stack_top + attr.stacksize);
    memset(stack_top, 0, attr.stacksize);
    uthread_attr_setstackaddr(&attr, stack_top);
    uthread_attr_setdetachstate(&attr, UTHREAD_CREATE_DETACHED);
    
    uthread_t tid = uthread_create(&attr, thread_test, "hello, THREAD1!");
    printf("init: create thread %d\n", tid);
    uthread_t tid2 = uthread_create(NULL, thread_test, "hello, THREAD2!");
    printf("init: create thread %d\n", tid2);
    
    //sleep(3);
    
    //sleep(2);
    void *retval = 0;
    uthread_detach(tid);
    uthread_join(tid, &retval);
    printf("init: %d uthread_join %d status %x done!\n", getpid(), tid, retval);
    uthread_join(tid2, &retval);
    printf("init: %d uthread_join %d status %x done!\n", getpid(), tid2, retval);
    
    sleep(3);
    //uthread_exit((void *)25);
    //}    
    while (1)
    {
        /* code */
    }
    
}
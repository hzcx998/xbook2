#include <sys/res.h>
#include <sys/vmm.h>
#include <sys/proc.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/pthread.h>
#include <sys/trigger.h>
#include <sys/time.h>

#define TTY_NAME    "tty0"
//#define DISK_NAME   "ide0"
#define DISK_NAME   "vfloppy"

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


void *thread_test2(void *arg)
{
    printf("thread_test2: hello, thread %d, arg=%x\n", pthread_self(), (unsigned int)arg);
    sleep(3);
    printf("thread_test2: will return soon!\n");
    return (void *)123;
}
void trigger_handler(int trig)
{
    printf("trigger_handler: trigger %d occur!\n", trig);

}

void *thread_test(void *arg)
{
    printf("thread_test: hello, thread %d, arg=%s\n", pthread_self(), arg);
    
    pthread_t tid;
    pthread_create(&tid, NULL, thread_test2, (void *)0x12345678);
    printf("thread_test: create thread %d\n", tid);
    
    pthread_join(tid, NULL);
    
    printf("thread_test: will return soon!\n");
    return (void *)pthread_self();
}

void test()
{
    printf("init: testing...\n");
    unsigned char *stack_top;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 4096);
    /* get heap start addr */
    stack_top = heap(0);
    /* expand heap */
    heap(stack_top + attr.stacksize);
    memset(stack_top, 0, attr.stacksize);
    pthread_attr_setstackaddr(&attr, stack_top);
    /* create the thread */
    pthread_t tid;
    pthread_create(&tid, &attr, thread_test, "hello, THREAD1!");
    printf("init: create thread %d\n", tid);
    void *retval = 0;
    trigger(TRIGALARM, trigger_handler);
    alarm(1);
    /* wait tig thread */
    pthread_join(tid, &retval);
    printf("init: %d pthread_join %d status %x done!\n", pthread_self(), tid, retval);
    printf("init: will return!\n");
}

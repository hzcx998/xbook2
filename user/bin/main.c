#include <sys/xbook.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <conio.h>
#include <string.h>

int func(int n)
{   
    if (n == 0 || n == 1)
        return 1; 
    else 
        return n * func(n - 1); 
}

void delay(int t)
{
    int i, j;
    for (i = 0; i < 100 * t; i++) {
        for (j = 0; j < 100; j++) {
            
        }    
    }
}

#define SEM_LOCK 0

void test_sem()
{
    int semid = x_semget("sem_test", 0, IPC_CREAT);
    if (semid < 0) {
        printf("bin: get sem failed!");
        return;
    }
    printf("bin: get sem %d.\n", semid);
    while (1)
    {
        
        //delay(1);
#if SEM_LOCK == 1        
        x_semdown(semid, 0);
#endif
        printf("bin: abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcd.\n");
#if SEM_LOCK == 1        
        x_semup(semid);
#endif  
    }
}

x_dev_t dev;

int main(int argc, char *argv[])
{
    
    dev = x_open("con0", 0);
    //x_ioctl(dev, CONIO_CLEAN, 0);
    
    int i = 0;
    while (i < argc)
    {
        printf("\n-%s ", argv[i]);    
        i++;
    }
    //test_sem();
    x_trigger(TRIGHSOFT, TRIG_IGN);
    x_trigger(TRIGUSR0, TRIG_IGN);
    x_trigger(TRIGLSOFT, TRIG_IGN);
    x_trigger(TRIGUSR1, TRIG_IGN);

    x_triggeron(TRIGLSOFT, x_getpid());
    
    x_triggeron(TRIGUSR1, x_getpid());

    trig_action_t act = {TRIG_IGN, 0};
    trig_action_t oldact;

    x_trigger_action(TRIGUSR0, &act, &oldact);
    printf("bin: handler=%x flags=%x\n", act.handler, act.flags);
    printf("bin: old handler=%x flags=%x\n", oldact.handler, oldact.flags);
    
    x_trigger_action(TRIGLSOFT, &oldact, &act);
    
    printf("bin: handler=%x flags=%x\n", act.handler, act.flags);
    printf("bin: old handler=%x flags=%x\n", oldact.handler, oldact.flags);
    
    x_trigger(TRIGUSR0, TRIG_IGN);
    x_trigger(TRIGLSOFT, TRIG_IGN);
    
    x_triggeron(TRIGPAUSE, x_getpid());
    //x_triggeron(TRIGHSOFT, x_getpid());

    /*
    int a = 0;
    int b = a/0;
    */
#if 0 /* SHM */
    int shmid = x_shmget("shm_test", 0, IPC_CREAT);
    if (shmid < 0) {
        printf("bin: child: get shm failed!");
        return -1;
    }

    printf("bin: child: get shm %d.", shmid);
    unsigned char *shmaddr = x_shmmap(shmid, NULL);
    if (shmaddr == (void *)-1) {
        printf("bin: child: map shm failed!");
        return -1;
    }
    printf("bin: child: map shm %x.", shmaddr);

    /*for (i = 0; i < 4096; i++) {
        shmaddr[i] = i % 255;
    }*/
    for (i = 0; i < 16; i++) {
        printf("%x ", shmaddr[i]);
    }
    printf("bin: child: test shm done!");
    
    x_shmunmap(shmaddr);
    
    x_shmput(shmid);
#endif /* SHM */
    /*
    shmid = x_shmget("shm_test", 0, SHM_CREAT | SHM_EXCL);
    if (shmid < 0) {
        printf("bin: child: get new shm failed!");
        return -1;
    }*/
#if 0 /* MSG */
    int msgid = x_msgget("usr_test", IPC_CREAT);
    if (msgid < 0) {
        printf("BIN: parent: get msg failed!");
        return -1;
    }
    printf("BIN: parent: get msg %d.", msgid);
    
    unsigned long heap = x_heap(0);
    x_heap(heap + 1024 + sizeof(x_msgbuf_t));

    x_msgbuf_t *msgbuf = (x_msgbuf_t *) heap;
    while (1)
    {
        int j, k;
        for (j = 0; j < 1000; j++) {
            for (k = 0; k < 10000; k++);
        }
        msgbuf->type = 0;
        memset(msgbuf->text, 0, 1024);
        int read = x_msgrcv(msgid, msgbuf, 1024, 100, 0);
        printf("BIN: parent: rcv msg %d type=%d!\n", read, msgbuf->type);
        for (i = 0; i < 16; i++) {
            printf("%x ", msgbuf->text[i]);
        }
        printf("\n");
    }
    
    printf("BIN: parent: rcv msg ok!");
#endif /* MSG */
#if 0 /* HEAP */
    
    log("in bin\n");
    printf("hello, printf! %d %s %x\n", 123, "xbook", 0xff1234cd);

    func(1);
    
    x_putc(dev, '!');
    x_putc(dev, '@');
     unsigned long heap = x_heap(0);
    printf("heap addr:%x\n", heap);

    x_heap(heap + 4096);
    printf("heap addr:%x\n", heap);
    unsigned char *buf = (unsigned char *) heap;
    memset(buf, 0, 4096);

    heap = x_heap(0);
    x_heap(heap + 4096 * 10);
    printf("heap addr:%x\n", heap);
    buf = (unsigned char *) heap;
    memset(buf, 0, 4096 * 10);

    heap = x_heap(0);
    printf("heap addr:%x\n", heap);
    
    x_heap(heap + 4096 * 1000);
    printf("heap addr:%x\n", heap);
    buf = (unsigned char *) heap;
    memset(buf, 0, 4096 * 1000);
    heap = x_heap(0);
    printf("heap addr:%x\n", heap);
#endif  /* HEAP */
    int pid = x_fork();
    if (pid > 0) {
        printf("bin-parent!\n");
        //x_close(dev);
        x_exit(12345);
    } else {
        printf("bin-child!\n");
        x_close(dev);
        x_exit(0x12345);
    }
    return 0;
}

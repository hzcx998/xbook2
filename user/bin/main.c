#include <sys/xcore.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <conio.h>
#include <string.h>

void semtest1()
{
    int semid = getres("sem_test", RES_IPC | IPC_SEM | IPC_CREAT, 1);
    if (semid < 0)
        exit(-1);
    printf("get sem id %d\n", semid);
    while (1)
    {
        writeres(semid, 0, NULL, 0);
        printf("bin1:  abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg-%d\n", semid);
        printf("bin2:  abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg-%d\n", semid);
        printf("bin3:  abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg-%d\n", semid);
        printf("bin4:  abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg-%d\n", semid);
        printf("bin5:  abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg-%d\n", semid);
        readres(semid, 0, NULL, 0);
    }
}

void msgtest()
{
    
    int msgid = getres("msg_test", RES_IPC | IPC_MSG | IPC_CREAT, 0);
    if (msgid < 0) {
        printf("test: parent: get msg failed!");
        return;
    }
    printf("bin: parent: get msg %d.\n", msgid);
    
    unsigned long hp = heap(0);
    heap(hp + 1024 + sizeof(x_msgbuf_t));

    x_msgbuf_t *msgbuf = (x_msgbuf_t *) hp;
    while (1)
    {
        int i;
        /*
        int i, j, k;
        for (j = 0; j < 1000; j++) {
            for (k = 0; k < 10000; k++);
        }*/
        msgbuf->type = 100; /* 读取消息的类型 */
        memset(msgbuf->text, 0, 1024);
        int read = readres(msgid, 0, msgbuf, 1024);
        printf("bin: parent: rcv msg %d type=%d!\n", read, msgbuf->type);
        for (i = 0; i < 16; i++) {
            printf("%x ", msgbuf->text[i]);
        }
        printf("\n");
    }
    
    printf("bin: parent: rcv msg ok!");

}

void shmtest2()
{ 
    /* 共享内存 */
    int shmid2 = getres("test", RES_IPC | IPC_SHM | IPC_CREAT, 4095);
    if (shmid2 < 0)
        exit(-1);

    printf("get shm id %d\n", shmid2);
    unsigned long shmaddr2;
    if (writeres(shmid2, 0, NULL, (size_t )&shmaddr2))
        printf("1: shm map failed!\n");

    printf("shm map at %x\n", shmaddr2);
    
    int *shmdata = (int *)shmaddr2;
    int j;
    for (j = 0; j < 16; j++) {
        printf("%x ", shmdata[j]);
    }
    readres(shmid2, 0, shmaddr2, 0);
}
int handle_count = 0;
void trigger_handler(int trig)
{
    printf("bin: trigger %d handled.\n", trig);
    handle_count++;
    if (handle_count > 10)
        triggeron(TRIGHSOFT, getpid());
}

void trigger_test()
{

    /*
    int a = 0;
    int b = a / 0;

    char *addr = (char *)0;
    *addr = 0;
    */

    trigger(TRIGHW, trigger_handler);
    trigger(TRIGLSOFT, TRIG_IGN);

    trig_action_t act = {trigger_handler, 0};
    trig_action_t oldact;
    
    trigger_action(TRIGHW, &act, &oldact);
    printf("oldact: handler=%x flags=%x\n", oldact.handler, oldact.flags);
    printf("act: handler=%x flags=%x\n", act.handler, act.flags);
    
    char *addr = (char *)0;
    *addr = 0;
    
    int a = 0;
    int b = a / 0;

    triggeron(TRIGUSR0, getpid());
    triggeron(TRIGLSOFT, getpid());
    triggeron(TRIGHW, getpid());
    
}

void pipe_test_read()
{
    int pip = getres("pipe_test", RES_IPC | IPC_PIPE | IPC_CREAT | IPC_READER, 0);
    if (pip < 0)
        return;
    char buf[8192];
    memset(buf, 0, 8192);
    //while (1) {
        int i, j;
        for (i = 0; i < 300; i++)
            for (j = 0; j < 100000; j++);
            
        int read = readres(pip, IPC_NOSYNC, buf, 4096);

        printf("child read pipe done! %d bytes.\n", read);
        for (i = 0; i < 32; i++) {
            printf("%c ", buf[i]);
        }
        printf("\n");
        
    //}
    putres(pip);
}

int main(int argc, char *argv[])
{
    printf("hello, bin!\n");
    
    pipe_test_read();
    trigger_test();
    
    shmtest2();
    //semtest1();
    //msgtest();
    putres(1);
    
    return 0;   
}

#if 0
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
#endif
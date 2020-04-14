#include <xcore/xcore.h>
#include <xcore/ipc.h>
#include <string.h>
#include <conio.h>

int func(int n)
{   
    if (n == 0 || n == 1)
        return 1; 
    else 
        return n * func(n - 1); 
}
#if 0
x_dev_t dev;

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
    int semid = x_semget("sem_test", 1, IPC_CREAT);
    if (semid < 0) {
        printf("test: parent: get sem failed!");
        return;
    }
    printf("test: parent: get sem %d.\n", semid);
    /*semid = x_semget("sem_test2", 1, IPC_CREAT);
    if (semid < 0) {
        printf("test: parent: get sem failed!");
        return;
    }
    printf("test: parent: get sem %d.\n", semid);*/
    /*x_semput(semid);
    semid = x_semget("sem_test", 1, IPC_CREAT | IPC_EXCL);
    if (semid < 0) {
        printf("test: parent: get sem failed!");
        return;
    }*/
    printf("test: parent: get sem %d.\n", semid);
    while (1)
    {
        //delay(1);
#if SEM_LOCK == 1        
        x_semdown(semid, 0);
#endif
        printf("test: 1234567123456712345671234567123456712345671234567.\n");
#if SEM_LOCK == 1        
        x_semup(semid);
#endif  
    }
}

#endif
void shmtest1()
{
    
    /* 共享内存 */
    int shmid = getres("test", RES_IPC | IPC_SHM | IPC_CREAT, 4095);
    if (shmid < 0)
        exit(-1);

    printf("get shm id %d\n", shmid);
    unsigned long shmaddr;
    if (writeres(shmid, 0, NULL, (size_t )&shmaddr))
        printf("1: shm map failed!\n");

    printf("shm map at %x\n", shmaddr);
    
    memset(shmaddr, 1, 4096);

    char *mapv = (char *)shmaddr;
    //memset(mapv, 0, 4096);
    if (writeres(shmid, 0, (void *)0x20100000, 0))
        printf("2: shm map failed!\n");
    mapv = (char *)0x20100000;

    printf("shm map at %x\n", 0x20100000);
    
    if (readres(shmid, 0, shmaddr, 0))
        return -1;
    printf("shm unmap at %x.\n", shmaddr);
    if (readres(shmid, 0, mapv, 0))
        return -1;
    printf("shm unmap at %x.\n", mapv);
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

void semtest1()
{
    int semid = getres("sem_test", RES_IPC | IPC_SEM | IPC_CREAT, 1);
    if (semid < 0)
        exit(-1);
    printf("get sem id %d\n", semid);
    while (1)
    {
        writeres(semid, 0, NULL, 0);
        printf("test1: 1234567123456712345671234567123456712345671234567-%d\n", semid);
        printf("test2: 1234567123456712345671234567123456712345671234567-%d\n", semid);
        printf("test3: 1234567123456712345671234567123456712345671234567-%d\n", semid);
        printf("test4: 1234567123456712345671234567123456712345671234567-%d\n", semid);
        printf("test5: 1234567123456712345671234567123456712345671234567-%d\n", semid);
        readres(semid, 0, NULL, 0);
    }
    
}

void msgtest()
{
#if 1 /* MSG */
    int msgid = getres("msg_test", RES_IPC | IPC_MSG | IPC_CREAT, 0);
    if (msgid < 0) {
        printf("test: parent: get msg failed!");
        return;
    }
    printf("test: parent: get msg %d.", msgid);
    
    unsigned long hp = heap(0);
    heap(hp + 1024 + sizeof(x_msgbuf_t));

    x_msgbuf_t *msgbuf = (x_msgbuf_t *) hp;
    while (1)
    {
        
        int j, k;
        for (j = 0; j < 1000; j++) {
            for (k = 0; k < 10000; k++);
        }
        
        msgbuf->type = 100;
        memset(msgbuf->text, 0x1f, 1024);
        int i;
        for (i = 0; i < 16; i++) {
            msgbuf->text[i] = i;
        }
        if (writeres(msgid, 0, msgbuf, 1024)) {
            printf("test: parent: snd msg failed!\n");
        }
    }
    printf("test: parent: snd msg ok!");
#endif /* MSG */
}

void msgtest2()
{
    
    int msgid = getres("msg_test", RES_IPC | IPC_MSG | IPC_CREAT, 0);
    if (msgid < 0) {
        printf("test: parent: get msg failed!");
        return;
    }
    printf("test: child: get msg %d.\n", msgid);
    
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
        printf("test: child: rcv msg %d type=%d!\n", read, msgbuf->type);
        for (i = 0; i < 16; i++) {
            printf("%x ", msgbuf->text[i]);
        }
        printf("\n");
    }
    
    printf("test: child: rcv msg ok!");

}
void trigger_test()
{

    trigger(TRIGHW, TRIG_IGN);
    trigger(TRIGLSOFT, TRIG_IGN);

    trig_action_t act = {TRIG_IGN, 0};
    trig_action_t oldact;
    
    trigger_action(TRIGUSR0, &act, &oldact);
    printf("oldact: handler=%x flags=%x\n", oldact.handler, oldact.flags);
    printf("act: handler=%x flags=%x\n", act.handler, act.flags);
    
    triggeron(TRIGLSOFT, getpid());
    triggeron(TRIGHW, getpid());
    
}

void pipe_test_write()
{
    int pip = getres("pipe_test", RES_IPC | IPC_PIPE | IPC_CREAT | IPC_WRITER, 0);
    if (pip < 0)
        return;
    //char *str = "hello, pipe!\n";
    char buf[8192];
    memset(buf, 0, 8192);

    //while (1)
    //{
        strcpy(buf, "hello, big pipe!\n");
        writeres(pip, IPC_NOSYNC, buf, 4096);
    //}
    
    printf("parent write pipe done!");
    //wait(NULL);
    putres(pip);
}

void pipe_test_read()
{
    int pip = getres("pipe_test", RES_IPC | IPC_PIPE | IPC_CREAT | IPC_READER, 0);
    if (pip < 0)
        return;
    char buf[8192];
    memset(buf, 0, 8192);
    while (1) {
        int i, j;
        for (i = 0; i < 300; i++)
            for (j = 0; j < 100000; j++);
            
        int read = readres(pip, 0, buf, 4096);

        printf("child read pipe done! %d bytes.\n", read);
        for (i = 0; i < 32; i++) {
            printf("%c ", buf[i]);
        }
        printf("\n");
        
    }
    putres(pip);
}


int main(int argc, char *argv[])
{

    func(100);

    int con = getres("con0", RES_DEV, 0);
    if (con < 0)
        return -1;
    con = getres("con0", RES_DEV, 0);
    if (con < 0)
        return -1;

    writeres(con, 0, "hello, console!\n", 16);
    printf("hello, printf!\n");
    ctlres(con, 123, 456);

    //trigger_test();


    // putres(con);
    char buf[10];
    readres(con, 0, buf, 10);
    //
    int pid = fork();
    if (pid > 0) {
        //shmtest1();
        //semtest1();
        //msgtest();
        pipe_test_write();

        int status;
        pid = wait(&status);
        printf("child %d exit with status=%d.\n", pid, status);

    } else {
        printf("I am child, I will load data.\n");
        
        //msgtest2();
        //shmtest2();
        /*pipe_test_read();
        exit(123);
        */
        unsigned long heap_pos = heap(0);
        printf("heap addr %x.\n", heap_pos);

        heap(heap_pos + 40 * 1024); // 40 kb 内存

        unsigned char *buf = (unsigned char *) heap_pos;
        memset(buf, 0, 40 * 1024);
        printf("alloc data at %x for 20 kb.\n", buf);

        int hd0 = getres("hd0", RES_DEV, 0);
        if (hd0 < 0) 
            exit(-1);

        /* 200 */
        if (readres(hd0, 200, buf, 50)) {
            printf("read disk failed!\n");
            exit(-1);
        }
        printf("load data success.\n");
        xfile_t file = {buf, 22*1024};
        int i;
        for (i = 0; i < 32; i++) {
            printf("%x ", buf[i]);
        }
        printf("ready run.\n");
        putres(hd0);

        char *_argv[4] = {"bin", "abc", "123", 0};
        //x_close(dev);
        execfile("bin", &file, _argv);
        //execraw("bin", _argv);
    }

    return 0;
    /*nt i;
    char *p;
    for (i = 0; i < argc; i++) {
        p = (char *)argv[i];
        while (*p++);
    }*/
#if 0
    dev = x_open("con0", 0);
     
    int i = 0;
    while (i < argc)
    {
        printf("\n-%s ", argv[i]);    
        
        i++;
    }
    //func(1000);
    printf("nice to meet you!");

/*
    unsigned long heap = heap(0);
    printf("heap addr:%x\n", heap);

    heap(heap + 4096);
    printf("heap addr:%x\n", heap);
    unsigned char *buf = (unsigned char *) heap;
    memset(buf, 0, 4096);

    heap = heap(0);
    heap(heap + 4096 * 10);
    printf("heap addr:%x\n", heap);
    buf = (unsigned char *) heap;
    memset(buf, 0, 4096 * 10);

    heap = heap(heap);
    printf("heap addr:%x\n", heap);

    heap = heap(0);
    printf("heap addr:%x\n", heap);

    heap(heap + 4096 * 100);
    printf("heap addr:%x\n", heap);
    buf = (unsigned char *) heap;
    memset(buf, 0, 4096 * 100);
    */
    //exit(0);
    int pid = x_fork();
    /*if (pid > 0) {
        log("parent!");
        int status;
        int pid2 = x_wait(&status);
        
        //msleep(3000);
        log("parent~\n");
    } else {
        log("child!");
        exit(123);
        
        log("child~\n");
    }
    pid = x_fork();*/
    if (pid > 0) {
        printf("I am parent, my child is %d.\n", pid);
        //test_sem();
       
        /*msgbuf->type = 0;
        memset(msgbuf->text, 0, 1024);
        int read = x_msgrcv(msgid, msgbuf, 1024, 10, IPC_EXCEPT);
        printf("test: parent: rcv msg %d type=%d!", read, msgbuf->type);
        for (i = 0; i < 16; i++) {
            printf("%x ", msgbuf->text[i]);
        }
        
        printf("test: parent: rcv msg ok!");*/
        
#if 0
        int shmid = x_shmget("shm_test", 4096, IPC_CREAT);
        if (shmid < 0) {
            printf("test: parent: get shm failed!");
            return -1;
        }
        
        printf("test: parent: get shm %d.", shmid);
        unsigned char *shmaddr = x_shmmap(shmid, NULL);
        if (shmaddr == (void *)-1) {
            printf("test: parent: map shm failed!");
            return -1;
        }
        
        printf("# test: parent: map shm %x.\n", shmaddr);
        int i;
        for (i = 0; i < 4096; i++) {
            shmaddr[i] = i % 255;
        }
        for (i = 0; i < 16; i++) {
            printf("%x ", shmaddr[i]);
        }
        x_shmunmap(shmaddr);
#endif
        int status;
        int pid2 = x_wait(&status);
        printf("I am parent, my child exit with %d.\n", status);
        
    } else {
        printf("I am child, I will load data.\n");
        
        unsigned long heap = heap(0);
        printf("heap addr %x.\n", heap);

        heap(heap + 40 * 1024); // 40 kb 内存

        unsigned char *buf = (unsigned char *) heap;
        memset(buf, 0, 40 * 1024);
        printf("alloc data at %x for 20 kb.\n", buf);

        x_dev_t dev_disk = x_open("hd0", 0);
        if (!dev_disk) 
            exit(-1);

        if (readres(dev_disk, 200, buf, 50)) {
            printf("read disk failed!\n");
            exit(-1);
        }
        printf("load data success.\n");
        x_file_t file = {buf, 22*1024};
        //int i;
        for (i = 0; i < 32; i++) {
            printf("%x ", buf[i]);
        }
        char *_argv[4] = {"bin", "abc", "123", 0};
        //x_close(dev);
        x_execfile("bin", &file, _argv);
    }
    
    //x_close(dev);
    /*pid = x_fork();
    if (pid > 0) {
        log("parent!");
        int status;
        int pid2 = x_wait(&status);
        
        //msleep(3000);
        log("parent~\n");
    } else {
        log("child!");
        exit(789);
        
        log("child~\n");
    }
    log("end~\n");

    exit(-1);
    */
    while (1) {
        /* 等待子进程 */
    }

    return 0;
#endif
}
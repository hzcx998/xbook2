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

    while (1)
    {
        strcpy(buf, "hello, big pipe!\n");
        writeres(pip, 0, buf, 4096);
    }
    
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
        //pipe_test_write();

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

        int hd0 = getres("ide0", RES_DEV, 0);
        if (hd0 < 0) 
            exit(-1);

        /* 200 */
        if (readres(hd0, 200, buf, 50 * 512) < 0) {
            printf("read disk failed!\n");
            exit(-1);
        }
        printf("load data success.\n");

        xfile_t file = {buf, 25*1024};
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
}
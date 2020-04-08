#include <sys/xbook.h>
#include <sys/ipc.h>
#include <string.h>
#include <conio.h>

int func(int n)
{   
    if (n == 0 || n == 1)
        return 1; 
    else 
        return n * func(n - 1); 
}

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

int main(int argc, char *argv[])
{
    /*nt i;
    char *p;
    for (i = 0; i < argc; i++) {
        p = (char *)argv[i];
        while (*p++);
    }*/
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

    heap = x_heap(heap);
    printf("heap addr:%x\n", heap);

    heap = x_heap(0);
    printf("heap addr:%x\n", heap);

    x_heap(heap + 4096 * 100);
    printf("heap addr:%x\n", heap);
    buf = (unsigned char *) heap;
    memset(buf, 0, 4096 * 100);
    */
    //x_exit(0);
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
       
#if 0 /* MSG */
        int msgid = x_msgget("usr_test", IPC_CREAT);
        if (msgid < 0) {
            printf("test: parent: get msg failed!");
            return -1;
        }
        printf("test: parent: get msg %d.", msgid);
        
        unsigned long heap = x_heap(0);
        x_heap(heap + 1024 + sizeof(x_msgbuf_t));

        x_msgbuf_t 
        *msgbuf = (x_msgbuf_t *) heap;
        while (1)
        {
            /*
            int j, k;
            for (j = 0; j < 1000; j++) {
                for (k = 0; k < 10000; k++);
            }
            */
            msgbuf->type = 100;
            memset(msgbuf->text, 0x1f, 1024);
            int i;
            for (i = 0; i < 16; i++) {
                msgbuf->text[i] = i;
            }
            if (x_msgsnd(msgid, msgbuf, 1024, 0)) {
                printf("test: parent: snd msg failed!");
            }
        }
        printf("test: parent: snd msg ok!");
#endif /* MSG */
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
        
        unsigned long heap = x_heap(0);
        printf("heap addr %x.\n", heap);

        x_heap(heap + 40 * 1024); // 40 kb 内存

        unsigned char *buf = (unsigned char *) heap;
        memset(buf, 0, 40 * 1024);
        printf("alloc data at %x for 20 kb.\n", buf);

        x_dev_t dev_disk = x_open("hd0", 0);
        if (!dev_disk) 
            x_exit(-1);

        if (x_read(dev_disk, 200, buf, 50)) {
            printf("read disk failed!\n");
            x_exit(-1);
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
}
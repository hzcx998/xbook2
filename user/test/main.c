#include <sys/xbook.h>
#include <sys/shm.h>
#include <string.h>
#include <conio.h>

int func(int n)
{   
    if (n == 0 || n == 1)
        return 1; 
    else 
        return n * func(n - 1); 
}

int __strlen(char *s)
{
    int n = 0;
    while (*s) {
        n++;
        s++;
    }
    return n;
}

x_dev_t dev;
void log(char *str)
{
    x_write(dev, 0, str, strlen(str));
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

        int shmid = x_shmget("shm_test", 4096, SHM_CREAT);
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
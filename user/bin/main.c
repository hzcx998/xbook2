#include <sys/xbook.h>
#include <sys/ioctl.h>
#include <sys/shm.h>
#include <conio.h>
#include <string.h>

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
    x_write(dev, 0, str, __strlen(str));
}

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

    int shmid = x_shmget("shm_test", 0, SHM_CREAT);
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
    
    /*
    shmid = x_shmget("shm_test", 0, SHM_CREAT | SHM_EXCL);
    if (shmid < 0) {
        printf("bin: child: get new shm failed!");
        return -1;
    }*/

    
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
    
    int pid = x_fork();
    if (pid > 0) {
        log("bin-parent!\n");
        //x_close(dev);
        x_exit(12345);
    } else {
        log("bin-child!\n");
        x_close(dev);
        x_exit(0x12345);
    }
    return 0;
}

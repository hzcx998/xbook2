#include <sys/xbook.h>
#include <sys/ioctl.h>
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
    /*
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
    */
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

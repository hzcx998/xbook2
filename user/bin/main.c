#include <xbook.h>
#include <ioctl.h>

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

    log("hello, exec!");

    func(1);

    x_ioctl(dev, CONIO_CLEAN, 0);

    x_putc(dev, '!');
    x_putc(dev, '@');
    
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
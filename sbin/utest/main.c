#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

void t()
{
    char b;
    int c[30];
    while (1) {
        
    }
}

int main()
{
    #if 0
    int a = 0;
    char buf[32] = {0};
    int c = a + 10;
    t();
    while (1) {
        
    }
    #endif
    #if 1
    /* 打开tty，用来进行基础地输入输出 */
    int tty0 = open("/dev/tty0", O_RDONLY);
    if (tty0 < 0) {
        while (1)
        {
            /* code */
        }
        
        return -1;
    }
    int tty1 = open("/dev/tty0", O_WRONLY);
    if (tty1 < 0) {
        while (1)
        {
            /* code */
        }
        close(tty0);
        return -1;
    }
    int tty2 = dup(tty1);
    //printf("[INIT]: start.\n");
    char *str = "[INIT]: start.\n";
    write(tty1, str, strlen(str));
    
    #endif
    while (1)
    {
        /* code */
    }
    
    return 0;
}

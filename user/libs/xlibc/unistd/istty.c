#include <unistd.h>
#include <sys/ioctl.h>

int isatty(int desc)
{
    if (desc < 0)
        return 0;
    int arg = 0;
    if (ioctl(desc, TIOCISTTY, &arg) < 0)
        return 0;
    return arg; 
}

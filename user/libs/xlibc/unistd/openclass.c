#include <unistd.h>

int openclass(const char *cname, int flags)
{
    char devname[32] = {0};
    if (probedev(cname, devname, 32) < 0)
        return -1; 
    int fd;
    fd = opendev(devname, flags);
    return fd;
}

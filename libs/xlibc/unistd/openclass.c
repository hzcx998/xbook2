#include <unistd.h>
#include <string.h>

int openclass(const char *cname, int flags)
{
    char _devname[32] = {0};
    char devname[32] = {0};
    if (probedev(cname, _devname, 32) < 0)
        return -1; 
    strcat(devname, "/dev/");
    strcat(devname, _devname);
    int fd = open(devname, flags);
    return fd;
}

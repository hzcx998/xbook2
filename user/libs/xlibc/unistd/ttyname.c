#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/udev.h>
#include <string.h>

char *ttyname(int desc)
{
    if (desc < 0)
        return NULL;
    static char name[DEVICE_NAME_LEN];
    memset(name, 0, DEVICE_NAME_LEN);
    if (ioctl(desc, TIOCNAME, name) < 0)
        return NULL;
    return name;
}

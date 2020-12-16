#include <sys/udev.h>
#include <sys/syscall.h>

int scandev(devent_t *de, device_type_t type, devent_t *out_de)
{
    return syscall3(int, SYS_SCANDEV, de, type, out_de);
}

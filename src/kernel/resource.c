#include <sys/resource.h>
#include <errno.h>

int sys_getpriority(int which, id_t who)
{
    return -ENOSYS;
}

int sys_setpriority(int which, id_t who, int prio)
{
    return -ENOSYS;    
}

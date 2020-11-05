
#include <xbook/resource.h>
#include <xbook/schedule.h>
#include <xbook/clock.h>

unsigned long sys_unid(int id)
{
    unsigned long _id;
    /* id(0-7) pid(8-15) systicks(16-31) */
    _id = (id & 0xff) + ((current_task->pid & 0xff) << 8) + ((systicks & 0xffff) << 16);
    return _id;
}

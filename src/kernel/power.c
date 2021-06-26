#include <xbook/power.h>
#include <errno.h>

int sys_reboot(int magic, int magic2, int cmd)
{
    if (magic != XBOOK_REBOOT_MAGIC1 && magic2 != XBOOK_REBOOT_MAGIC2)
        return -EINVAL;
    switch (cmd) {
    case XBOOK_REBOOT_CMD_RESTART:
        // TODO: 执行内存资源的释放以及物理设备的关闭
        do_reboot();
        break;
    case XBOOK_REBOOT_CMD_HALT:
        do_halt();
        break;
    case XBOOK_REBOOT_CMD_CAD_ON:
        return -ENOSYS;
    case XBOOK_REBOOT_CMD_CAD_OFF:
        return -ENOSYS;
    case XBOOK_REBOOT_CMD_POWER_OFF:
        // TODO: 执行内存资源的释放以及物理设备的关闭
        do_poweroff();
        break;
    case XBOOK_REBOOT_CMD_RESTART2:
        return -ENOSYS;
    default:
        break;
    }
    return -EINVAL;
}
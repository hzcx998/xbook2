#include <xbook/fs.h>
#include <xbook/diskman.h>
#include <xbook/debug.h>
#include <fsal/fsal.h>
#include <fsal/fstype.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>

int file_system_init()
{
    printk(KERN_INFO "fs: init start.\n");
    if (disk_manager_init() < 0)
        panic("fs: init disk manager failed!\n");

    if (fstype_init() < 0)
        panic("fs: init fstype failed, service stopped!\n");
    
    if (fsal_init() < 0) {
        panic("fs: init fsal failed, service stopped!\n");
    }
    printk(KERN_INFO "fs: init done.\n");
    
    int fd = sys_open("/tmp.txt", O_RDWR);
    if (fd < 0) {
        panic("fs: open file failed!\n");
    }
    
    printk(KERN_DEBUG "dup %d\n", sys_dup(fd));
    printk(KERN_DEBUG "dup %d\n", sys_dup(fd));

    int retval = sys_close(fd);
    printk(KERN_DEBUG "retval %d\n", retval);
    retval = sys_close(1);
    printk(KERN_DEBUG "retval %d\n", retval);
    retval = sys_close(2);
    printk(KERN_DEBUG "retval %d\n", retval);

    fd = sys_opendev("/com0", O_DEVEX | O_RDWR);
    if (fd < 0) {
        panic("fs: open device failed!\n");
    }
    printk(KERN_DEBUG "dup %d->%d\n", fd, sys_dup(fd));
    printk(KERN_DEBUG "dup %d->%d\n", fd, sys_dup(fd));

    retval = sys_close(fd);
    printk(KERN_DEBUG "retval %d\n", retval);
    retval = sys_close(1);
    printk(KERN_DEBUG "retval %d\n", retval);
    retval = sys_close(2);
    printk(KERN_DEBUG "retval %d\n", retval);

    spin("test");
    return 0;
}

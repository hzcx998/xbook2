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
    
    int fd = sys_open("/tmp2.txt", O_CREAT | O_RDWR);
    if (fd < 0) {
        panic("fs: open file failed!\n");
    }
    
    printk(KERN_DEBUG "dup %d\n", sys_dup(fd));
    printk(KERN_DEBUG "dup %d\n", sys_dup(fd));

    printk(KERN_DEBUG "write %d\n", sys_write(0, "fd0", 3));
    sys_fsync(0);
    printk(KERN_DEBUG "write %d\n", sys_write(1, "fd1", 3));
    sys_fsync(1);
    printk(KERN_DEBUG "write %d\n", sys_write(2, "fd2", 3));    
    sys_fsync(2);
    
    sys_lseek(0, 0, SEEK_SET);
    char buf[12] = {0};
    printk(KERN_DEBUG "read %d\n", sys_read(0, buf, 12));
    printk(KERN_DEBUG "buf %s\n", buf);

    int retval = sys_close(fd);
    printk(KERN_DEBUG "retval %d\n", retval);
    retval = sys_close(1);
    printk(KERN_DEBUG "retval %d\n", retval);
    retval = sys_close(2);
    printk(KERN_DEBUG "retval %d\n", retval);

    fd = sys_opendev("/com0", O_RDWR);
    if (fd < 0) {
        panic("fs: open device failed!\n");
    }
    printk(KERN_DEBUG "dup %d->%d\n", fd, sys_dup(fd));
    printk(KERN_DEBUG "dup %d->%d\n", fd, sys_dup(fd));
    printk(KERN_DEBUG "write %d\n", sys_write(0, "fd0", 3));
    printk(KERN_DEBUG "write %d\n", sys_write(1, "fd1", 3));
    printk(KERN_DEBUG "write %d\n", sys_write(2, "fd2", 3));    
    retval = sys_close(fd);
    printk(KERN_DEBUG "retval %d\n", retval);
    retval = sys_close(1);
    printk(KERN_DEBUG "retval %d\n", retval);
    retval = sys_close(2);
    printk(KERN_DEBUG "retval %d\n", retval);

    spin("test");
    return 0;
}

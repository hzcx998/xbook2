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
    return 0;
}

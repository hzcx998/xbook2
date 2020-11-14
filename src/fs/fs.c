#include <xbook/fs.h>
#include <xbook/diskman.h>
#include <xbook/debug.h>
#include <fsal/fsal.h>
#include <fsal/fstype.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>

int init_fs()
{
    printk("[fs]: init start...\n");
    if (init_disk_driver() < 0)
        panic("[fs]: init disk driver failed!\n");

    /* 初始化接口部分 */
    if (init_fstype() < 0) {
        panic("init fstype failed, service stopped!\n");
    }
    
    /* 初始化文件系统抽象层 */
    if (init_fsal() < 0) {
        panic("init fsal failed, service stopped!\n");
    }
    printk("[fs]: init done.\n");
    return 0;
}

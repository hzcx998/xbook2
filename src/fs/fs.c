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
    
    // open sbin/init
#if 0
    int fd = sys_open("/sbin/init", O_RDONLY);
    if (fd < 0) {
        printk("open fd failed!\n");
        return -1;
    }
    uint32_t *buf = kmalloc(1024 * 50);
    size_t need = 32 * KB;
    sys_lseek(fd, 0, SEEK_SET);
    int rd = sys_read(fd, buf, need);
    printk("need: %d, read: %d\n", need, rd);
    #if 0
    int i;
    for (i = 0; i <  512*15 / 4; i++) {
        printk(" %8x ", buf[i]);
    }
    #endif
    spin("test");
#endif
#if 0
    /* test */
    int fd = sys_open("/root/kfs", O_CREAT | O_RDWR);
    if (fd < 0)
        return -1;
    
    int fd1 = sys_open("/root/kfs", O_CREAT | O_RDWR);
    if (fd1 < 0)
        return -1;
    printk("fd %d fd1 %d.\n", fd, fd1);

    int wr = sys_write(fd, "hello", 5);
    printk("write file:%d %d bytes.\n", fd, wr);
    sys_close(fd);
    fd = sys_open("/root/kfs", O_CREAT | O_RDWR);
    if (fd < 0)
        return -1;
    char buf[10];
    int rd = sys_read(fd, buf, 10);
    printk("read file:%d %d bytes.\n", fd, rd);
    sys_close(fd);
    printk("data:%s\n", buf);




    dir_t dir = sys_opendir("/");
    if (dir < 0) {
        printk("open dir failed!\n");
    }

    dirent_t dirent;
    do {
        if (sys_readdir(dir, &dirent) < 0)
            break;
        
        printk("dir: %s\n", dirent.d_name);
    } while (1);
    sys_closedir(dir);
#endif
    return 0;
}

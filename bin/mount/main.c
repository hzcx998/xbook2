#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/mount.h>
#include <sys/ioctl.h>

#define LOOPBACK_NAME   "loopback"

int loop_device_setup(char *dev_path, char *file_path);
int loop_device_setdown(char *dev_path);

/**
 * mount /dev/hda /mnt/c
 * mount -t vfat /dev/hda /mnt/c
 * mount -o loop /hdd.img /mnt/d
 */
static void print_usage()
{
    printf("Usage: mount [-t fstype] [-o options] device dir\n");
    printf("Options:\n");
    printf("  -t        file system type.\n");
    printf("            fat12: DOS fat12.\n");
    printf("            fat16: DOS fat16.\n");
    printf("            fat32: Windows 9x fat32.\n");
    printf("  -o        options.\n");
    printf("            loop: Use loop block device to link a regular file.\n");
}

int main(int argc, char *argv[]) 
{
    if (argc < 2) {
        print_usage();
        return -1;
    }
    int result;
    char *arg_device = NULL;
    char *arg_dir = NULL;
    char *arg_fs = NULL;
    char *arg_option = NULL;
    int mount_flags = 0;
    int info_visiable = 0;
    opterr = 0;  //使getopt不行stderr输出错误信息

    while( (result = getopt(argc, argv, "hvt:o:")) != -1 ) {
        switch(result) {
        case 'h':
            print_usage();
            return 0;
        case 't':
            arg_fs = optarg;
            break;
        case 'v':
            info_visiable = 1;
            break;
        case 'o':
            arg_option = optarg;
            break;
        case '?':
            if (optopt == 't') {    // no arg
                fprintf(stderr, "mount: no file system type!\n", optopt);
            } else if (optopt == 'o') {    // no arg
                fprintf(stderr, "mount: no options!\n", optopt);
            } else {
                fprintf(stderr, "mount: unknown option '%c'!\n", optopt);
            }
            return -1;
        default:
            fprintf(stderr, "mount: option error!\n");
            return -1;
        }
    }
    // 处理后续固定参数
    if (argv[optind]) {
        arg_device = argv[optind];
        optind++;
        if (argv[optind]) {
            arg_dir = argv[optind];
        }
    }
    // printf("mount: fs=%s, option=%s, device=%s, dir=%s\n", arg_fs, arg_option, arg_device, arg_dir);
    if (arg_device == NULL || arg_dir == NULL) {
        fprintf(stderr, "mount: device or dir error!\n");
        return -1;    
    }
    if (!arg_fs) {  /* 没有指定文件系统，就自动选择 */
        arg_fs = "auto";
    }
    char loop_dev_path[32] = {0};
    if (arg_option) {
        if (!strcmp(arg_option, "loop")) {  /* -o loop */
            /* get a free loop device */
            char devname[32] = {0};
            if (probedev("loop", devname, 32) < 0) {
                fprintf(stderr, "mount: no free loop device for %s!\n", arg_device);
                return -1;
            }
            strcat(loop_dev_path, "/dev/");
            strcat(loop_dev_path, devname);
        } else if (!strncmp(arg_option, LOOPBACK_NAME, strlen(LOOPBACK_NAME))) {    /* -o loopback=xxx */
            /* 查看是否有指定 */
            char *config_device = arg_option + strlen(LOOPBACK_NAME);
            if (*config_device == '=') {    /* order device */
                config_device++;
                strcat(loop_dev_path, config_device);
            } else {
                fprintf(stderr, "mount: invalid option arg %s!\n", arg_option);
            }
        } else {
            fprintf(stderr, "mount: option arg %s not support!\n", arg_device);
            return -1;
        }
        
        /* 将device镜像文件和一个环回设备关联 */
        char *file_path = arg_device;   /* 设备参数就是文件参数 */
        
        /* 检测文件是否存在 */
        if (access(file_path, F_OK) < 0) {
            fprintf(stderr, "mount: file %s not exist or no permission to access!\n", file_path);
            return -1;
        }

        if (loop_device_setup(loop_dev_path, file_path) < 0) {
            return -1;
        }
        if (info_visiable)
            printf("mount: set device %s with file %s success.\n", loop_dev_path, file_path);
        arg_device = loop_dev_path;  /* 指向环回设备，挂载环回设备 */
    }

    if (mount(arg_device, arg_dir, arg_fs, mount_flags) < 0) {
        fprintf(stderr, "mount: device=%s, dir=%s, file system=%s failed!\n", 
            arg_device, arg_dir, arg_fs);
        loop_device_setdown(loop_dev_path);
        return -1;
    }
    if (info_visiable)
        printf("mount: mount device %s to dir %s success!\n", arg_device, arg_dir);
    return 0;
}

int loop_device_setup(char *dev_path, char *file_path)
{
    int fd = open(dev_path, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "device %s not exist or no permission to access.\n", dev_path);
        return -1;
    }
    if (ioctl(fd, DISKIO_SETUP, file_path) < 0) {
        fprintf(stderr, "set device %s with file %s failed!\n", dev_path, file_path);
        close(fd);
        return -1;
    }
    close(fd);  
    return 0;  
}

int loop_device_setdown(char *dev_path)
{
    int fd = open(dev_path, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "device %s not exist or no permission to access.\n", dev_path);
        return -1;
    }
    ioctl(fd, DISKIO_SETDOWN, NULL);
    close(fd);
    return 0;
}

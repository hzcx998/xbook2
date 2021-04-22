#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/ioctl.h>

/**
 * losetup /dev/loop1 /tmp/image.img
 * losetup -d /dev/loop1
 */
static void print_usage()
{
    printf("Usage: losetup [-d] [device] [file]\n");
    printf("Options:\n");
    printf("  -d        delect device link with file\n");
}

int main(int argc, char *argv[]) 
{
    if (argc < 2) {
        print_usage();
        return -1;
    }
    int result;
    char *arg_device = NULL;
    char *arg_file = NULL;
    int is_delete_device = 0;
    
    opterr = 0;  //使getopt不行stderr输出错误信息

    while( (result = getopt(argc, argv, "hd")) != -1 ) {
        switch(result) {
        case 'h':
            print_usage();
            return 0;
        case 'd':
            is_delete_device = 1;
            break;
        case '?':
            fprintf(stderr, "losetup: unknown option '%c'!\n", optopt);
            return -1;
        default:
            fprintf(stderr, "losetup: option error!\n");
            return -1;
        }
    }
    // 处理后续固定参数
    if (argv[optind]) { /* device */
        arg_device = argv[optind];
        optind++;
        if (argv[optind]) { /* file */
            arg_file = argv[optind];
        }
    }
    if (arg_device == NULL) {
        fprintf(stderr, "losetup: no device!\n");
        return -1;
    }
    /* open device then setup/setdown it */
    int fd = open(arg_device, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "losetup: device %s not exist or no permission to access.\n", arg_device);
        return -1;
    }
    
    if (!is_delete_device) {         // do set
        if (arg_file == NULL) {
            fprintf(stderr, "losetup: no file!\n");
            close(fd);
            return -1;
        }
        /* 检测文件是否存在 */
        if (access(arg_file, F_OK) < 0) {
            fprintf(stderr, "losetup: file %s not exist or no permission to access!\n", arg_file);
            close(fd);
            return -1;
        }

        if (ioctl(fd, DISKIO_SETUP, arg_file) < 0) {
            fprintf(stderr, "losetup: set device %s with file %s failed!\n", arg_device, arg_file);
            close(fd);
            return -1;
        }
        printf("losetup: set device %s with file %s success.\n", arg_device, arg_file);
    } else {        // do delete
        if (ioctl(fd, DISKIO_SETDOWN, NULL) < 0) {
            fprintf(stderr, "losetup: delete device failed!\n", arg_device);
            close(fd);
            return -1;
        }
        printf("losetup: delete device %s success.\n", arg_device);
    }
    close(fd);
    return 0;
}
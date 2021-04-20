#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/mount.h>

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
    opterr = 0;  //使getopt不行stderr输出错误信息

    while( (result = getopt(argc, argv, "ht:o:")) != -1 ) {
        switch(result) {
        case 'h':
            print_usage();
            return 0;
        case 't':
            arg_fs = optarg;
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
    if (arg_option) {
        /* TODO: add loop device support! */
        if (!strcmp(arg_option, "loop")) {
            // use loop device
        }
    }

    if (mount(arg_device, arg_dir, arg_fs, mount_flags) < 0) {
        fprintf(stderr, "mount: device=%s, dir=%s, file system=%s failed!\n", 
            arg_device, arg_dir, arg_fs);
        return -1;
    }
    printf("mount: mount device %s to dir %s success!\n", arg_device, arg_dir);
    return 0;
}

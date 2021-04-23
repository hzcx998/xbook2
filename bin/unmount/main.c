#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

/**
 * unmount -t fat32 /dev/hda
 * unmount /dev/hda
 * unmount /mnt
 */
static void print_usage()
{
    printf("Usage: unmount [-t fstype] device|dir \n");
    printf("Options:\n");
    printf("  -t        file system type.\n");
    printf("            fat12: DOS fat12.\n");
    printf("            fat16: DOS fat16.\n");
    printf("            fat32: Windows 9x fat32.\n");
}

int main(int argc, char *argv[]) 
{
    if (argc < 2) {
        print_usage();
        return -1;
    }
    int result;
    char *arg_pathname = NULL;
    char *arg_fs = NULL;
    int unmount_flags = 0;
    int info_visiable = 0;
    opterr = 0;  //使getopt不行stderr输出错误信息

    while( (result = getopt(argc, argv, "hvt:")) != -1 ) {
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
        case '?':
            if (optopt == 't') {    // no arg
                fprintf(stderr, "unmount: no file system type!\n", optopt);
            } else {
                fprintf(stderr, "unmount: unknown option '%c'!\n", optopt);
            }
            return -1;
        default:
            fprintf(stderr, "unmount: option error!\n");
            return -1;
        }
    }
    // 处理后续固定参数
    if (argv[optind]) {
        arg_pathname = argv[optind];
    }
    // printf("unmount: fs=%s, device=%s\n", arg_fs, arg_device);
    if (arg_pathname == NULL) {
        fprintf(stderr, "unmount: no path name!\n");
        return -1;
    }
    if (!arg_fs) {  /* 没有指定文件系统，就自动选择 */
        arg_fs = "auto";
    }
    /* TODO: 检测文件系统类型是否匹配，不匹配就返回 */
    struct stat stat_buf;
    if (stat(arg_pathname, &stat_buf) < 0) {
        fprintf(stderr, "unmount: get pathname %s state info failed!\n", 
            arg_pathname);
        return -1;
    }
    if (!(stat_buf.st_mode & (S_IFBLK | S_IFDIR))) {
        fprintf(stderr, "unmount: pathname %s must be a dir or block device!\n", 
            arg_pathname);
        return -1;
    }
    if (unmount(arg_pathname, unmount_flags) < 0) {
        fprintf(stderr, "unmount: pathname %s failed!\n", 
            arg_pathname);
        return -1;
    }
    if (info_visiable)
        printf("unmount: unmount file system on path %s success!\n", arg_pathname);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/mount.h>

/**
 * mkfs -t fat32 /dev/hda
 * mkfs /dev/hda
 */
static void print_usage()
{
    printf("Usage: mkfs [-t fstype] device\n");
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
    char *arg_device = NULL;
    char *arg_fs = NULL;
    int mkfs_flags = 0;
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
                fprintf(stderr, "mkfs: no file system type!\n", optopt);
            } else {
                fprintf(stderr, "mkfs: unknown option '%c'!\n", optopt);
            }
            return -1;
        default:
            fprintf(stderr, "mkfs: option error!\n");
            return -1;
        }
    }
    // 处理后续固定参数
    if (argv[optind]) {
        arg_device = argv[optind];
    }
    //printf("mkfs: fs=%s, device=%s.\n", arg_fs, arg_device);
    if (arg_device == NULL) {
        fprintf(stderr, "mkfs: device error!\n");
        return -1;
    }
    if (!arg_fs) {  /* 没有指定文件系统，就自动选择 */
        arg_fs = "auto";
    }
    if (mkfs(arg_device, arg_fs, mkfs_flags) < 0) {
        fprintf(stderr, "mkfs: device=%s file system=%s failed!\n", 
            arg_device, arg_fs);
        return -1;
    }
    if (info_visiable)
        printf("mkfs: make file system %s on disk %s success!\n", arg_fs, arg_device);
    return 0;
}
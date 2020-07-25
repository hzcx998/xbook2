#include <sys/res.h>
#include <sys/vmm.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/trigger.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/kfile.h>
#include <sys/ipc.h>
#include <sys/proc.h>
#include <ff.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <fsal/fsal.h>
#include <debug.h>

#include <core/filesrv.h>
#include <core/if.h>
#include <core/fstype.h>

/**
 * init_srvcore - 初始化服务核心
 * 
 */
int init_srvcore()
{
    /* 初始化接口部分 */
    if (init_fstype() < 0) {
        srvprint("init fstype failed, service stopped!\n");
        return -1;
    }

    /* 初始化接口部分 */
    if (init_srv_interface() < 0) {
        srvprint("init srv if failed, service stopped!\n");
        return -1;
    }
    return 0;
}

/* 文件映射表 */
struct file_map file_map_table[] = {
    {PATH_GUISRV, 1, NULL},
    {PATH_NETSRV, 1, NULL},
    {"/bin/tests", 0, NULL},
};

/**
 * init_child_proc - 创建一些子进程
 * 
 */
int init_child_proc()
{
    struct file_map *fmap;
    int pid = 0;
    int i;
    for (i = 0; i < ARRAY_SIZE(file_map_table); i++) {
        fmap = &file_map_table[i];
        if (fmap->execute) {
            pid = fork();
            if (pid < 0) {
                srvprint("fork failed!\n");
                return -1;
            }
            if (!pid) { /* 子进程执行新进程 */
                if (execv(fmap->path, fmap->argv)) {
                    srvprint("execv failed!\n");
                    exit(-1);
                }
            }
        }
    }
    return 0;
}

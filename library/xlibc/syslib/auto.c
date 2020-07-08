#include <sys/dir.h>

/**
 * _enter_preload - 进入预先加载
 * 
 * 当某些内容在使用前需要初始化时，但是，使用过程中并未进行
 * 初始化（不需要初始化），就需要在此进行初始化。
 * 简单的说，用户不知道要初始化，但是其实需要初始化。
 */
void _enter_preload(int argc, char *argv[])
{
    /* 默认使用根目录 */
    __setcwd(ROOT_DIR_BUF);
}

/**
 * _exit_cleanup - 退出清理机制
 * 
 * 进程退出时，需要自动处理清理一些资源，写在这里面。
 */
void _exit_cleanup()
{


}

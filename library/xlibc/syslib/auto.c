#include <sys/dir.h>
#include <malloc.h>

/* 环境变量指针，全局 */
char **_environ;

/* 退出之前需要执行的回调函数 */
extern void __atexit_callback();

/**
 * _enter_preload - 进入预先加载
 * 
 * 当某些内容在使用前需要初始化时，但是，使用过程中并未进行
 * 初始化（不需要初始化），就需要在此进行初始化。
 * 简单的说，用户不知道要初始化，但是其实需要初始化。
 */
void _enter_preload(int argc, char *const argv[], char *const envp[])
{
    /* 设置environ全局变量 */
    _environ = (char **)envp;
    /* 设置C语言的环境变量 */
    
}

/**
 * _exit_cleanup - 退出清理机制
 * 
 * 进程退出时，需要自动处理清理一些资源，写在这里面。
 */
void _exit_cleanup()
{
    __atexit_callback();

}

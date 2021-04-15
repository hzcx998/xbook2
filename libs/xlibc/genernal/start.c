#include <stdlib.h>

extern void _enter_preload(int argc, char *const argv[], char *const envp[]);

void __libc_start_main(
    int (*main) (int, char **, char **),
    int argc,
    char **argv,
    char **envp,
    void (*init) (void),
    void (*fini) (void),
    void *stack_end)
{
    /* 初始化和参数相关的内容 */
    _enter_preload(argc, argv, envp);
    init();
    int retval = main(argc, argv, envp);
    fini();
    exit(retval);
}
#include "test.h"

typedef int (*main_t) (int , char **);
typedef struct {
    char *name;
    main_t func;
} testfunc_t;

testfunc_t test_table[] = {
    {"fstat", fstat_test},
    {"exit", exit_test},
    {"execve", execve_test},
    {"dup2", dup2_test},
    {"dup", dup_test},
    {"close", close_test},
    {"fork", fork_test},
    {"clone", clone_test},
    {"chdir", chdir_test},
    {"brk", brk_test},
    {"openat", openat_test},
    {"sockcall", test_sockcall},
    {"port_comm2", port_comm_test2},
    {"port_comm3", port_comm_test3},
    {"pipe", pipe_test},
    {"shm", shm_test},
    {"xlibc", xlibc_test},
    {"math", math_test},
    {"pyt", pty_test},
    {"sleep", sleep_test},
    {"exp", exp_test},
    {"fifo", fifo_test},
    {"sys", sys_test},
    //{"pthread", pthread_test},
    {"file", file_test},
    {"file2", file_test2},
    {"perm", perm_test},
    /*{"socket", socket_test},
    {"socket2", socket_test2},
    {"socket3", socket_test3},
    {"socket4", socket_test4},
    {"backtrace", backtrace_test},
    {"backtrace2", backtrace_test2},
    */
    {"video", video_test},
    {"signal", signal_test},
    {"proc", proc_test},
    {"port_comm", port_comm_test},
    {"file", file_test3},
    {"fcntl", fcntl_test},
    {"tty", tty_test},
    {"id", id_test},
    {"pty2", pty_test2},
    {"loop", loop_test},
};


int main(int argc, char *argv[])
{
    printf("testing start...\n");
    
    int retval = -1;
    if (argc == 1) {
        retval = test_table[0].func(argc, argv);  
        printf("\ntest %s demo done.\n", test_table[0].name);
    } else if (argc == 2) {
        char *p = argv[1];
        int i;
        for (i = 0; i < ARRAY_SIZE(test_table); i++)
            if (!strcmp(p, test_table[i].name)) {
                retval = test_table[i].func(argc, argv);
                printf("\ntest %s demo done.\n", test_table[i].name);
                break;
            }
        if (i >= ARRAY_SIZE(test_table))
            printf("test %s demo not found!\n", p);
    }    
    return retval;
}

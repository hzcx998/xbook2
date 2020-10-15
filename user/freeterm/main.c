#include <stdio.h>
#include <stdlib.h>
#include <ft_console.h>
#include <ft_cmd.h>
#include <ft_window.h>
#include <pty.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <ft_pty.h>

static void exit_freeterm()
{
    ft_pty_exit(&ft_pty);
    exit_cmd_man();
    exit_console();
}

int main(int argc, char *argv[]) 
{
    if (init_console() < 0) {
        printf("freeterm: init console failed!\n");
        return -1;
    }
    
    if (init_cmd_man() < 0) {
        printf("freeterm: init cmd failed!\n");
        exit_console();
        return -1;
    }

    if (ft_pty_init(&ft_pty) < 0) {
        printf("freeterm: init pty failed!\n");
        exit_cmd_man();
        exit_console();
        return -1;
    }
    
    int pid = fork();
    if (pid < 0) {
        printf("freeterm: do fork failed!\n");
        return -1;
    } else if (!pid) { // 子进程
        ft_pty_launch(&ft_pty, "/bin/sh");
    } else { // 父进程
        ft_pty.pid_slaver = pid; // 记录子进程的进程pid
    }

    atexit(exit_freeterm);
    window_loop();
    return 0;
}

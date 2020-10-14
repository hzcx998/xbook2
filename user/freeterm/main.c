#include <stdio.h>
#include <stdlib.h>
#include <sh_console.h>
#include <sh_cmd.h>
#include <sh_window.h>
#include <pty.h>
#include <unistd.h>
#include <sys/ioctl.h>


static void exit_freeterm()
{
    exit_cmd_man();
    exit_console();
}
int fdm;
int main(int argc, char *argv[]) 
{
    if (init_console() < 0) {
        printf("bosh: init console failed!\n");
        return -1;
    }
    
    if (init_cmd_man() < 0) {
        printf("bosh: init cmd failed!\n");
        exit_console();
        return -1;
    }

    #if 0
    char *argv[3] = {"/bin/infones", "/res/nes/mario.nes", NULL};
    execute_cmd(2, argv);
    #endif
    
    fdm =  posix_openpt(O_RDWR);
    if (fdm < 0) {
        printf("open device failed!\n");
        return -1;
    }
    
    unlockpt(fdm);
    grantpt(fdm);
    int pyflags = PTTY_RDNOBLK;
    ioctl(fdm, TIOCSFLGS, &pyflags);

    int pid = fork();
    if (pid < 0) {
        printf("freeterm: do fork failed!\n");
        return -1;
    }
    if (!pid) { // 子进程
        char *sname = ptsname(fdm);
    
        if (sname == NULL) {
            printf("get slave tty name failed!\n");
            return -1;
        }
        printf("slave tty %s\n", sname);
        int fds = open(sname, O_DEVEX | O_RDWR, 0);
        if (fds < 0) {
            printf("open slave failed!\n");
            return -1;
        }
        // 重定向
        dup2(fds, 0);
        dup2(fds, 1);
        dup2(fds, 2);
        close(fds);
        close(fdm);
        printf("open slave tty ok!\n");
        char buf[12 + 1];
        int count = 0;
        while (count < 5)
        {
            memset(buf, 0, 12);
            read(0, buf, 12);
            printf("slaver read: %s\n", buf);
            
            count++;
        }
        printf("slave tty exit!\n");
        fflush(stdout);
        // close(0);
        exit(0);
    }
    atexit(exit_freeterm);

    window_loop();
    return 0;
}

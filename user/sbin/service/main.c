#include <stdio.h>
#include <unistd.h>
#include <sys/proc.h>

int main(int argc, char *argv[]) 
{
    if (argc < 2) {
        printf("start: please input service name.\n");
        return -1;
    }
    char *servname = argv[1];
    if (access(servname, F_OK) < 0) {
        printf("start service %s failed!\n", servname);
        return -1;
    }
    char *_argv[2] = {servname, NULL};
    pid_t pid = create_process(_argv, environ, PROC_CREATE_STOP);
    if (pid < 0) {
        printf("start service %s failed!\n", servname);
        return -1;
    }
    resume_process(pid);
    printf("start service %s success!\n", servname);
    return 0;
}

#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <sys/syscall.h>

pid_t tcgetpgrp( int filedes)
{
    pid_t pid = -1;
    if (ioctl ((filedes), TIOCGPGRP, &(pid)) < 0)
        return -1;
    return pid;
}

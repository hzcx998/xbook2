#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <termios.h>

int tcsetpgrp( int filedes, pid_t pgrpid )
{
    return ioctl ((filedes), TIOCSPGRP, &(pgrpid));
}

#include <unistd.h>

int setuid(uid_t uid)
{
    return 0;
}
uid_t getuid(void)
{
    return 0;
}
uid_t geteuid(void)
{
    return 0;
}

int setgid(gid_t gid)
{
    return 0;
}

gid_t getgid(void)
{
    return 0;
}

gid_t getegid(void)
{
    return 0;
}

pid_t getpgrp(void)
{
    return 0;
}

pid_t tcgetpgrp( int filedes )
{
    return 0;
}

int tcsetpgrp( int filedes, pid_t pgrpid )
{
    return 0;
}
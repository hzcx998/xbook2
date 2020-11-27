#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <pty.h>
#include <string.h>

#ifndef _HAS_OPENPT
int posix_openpt(int flags)
{
    char devname[32] = {0};
    if (probedev("ptm", devname, 32) < 0)
        return -1; 

    int fd_master;
    fd_master = opendev(devname, flags);
    return fd_master;
}
#endif

#ifndef _HAS_PTSNAME
char *ptsname(int fd)
{
    int i_slave;
    static char pts_name[16];

    if(ioctl(fd, TIOCGPTN, &i_slave) < 0)
    {
        return NULL;
    }

    pts_name[0] = 0;
    snprintf(pts_name, sizeof(pts_name), "pts%d", i_slave);
    return pts_name;
}
#endif

#ifndef _HAS_GRANTPT
int grantpt(int fd_master)
{
    char *pts_name;

    pts_name = ptsname(fd_master);
    pts_name++; // nothing
    return 0;
    // return (chmod(pts_name, S_IRUSR | S_IWUSR | S_IWGRP);
}
#endif

#ifndef _HAS_UNLOCKPT
int unlockpt(int fd_master)
{
    int i_lock = 0;
    return (ioctl(fd_master, TIOCSPTLCK, &i_lock));
}
#endif

int openpty(int *amaster, int *aslave, char *name,  
        struct termios *termp, struct winsize *winp)  
{
    const char *slave;  
    int mfd = -1, sfd = -1;  
    *amaster = *aslave = -1;  
    mfd = posix_openpt(O_RDWR | O_NOCTTY);  
    if (mfd < 0) 
        goto err;  
    if (grantpt(mfd) == -1 || unlockpt(mfd) == -1)  
        goto err;  
    if ((slave = ptsname(mfd)) == NULL)  
        goto err;  
    if ((sfd = opendev(slave, O_RDONLY | O_NOCTTY)) == -1)  
        goto err;  
    /*if (ioctl(sfd, I_PUSH, "ptem") == -1 ||  
        (termp != NULL && tcgetattr(sfd, termp) < 0))  
        goto err;  */
    if (amaster)  
        *amaster = mfd;  
    if (aslave)  
        *aslave = sfd;  
    if (winp)  
        ioctl(sfd, TIOCSWINSZ, winp);  
    return 0;  
err:  
    if (sfd != -1)  
        close(sfd);  
    close(mfd);  
    return -1;  
}

int ptym_open(char *pts_name, int namelen)
{
    char *name;
    int fdm;

    fdm = posix_openpt(O_RDWR | O_NOCTTY);
    if(fdm < 0)
    {
        return -1;
    }

    if(grantpt(fdm) < 0)
    {
        close(fdm);
        return -1;
    }

    if(unlockpt(fdm) < 0)
    {
        close(fdm);
        return -1;
    }

    if((name = ptsname(fdm)) == NULL)
    {
        close(fdm);
        return -1;
    }

    strncpy(pts_name, name, namelen);
    pts_name[namelen - 1] = 0;
    return fdm;
}

int ptys_open(const char *name)
{
    int fds;
    if((fds = opendev(name, O_RDWR))<0)
    {
        return -1;
    }
    return fds;
}



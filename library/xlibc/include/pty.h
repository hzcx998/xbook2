#ifndef _PTY_H
#define _PTY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <termios.h>

int posix_openpt(int flags);
char *ptsname(int fd);
int grantpt(int fd_master);
int unlockpt(int fd_master);
int openpty(int *amaster, int *aslave, char *name,  
        struct termios *termp, struct winsize *winp);
int ptym_open(char *pts_name, int namelen);
int ptys_open(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* _PTY_H */
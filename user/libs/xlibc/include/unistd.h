#ifndef _LIB_UNISTD_H
#define _LIB_UNISTD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <stddef.h>
#include <fcntl.h>

/* 文件模式 */
#define M_IREAD  (1 << 2)     //文件可读取权限
#define M_IWRITE (1 << 1)    //文件可写入权限
#define M_IEXEC  (1 << 0)     //文件可执行权限

#ifndef SEEK_SET
/* file seek */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

/* file acesss 文件检测 */
#define R_OK 4 /* Test for read permission. */
#define W_OK 2 /* Test for write permission. */
#define X_OK 1 /* Test for execute permission. */
#define F_OK 0 /* Test for existence. */

#define STDIN_FILENO    0  /* 标准输入文件号 */
#define STDOUT_FILENO   1  /* 标准输出文件号 */
#define STDERR_FILENO   2  /* 标准错误文件号 */

int brk(void *addr);
void *sbrk(int increment);

int open(const char *path, int flags, ...);
int close(int fd);
int read(int fd, void *buffer, size_t nbytes);
int write(int fd, const void *buffer, size_t nbytes);
int lseek(int fd, off_t offset, int whence);
int access(const char *filenpath, int mode);
int unlink(const char *path);
int ftruncate(int fd, off_t offset);
int fsync(int fd);
int ioctl(int fd, int cmd, void *arg);

int dup(int fd);
int dup2(int oldfd, int newfd);

long tell(int fd);

int mkdir(const char *path, mode_t mode);
int rmdir(const char *path);
int _rename(const char *source, const char *target);

int chdir(const char *path);
char *getcwd(char *buf, int bufsz);

int execve(const char *pathname, char *const argv[], char *const envp[]);
int execle(const char *pathname, char *const envp[], const char *arg, ...);
int execv(const char *pathname, char *const argv[]);
int execl(const char *pathname, const char *arg, ...);
int execvp(const char *filename, char *const argv[]);
int execlp(const char *filename, const char *arg, ...);
int usleep(useconds_t usec);

int isatty(int desc);
char *ttyname(int desc);

extern char **_environ;
#define environ _environ

int pipe(int fd[2]);

/* supported by xbook kernel */
int probedev(const char *name, char *buf, size_t buflen);
int openclass(const char *cname, int flags);
int opendev(const char *devname, int flags);
int openfifo(const char *fifoname, int flags);
int openclass(const char *cname, int flags);

#define mkfifo openfifo

/* id */
int setuid(uid_t uid);
uid_t getuid(void);
uid_t geteuid(void);

gid_t getgid(void);
gid_t getegid(void);

int setgid(gid_t gid);

/* process grpub */
pid_t getpgid( pid_t pid);
pid_t getpgrp(void);
int setpgrp(void);
int setpgid(pid_t pid,pid_t pgid);

pid_t tcgetpgrp( int filedes );
int tcsetpgrp( int filedes, pid_t pgrpid );

int gethostname(char *name, size_t len);

enum {
    _SC_ARG_MAX = 0,
    _SC_CHILD_MAX,
    _SC_HOST_NAME_MAX,
    _SC_LOGIN_NAME_MAX,
    _SC_NGROUPS_MAX,
    _SC_CLK_TCK,
    _SC_OPEN_MAX,
    _SC_PAGESIZE,
    _SC_PAGE_SIZE,
    _SC_RE_DUP_MAX,
    _SC_STREAM_MAX,
    _SC_SYMLOOP_MAX,
    _SC_TTY_NAME_MAX,
    _SC_TZNAME_MAX,
    _SC_VERSION,
};

long sysconf(int name);

#define getmaxgroups()  sysconf(_SC_NGROUPS_MAX)
#define getmaxchild()   sysconf(_SC_CHILD_MAX)
#define getdtablesize() sysconf(_SC_OPEN_MAX)


#include <sys/proc.h>
#include <getopt.h>

#ifdef __cplusplus
}
#endif

#endif  /* _LIB_UNISTD_H */

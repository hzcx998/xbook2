#ifndef __UNISTD_H__
#define __UNISTD_H__

#include "stddef.h"

extern int __clone(int (*func)(void *), void *stack, int flags, void *arg, ...);

int open(const char *, int);
int openat(int, const char*, int);

ssize_t read(int, void *, size_t);
ssize_t write(int, const void *, size_t);

int close(int);
pid_t getpid(void);
pid_t getppid(void);
int sched_yield(void);
void exit(int);
pid_t fork(void);
pid_t clone(int (*fn)(void *arg), void *arg, void *stack, size_t stack_size, unsigned long flags);
int exec(char *);
int execve(const char *, char *const [], char *const []);
int waitpid(int, int *, int);
int64 get_time();
int sys_get_time(TimeVal *ts, int tz); // syscall ID: 169; tz 表示时区，这里无需考虑，始终为0; 返回值：正确返回 0，错误返回 -1。
int sleep(unsigned long long);
int set_priority(int prio);
void *mmap(void *, size_t, int, int, int, off_t);
int munmap(void *start, size_t len);
int wait(int *);
int spawn(char *file);
int mailread(void *buf, int len);
int mailwrite(int pid, void *buf, int len);

int fstat(int fd, struct kstat *st);
int sys_linkat(int olddirfd, char *oldpath, int newdirfd, char *newpath, unsigned int flags);
int sys_unlinkat(int dirfd, char *path, unsigned int flags);
int link(char *old_path, char *new_path);
int unlink(char *path);
int time(unsigned long *tloc);
int brk(void *);

char *getcwd(char *, size_t);
int chdir(const char *);
int mkdir(const char *, mode_t);
int getdents(int fd, struct linux_dirent64 *dirp64, unsigned long len);
int pipe(int [2]);
int dup(int);
int dup2(int, int);

unsigned int alarm(unsigned int seconds);

uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
int setuid(uid_t uid);
int seteuid(uid_t euid);
int setgid(gid_t gid);
int setegid(gid_t egid);
int getgroups(int count, gid_t list[]);
int setgroups(size_t count, const gid_t list[]);

int setresuid(uid_t ruid, uid_t euid, uid_t suid);
int getresuid(uid_t *ruid, uid_t *euid, uid_t *suid);
int setresgid(uid_t rgid, uid_t egid, uid_t sgid);
int getresgid(uid_t *rgid, uid_t *egid, uid_t *sgid);

int setpgid(pid_t pid, pid_t pgid);
pid_t getpgid(pid_t pid);
pid_t getpgrp(void);
pid_t gettid(void);
pid_t setsid(void);
pid_t getsid(pid_t pid);

#endif // __UNISTD_H__

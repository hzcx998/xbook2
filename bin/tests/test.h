#ifndef _TEST_H
#define _TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
//#include <semaphore.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>
#include <sys/proc.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/dir.h>
#include <sys/types.h>

#define TEST_START(x) puts("========== START ");puts(x);puts(" ==========\n");
#define TEST_END(x) puts("========== END ");puts(x);puts(" ==========\n");

static inline void sys_err(char *str)
{
    printf("sys err: %s\n", str);
    exit(-1);
}

int yield_test(int argc, char* argv[]);
int waitpid_test(int argc, char* argv[]);
int wait_test(int argc, char* argv[]);
int unlink_test(int argc, char* argv[]);
int uname_test(int argc, char* argv[]);
int mount_test(int argc, char* argv[]);
int times_test(int argc, char *argv[]);
int sleep_test2(int argc, char *argv[]);
int pipe_test2(int argc, char *argv[]);
int munmap_test(int argc, char* argv[]);
int mmap_test(int argc, char* argv[]);
int mkdir_test(int argc, char* argv[]);
int gettimeofday_test(int argc, char* argv[]);
int getppid_test(int argc, char* argv[]);
int getpid_test(int argc, char* argv[]);
int getdents_test2(int argc, char* argv[]);
int getdents_test(int argc, char* argv[]);
int getcwd_test(int argc, char* argv[]);
int fstat_test(int argc, char* argv[]);
int exit_test(int argc, char* argv[]);
int execve_test(int argc, char* argv[]);
int dup2_test(int argc, char* argv[]);
int dup_test(int argc, char* argv[]);
int pipe_test(int argc, char *argv[]);
int shm_test(int argc, char *argv[]);
int xlibc_test(int argc,char *argv[]);
int math_test(int argc, char *argv[]);

int pty_test(int argc, char *argv[]);
int sleep_test(int argc, char *argv[]);
int exp_test(int argc, char *argv[]);
int fifo_test(int argc, char *argv[]);
int sys_test(int argc, char *argv[]);
int pthread_test(int argc, char *argv[]);

int file_test(int argc, char *argv[]);
int file_test2(int argc, char *argv[]);

int perm_test(int argc, char *argv[]);

int socket_test(int argc, char *argv[]);
int socket_test2(int argc, char *argv[]);
int socket_test3(int argc, char *argv[]);

int backtrace_test(int argc, char *argv[]);
int video_test(int argc, char *argv[]);
int signal_test(int argc, char *argv[]);
int proc_test(int argc, char *argv[]);

int port_comm_test(int argc, char *argv[]);
int gui_test(int argc, char *argv[]);
int file_test3(int argc, char *argv[]);
int fcntl_test(int argc, char *argv[]);
int tty_test(int argc, char *argv[]);

int id_test(int argc, char *argv[]);
int pty_test2(int argc, char *argv[]);
int backtrace_test2(int argc, char* argv[]);
int loop_test(int argc, char *argv[]);

int fifo_test2(int argc, char *argv[]);
int test_sockcall(int argc, char *argv[]);
int port_comm_test2(int argc, char *argv[]);
int port_comm_test3(int argc, char *argv[]);
int socket_test4(int argc, char *argv[]);
int openat_test(int argc, char *argv[]);
int brk_test(int argc, char* argv[]);
int chdir_test(int argc, char* argv[]);
int clone_test(int argc, char* argv[]);
int fork_test(int argc, char* argv[]);
int close_test(int argc, char* argv[]);

#endif // _TEST_H

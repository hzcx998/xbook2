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
#include <semaphore.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>
#include <sys/proc.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/dir.h>
#include <sys/types.h>

int pipe_test(int argc, char *argv[]);
int shm_test(int argc, char *argv[]);
int trig_test(int argc, char *argv[]);
int xlibc_test(int argc,char *argv[]);
int math_test(int argc, char *argv[]);

int cjson_main(void);
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
int socket_test4(int argc, char *argv[]);

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

static inline void sys_err(char *str)
{
    printf("sys err: %s\n", str);
    exit(-1);
}

#endif // _TEST_H

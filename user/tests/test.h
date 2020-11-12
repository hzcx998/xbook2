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
#include <sys/socket.h>

int socket_test(int argc, char *argv[]);
int socket_test2(int argc, char *argv[]);
int socket_test3(int argc, char *argv[]);
int socket_test4(int argc, char *argv[]);

int pipe_test(int argc, char *argv[]);
int shm_test(int argc, char *argv[]);
int trig_test(int argc, char *argv[]);
int xlibc_test(int argc,char *argv[]);
int math_test(int argc, char *argv[]);

int http_test(int argc, char **argv);
int buddy_test(int argc,char *argv[]);
int cjson_main(void);
int pty_test(int argc, char *argv[]);
int sleep_test(int argc, char *argv[]);
int exp_test(int argc, char *argv[]);
int fifo_test(int argc, char *argv[]);

#endif // _TEST_H

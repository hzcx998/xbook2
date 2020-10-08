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

/* ---- socket test ---- */
int socket_test(int argc, char *argv[]);
int socket_test2(int argc, char *argv[]);
int socket_test3(int argc, char *argv[]);
int socket_test4(int argc, char *argv[]);
/* ----socket test end ----*/

/* ---- pipe test ---- */
int pipe_test(int argc, char *argv[]);
/* ----socket test end ----*/

int shm_test(int argc, char *argv[]);

int trig_test(int argc, char *argv[]);

/* ---- xlibc test ---- */
int xlibc_test(int argc,char *argv[]);
/* ---- xlibc test end ---- */
int math_test(int argc, char *argv[]);

int http_test(int argc, char **argv);

int buddy_test(int argc,char *argv[]);

int cjson_main(void);

#endif // _TEST_H

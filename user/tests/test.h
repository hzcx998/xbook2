#ifndef _TEST_H
#define _TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
#include <assert.h>
#include <errno.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>
#include <srv/guisrv.h>
#include <sys/srvcall.h>
#include <sys/proc.h>
#include <sys/res.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <sys/socket.h>

/* ---- video test ---- */
int video_test(int argc, char *argv[]);
int test_png_main(int argc, char **argv);
int  screen_output_pixel(int x, int y, uint32_t  color);
int jpg_display(char * path);
/* ----video test end ----*/


/* ---- socket test ---- */
int socket_test(int argc, char *argv[]);
int socket_test2(int argc, char *argv[]);
int socket_test3(int argc, char *argv[]);
int socket_test4(int argc, char *argv[]);
/* ----socket test end ----*/

/* ---- pipe test ---- */
int pipe_test(int argc, char *argv[]);
/* ----socket test end ----*/

#endif // _TEST_H

#ifndef _TEST_H
#define _TEST_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <time.h>
#include <sys/syscall.h>

#define $(func) #func

int oscamp_signal(int argc, char *argv[]);
int test_sched(int argc, char *argv[]);
int test_time(int argc, char *argv[]);
int test_kill(int argc, char *argv[]);
int test_xattr(int argc, char *argv[]);
int test_reboot(int argc, char *argv[]);



#endif // _TEST_H


#ifndef _SERV_TESTSERV_H
#define _SERV_TESTSERV_H

#include <stdint.h>

#include "test.public.h"
#define SERV_NAME "test"

/* 服务调用的名字 */
#define SERVPORT_NAME   SERV_NAME"serv"   

#define SERVDEBUG   1
#if SERVDEBUG
#define print_fmt(fmt) fmt
#define SERVPRINT(fmt, ...) \
        printf("%s: " print_fmt(fmt), SERVPORT_NAME , ##__VA_ARGS__)
#else
    #define SERVPRINT(fmt, ...) 
#endif

#endif  /* _SERV_TESTSERV_H */


#ifndef _TESTSERV_SERVICE_H
#define _TESTSERV_SERVICE_H

#include <stdint.h>

// import other file
#include "inneral/test.config.h"

#undef SERV_NAME
#define SERV_NAME "test"

#define TESTSERV_NAME   SERV_NAME"serv"   

#define SERVDEBUG   1
#if SERVDEBUG
#define __print_fmt(fmt) fmt
#define SERVPRINT(fmt, ...) \
        printf("%s: " __print_fmt(fmt), TESTSERV_NAME , ##__VA_ARGS__)
#else
    #define SERVPRINT(fmt, ...) 
#endif

#endif  /* _TESTSERV_SERVICE_H */

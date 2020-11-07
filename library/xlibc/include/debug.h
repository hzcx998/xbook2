#ifndef _LIB_DEBUG_H
#define _LIB_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"

#define NPRINT_DEBUG

#define DEBUG_VOID_CAST  (void)

#ifndef NPRINT_DEBUG
#define dbgprint(fmt, ...)  \
    printf(fmt, __VA_ARGS__)
#else
#define dbgprint(...)  DEBUG_VOID_CAST(0)
#endif


#ifdef __cplusplus
}
#endif

#endif  /* _LIB_DEBUG_H */

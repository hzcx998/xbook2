
#ifndef _XLIBC_SPIN_H
#define _XLIBC_SPIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#define spin() \
        do { \
            printf("spin in %s : from Jason Hu.\n", __func__); \
            while (1); \
        } while(0)

#ifdef __cplusplus
}
#endif

#endif  /* _XLIBC_SPIN_H */
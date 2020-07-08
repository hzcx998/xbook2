
#ifndef _XLIBC_SPIN_H
#define _XLIBC_SPIN_H

#include <stdio.h>
#include <stdlib.h>

#define spin() \
        do { \
            printf("spin in %s : from Jason Hu.\n", __func__); \
            while (1); \
        } while(0)

#endif  /* _XLIBC_SPIN_H */
/*
    File:       sqrt.c

    Contains:   For sqrt(x)

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/

#include <math.h>

M_FLOAT sqrt(float x) {
    /* It's not important */
    if (fabs(x) < EPSILON)
        return (0.0);

    float x2 = x * 0.5F, y = x;
    long i = *(long*) & y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*) & i;
    y *= (1.5F - (x2 * y * y));
    y *= (1.5F - (x2 * y * y));

    return (1.0 / y);
}

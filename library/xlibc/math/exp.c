/*
    File:       exp.c

    Contains:   For e^x

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/

#include <math.h>

M_FLOAT exp(M_FLOAT x) {
    /* It's not important */
    if (fabs(x) < EPSILON)
        return (1.0);
    x = 1.0 + x / 16384; /* set exactness */
    x *= x; /* exactness = 2 */
    x *= x; /* exactness = 4 */
    x *= x; /* exactness = 8 */
    x *= x; /* exactness = 16 */
    x *= x; /* exactness = 32 */
    x *= x; /* exactness = 64 */
    x *= x; /* exactness = 128 */
    x *= x; /* exactness = 256 */
    x *= x; /* exactness = 512 */
    x *= x; /* exactness = 1024 */
    x *= x; /* exactness = 2048 */
    x *= x; /* exactness = 4096 */
    x *= x; /* exactness = 8192 */
    x *= x; /* exactness = 16384 */
    return x;
}

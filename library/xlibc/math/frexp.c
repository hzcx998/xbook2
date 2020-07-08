/*
    File:       frexp.c

    Contains:   Split a float into mantissa and exponent

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/


#include <math.h>

M_FLOAT frexp(double x, int *exptr) {
    union {
        double* _x;
        double_t* x;
    } ux;

    ux._x = &x;

    if (exptr != NULL)
        *exptr = ux.x->exponent - 0x3fe;

    ux.x->exponent = 0x3fe;

    return x;
}


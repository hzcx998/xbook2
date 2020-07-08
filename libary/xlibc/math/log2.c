/*
    File:       log2.c

    Contains:   For log(2)(x)

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/


#include <math.h>

M_FLOAT log2(float x) {
    union {
        float f;
        unsigned int i;
    } vx = { x };
    union {
        unsigned int i;
        float f;
    } mx = { (vx.i & 0x007FFFFF) | (0x7e << 23) };
    float y = vx.i;
    y *= 1.0 / (1 << 23);

    return (y - 124.22544637f - 1.498030302f * mx.f - 1.72587999f / (0.3520887068f + mx.f));
}

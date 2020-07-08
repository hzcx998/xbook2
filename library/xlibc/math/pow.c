/*
    File:       pow.c

    Contains:   For x^y

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/


#include <math.h>

M_FLOAT pow(double x, double y) {
    union {
        double d;
        int x[2];
    } u = { x };
    u.x[1] = (int)(y * (u.x[1] - 1072632447) + 1072632447);
    u.x[0] = 0;
    return u.d;
}

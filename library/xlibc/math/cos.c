/*
    File:       cos.c

    Contains:   For cos(x)

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/


#include <math.h>

M_FLOAT cos(M_FLOAT x) {
    return sin(x + MATH_PI_2);
}

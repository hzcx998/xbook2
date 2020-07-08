/*
    File:       tan.c

    Contains:   For tan(x)

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/

#include <math.h>

M_FLOAT tan(M_FLOAT x) {
    return (sin(x) / sin(x + MATH_PI_2));
}

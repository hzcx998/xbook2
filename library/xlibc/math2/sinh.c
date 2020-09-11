/*
    File:       sinh.c

    Contains:   For sinh(x)

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/

#include <math.h>

M_FLOAT sinh(M_FLOAT x) {
    double exponent = exp(x);
    return ((exponent - 1.0 / exponent) / 2.0);
}

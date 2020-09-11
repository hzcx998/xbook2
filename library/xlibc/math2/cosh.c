/*
    File:       cosh.c

    Contains:   For cosh(x)

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/


#include <math.h>

M_FLOAT cosh(M_FLOAT x) {
    double exponent = exp(x);
    return ((exponent + 1.0 / exponent) / 2.0);
}

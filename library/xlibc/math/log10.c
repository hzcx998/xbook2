/*
    File:       log10.c

    Contains:   For log(10)(x)

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/

#include <math.h>

M_FLOAT log10(M_FLOAT x) {
    return (LOG_MULTIPLE / MATH_LN10 * log2(x));
}

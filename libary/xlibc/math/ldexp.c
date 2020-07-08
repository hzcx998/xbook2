/*
    File:       ldexp.c

    Contains:   For x * 2^exp

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/


#include <math.h>

M_FLOAT ldexp(M_FLOAT x, int exp) {
    return (x * (double)pow(2.0, exp));
}

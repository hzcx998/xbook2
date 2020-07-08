/*
    File:       asin.c

    Contains:   For arcsin(x)

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/

#include <math.h>

M_FLOAT asin(M_FLOAT x) {
    return atan2(x, sqrt(1.0 - x * x));
}

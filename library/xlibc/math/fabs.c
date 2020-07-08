/*
    File:       fabs.c

    Contains:   For abs(x)

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/


#include <math.h>

M_FLOAT fabs(M_FLOAT x) {
    return (x < 0 ? -x : x);
}

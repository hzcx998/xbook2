/*
    File:       acos.c

    Contains:   For arccos(x)

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/

#include <math.h>

M_FLOAT acos(M_FLOAT x) {
    return atan2(sqrt(1.0 - x * x), x);
}

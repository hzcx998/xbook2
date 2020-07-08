/*
    File:       floor.c

    Contains:   For round down

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/


#include <math.h>

M_FLOAT floor(M_FLOAT x) {
    int intX = x;
    if (x < 0 && x < intX)
        return intX - 1.0;
    return intX;
}

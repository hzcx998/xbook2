/*
    File:       fmax.c

    Contains:   

    Written by: Jason Hu

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/

#include <math.h>

M_FLOAT fmax(M_FLOAT x, M_FLOAT y) {
    return x > y ? x : y;
}

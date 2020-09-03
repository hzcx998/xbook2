/*
    File:       fmin.c

    Contains:   

    Written by: Jason Hu

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/

#include <math.h>

M_FLOAT fmin(M_FLOAT x, M_FLOAT y) {
    return x < y ? x : y;
}

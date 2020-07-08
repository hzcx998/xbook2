/*
    File:       log.c

    Contains:   For log(e)(x)

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/

#include <math.h>

M_FLOAT log(M_FLOAT x) {
    return (LOG_MULTIPLE * log2(x));
}

/*
    File:       round.c

    Contains:   For rounding

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/

#include <math.h>

M_FLOAT round(M_FLOAT x) {
    if (x > 0)
        return (int)(x + 0.5);
    return (int)(x - 0.5);
}

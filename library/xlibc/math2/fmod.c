/*
    File:       fmod.c

    Contains:   For x % y in float

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/


#include <math.h>

M_FLOAT fmod(M_FLOAT x, M_FLOAT y) {
    return (x - (int)(x / y) * y);
}

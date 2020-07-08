/*
    File:       atan.c

    Contains:   For arctan(x)

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/


#include <math.h>

M_FLOAT atan(M_FLOAT x) {
  return (MATH_PI_4 * x - x * (fabs(x) - 1) * (0.2447 + 0.0663 * fabs(x)));
}

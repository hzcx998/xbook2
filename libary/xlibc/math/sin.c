/*
    File:       sin.c

    Contains:   For sin(x)

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/

#include <math.h>

M_FLOAT sin(M_FLOAT x) {
    if (fabs(x) < EPSILON)
        return (0.0);

    x = -1.0 * (fmod(x, M_PI_D) - MATH_PI);
    const M_FLOAT B = 4 / MATH_PI;
    const M_FLOAT C = -4 / (MATH_PI * MATH_PI);

    M_FLOAT y = B * x + C * x * (x > 0 ? x : -x);
    y = 0.115 * (y * (y > 0 ? y : -y) - y) + y;

    return (0.885 * y + 0.115 * y * (y > 0 ? y : -y));
}

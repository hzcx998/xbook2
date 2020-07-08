/*
    File:       modf.c

    Contains:   Decompose x to get the integer and fractional parts

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/

#include <math.h>

double modf(double x, double *intptr) {
    if (intptr == NULL)
        return *(double*)NULL;
    double fraction = x;
    *intptr = (double)(int)x;
    return (fraction - *intptr);
}

/*
    File:       math.c

    Contains:   For all math const data

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/


#include <math.h>

#ifdef __LIB_32B__
const M_FLOAT MATH_PI = M_PI;
const M_FLOAT MATH_PI_D = M_PI_D;
const M_FLOAT MATH_PI_2 = M_PI_2;
const M_FLOAT MATH_PI_4 = M_PI_4;
const M_FLOAT MATH_LN10 = M_LN10;
#else
const M_FLOAT MATH_PI = M_PIl;
const M_FLOAT MATH_PI_D = M_PI_Dl;
const M_FLOAT MATH_PI_2 = M_PI_2l;
const M_FLOAT MATH_PI_4 = M_PI_4l;
const M_FLOAT MATH_LN10 = M_LN10l;
#endif

const float EPSILON = 0.000001;
const M_FLOAT LOG_MULTIPLE = 0.69314718f;

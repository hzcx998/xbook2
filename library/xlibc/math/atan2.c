/*
    File:       atan2.c

    Contains:   For arctan2(x)

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/


#include <math.h>

#define DBL_EPSILON 2.2204460492503131e-016

#ifdef __LIB_32B__
#define PI M_PI
#else
#define PI M_PIl
#endif

static const float atan2_p1 = 0.9997878412794807f * (float)(180 / PI);
static const float atan2_p3 = -0.3258083974640975f * (float)(180 / PI);
static const float atan2_p5 = 0.1555786518463281f * (float)(180 / PI);
static const float atan2_p7 = -0.04432655554792128f * (float)(180 / PI);

M_FLOAT atan2(M_FLOAT x, M_FLOAT y) {
    float ax = fabs(x), ay = fabs(y);
    float a, c, c2;
    if(ax >= ay) {
        c = ay / (ax + (float)DBL_EPSILON);
        c2 = c*c;
        a = (((atan2_p7*c2 + atan2_p5) * c2 + atan2_p3) * c2 + atan2_p1) * c;
    } else {
        c = ax / (ay + (float)DBL_EPSILON);
        c2 = c*c;
        a = 90.f - (((atan2_p7 * c2 + atan2_p5) * c2 + atan2_p3) * c2 + atan2_p1) * c;
    }
    if( x < 0 )
        a = 180.f - a;
    if( y < 0 )
        a = 360.f - a;
    return a;
}

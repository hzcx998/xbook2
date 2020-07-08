#ifndef _XLIBC_MATH_H
#define _XLIBC_MATH_H

#include <arch/config.h>
#include <stddef.h>


/* max() & min() */
#define	MAX(a,b)	((a) > (b) ? (a) : (b))
#define	MIN(a,b)	((a) < (b) ? (a) : (b))

/* 除后上入 */
#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))

/* 除后下舍 */
#define DIV_ROUND_DOWN(X, STEP) ((X) / (STEP))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif
/*
    File:       math.h

    Contains:   Define math type, functions and macros

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/

/* IEEE ISO */
typedef struct {
    unsigned int mantissa:23;
    unsigned int exponent:8;
    unsigned int sign:1;
} float_t;

typedef struct {
    unsigned int mantissal:32;
    unsigned int mantissah:20;
    unsigned int exponent:11;
    unsigned int sign:1;
} double_t;

typedef struct {
    unsigned int mantissal:32;
    unsigned int mantissah:32;
    unsigned int exponent:15;
    unsigned int sign:1;
    unsigned int empty:16;
} long_double_t;
/* IEEE ISO */

#define M_E        2.7182818284590452354   /* e */
#define M_LOG2E    1.4426950408889634074   /* log_2 e */
#define M_LOG10E   0.43429448190325182765  /* log_10 e */
#define M_LN2      0.69314718055994530942  /* log_e 2 */
#define M_LN10     2.30258509299404568402  /* log_e 10 */
#define M_PI       3.14159265358979323846  /* pi */
#define M_PI_D     6.28318530717958646692  /* pi*2 */
#define M_PI_2     1.57079632679489661923  /* pi/2 */
#define M_PI_4     0.78539816339744830962  /* pi/4 */
#define M_1_PI     0.31830988618379067154  /* 1/pi */
#define M_2_PI     0.63661977236758134308  /* 2/pi */
#define M_2_SQRTPI 1.12837916709551257390  /* 2/sqrt(pi) */
#define M_SQRT2    1.41421356237309504880  /* sqrt(2) */
#define M_SQRT1_2  0.70710678118654752440  /* 1/sqrt(2) */

#ifdef __LIB_64B__
#define M_El        2.7182818284590452353602874713526625L  /* e */
#define M_LOG2El    1.4426950408889634073599246810018922L  /* log_2 e */
#define M_LOG10El   0.4342944819032518276511289189166051L  /* log_10 e */
#define M_LN2l      0.6931471805599453094172321214581766L  /* log_e 2 */
#define M_LN10l     2.3025850929940456840179914546843642L  /* log_e 10 */
#define M_PIl       3.1415926535897932384626433832795029L  /* pi */
#define M_PI_Dl     6.2831853071795864669252867665590058L  /* pi*2 */
#define M_PI_2l     1.5707963267948966192313216916397514L  /* pi/2 */
#define M_PI_4l     0.7853981633974483096156608458198757L  /* pi/4 */
#define M_1_PIl     0.3183098861837906715377675267450287L  /* 1/pi */
#define M_2_PIl     0.6366197723675813430755350534900574L  /* 2/pi */
#define M_2_SQRTPIl 1.1283791670955125738961589031215452L  /* 2/sqrt(pi) */
#define M_SQRT2l    1.4142135623730950488016887242096981L  /* sqrt(2) */
#define M_SQRT1_2l  0.7071067811865475244008443621048490L  /* 1/sqrt(2) */
#endif /* __LIB_64B__ */

#ifdef __LIB_32B__
#define M_FLOAT float
#else
#define M_FLOAT double
#endif /* __LIB_32B__ */

M_FLOAT acos(M_FLOAT);
M_FLOAT asin(M_FLOAT);
M_FLOAT atan(M_FLOAT);
M_FLOAT atan2(M_FLOAT, M_FLOAT);
M_FLOAT ceil(M_FLOAT);
M_FLOAT cos(M_FLOAT);
M_FLOAT cosh(M_FLOAT);
M_FLOAT exp(M_FLOAT);
M_FLOAT fabs(M_FLOAT);
M_FLOAT floor(M_FLOAT);
M_FLOAT fmod(M_FLOAT, M_FLOAT);
M_FLOAT frexp(double, int *);
M_FLOAT ldexp(M_FLOAT, int);
M_FLOAT log(M_FLOAT);
M_FLOAT log2(float);
M_FLOAT log10(M_FLOAT);
double modf(double , double *);
M_FLOAT pow(double, double);
M_FLOAT round(M_FLOAT);
M_FLOAT sin(M_FLOAT);
M_FLOAT sinh(M_FLOAT);
M_FLOAT sqrt(float);
M_FLOAT tan(M_FLOAT);
M_FLOAT tanh(M_FLOAT);

#ifdef __LIB_32B__
#define acosf(x)    acos(x)
#define asinf(x)    asin(x)
#define atanf(x)    atan(x)
#define atan2f(x,y) atan2(x,y)
#define ceilf(x)    ceil(x)
#define cosf(x)     cos(x)
#define coshf(x)    cosh(x)
#define expf(x)     exp(x)
#define fabsf(x)    fabs(x)
#define floorf(x)   floor(x)
#define fmodf(x,y)  fmod(x,y)
#define frexpf(x,y) frexp(x,y)
#define ldexpf(x,y) ldexp(x,y)
#define logf(x)     log(x)
#define log2f(x)    log2(x)
#define log10f(x)   log10(x)
#define powf(x,y)   pow(x,y)
#define roundf(x)   round(x)
#define sinf(x)     sin(x)
#define sinhf(x)    sinh(x)
#define sqrtf(x)    sqrt(x)
#define tanf(x)     tan(x)
#define tanhf(x)    tanh(x)
#endif /* __LIB_32B__ */

extern const M_FLOAT MATH_PI;
extern const M_FLOAT MATH_PI_D;
extern const M_FLOAT MATH_PI_2;
extern const M_FLOAT MATH_PI_4;
extern const M_FLOAT MATH_LN10;

extern const float EPSILON;
extern const M_FLOAT LOG_MULTIPLE;



#define FP_NAN					0
#define FP_INFINITE				1
#define FP_ZERO					2
#define FP_SUBNORMAL			3
#define FP_NORMAL				4

#define NAN						__builtin_nan("")
#define INFINITY				__builtin_inf()
#define	HUGE_VALF				__builtin_huge_valf()
#define	HUGE_VAL				__builtin_huge_val()
#define	HUGE_VALL				__builtin_huge_vall()

#define	isgreater(x, y)			__builtin_isgreater((x), (y))
#define	isgreaterequal(x, y)	__builtin_isgreaterequal((x), (y))
#define	isless(x, y)			__builtin_isless((x), (y))
#define	islessequal(x, y)		__builtin_islessequal((x), (y))
#define	islessgreater(x, y)		__builtin_islessgreater((x), (y))
#define	isunordered(x, y)		__builtin_isunordered((x), (y))


#endif /* _XLIBC_MATH_H */
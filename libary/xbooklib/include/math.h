#ifndef _LIB_MATH_H
#define _LIB_MATH_H

/* max() & min() */
#define	MAX(a,b)	((a) > (b) ? (a) : (b))
#define	MIN(a,b)	((a) < (b) ? (a) : (b))

/* 除后上入 */
#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))

/* 除后下舍 */
#define DIV_ROUND_DOWN(X, STEP) ((X) / (STEP))

#endif /* _LIB_MATH_H */

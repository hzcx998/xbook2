#ifndef _XBOOK_MATH_H
#define _XBOOK_MATH_H

/* max() & min() */
#define	MAX(a,b)	((a) > (b) ? (a) : (b))
#define	MIN(a,b)	((a) < (b) ? (a) : (b))

/* 除后上入 */
#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))

/* 除后下舍 */
#define DIV_ROUND_DOWN(X, STEP) ((X) / (STEP))

#define ALIGN_WITH(x, y) ((x + (y - 1)) & (~(y - 1)))

#endif /* _XBOOK_MATH_H */

#ifndef _XBOOK_MATH_H
#define _XBOOK_MATH_H

/* max() & min() */
#define	MAX(a,b)	((a) > (b) ? (a) : (b))
#define	MIN(a,b)	((a) < (b) ? (a) : (b))

#define	ABS(a)	((a) > 0 ? (a) : (-a))

/* 除后上入 */
#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))

/* 除后下舍 */
#define DIV_ROUND_DOWN(X, STEP) ((X) / (STEP))

#define ALIGN_WITH(x, y) ((x + (y - 1)) & (~(y - 1)))

/* 数组大小除以单个数组就是数组成员数量 */
#define ARRAY_SIZE(array)   (sizeof(array) / sizeof(array[0]))

/* 判断一个数是否为2的次幂 */
#define is_power_of_2(n) (n != 0 && ((n & (n - 1)) == 0)) 

static inline long fls(long x)
{
    long position;
    long i;
    if (0 != x) {
        for (i = (x >> 1), position = 0; i != 0; ++position)
            i >>= 1;
    } else
        position = -1;
    return position + 1;
}

static inline unsigned long roundup_pow_of_two(unsigned long x)
{
    return 1UL << fls(x - 1);
}

static inline int powi(int x, int n)
{
    int res = 1;
    if(n < 0){
        x = 1 / x;
        n = -n;
    }
    while(n){
        if(n & 1)
            res *= x;
        x *= x;
        n >>= 1;
    }
    return res;
}


#endif /* _XBOOK_MATH_H */

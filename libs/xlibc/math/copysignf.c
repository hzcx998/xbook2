#include <math.h>
#include <stdint.h>
// #include "math_private.h"

float copysignf(float x, float y)
{
	uint32_t ix,iy;
	GET_FLOAT_WORD(ix,x);
	GET_FLOAT_WORD(iy,y);
	SET_FLOAT_WORD(x,(ix&0x7fffffff)|(iy&0x80000000));
    return x;
}
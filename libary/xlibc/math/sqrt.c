#include <math.h>

/*
* sqrt - 计算平方根
* @x: 值
*/
double sqrt (double x)
{
    double xhalf = 0.5f*x;
    int i = *(int*)&x;
    i = 0x5f375a86- (i>>1);
    x = *(float*)&i;
    x = x*(1.5f-xhalf*x*x);
	x = 1.0 / x;
    return x;
}
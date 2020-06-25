#include <math.h>

/*
* cos - 计算余弦值
* @x: 值
*/
double cos(double x) 
{
	double tmp = sin (x);
	tmp *= tmp;
	return sqrt (1 - tmp);
}

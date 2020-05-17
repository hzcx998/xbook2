#include <math.h>

/**
* sin - 计算正弦值
* @x: 值
*/
double sin(double x) 
{
	double n = x,sum=0;
    int i = 1;
    do {
        sum += n;
        i++;
        n = -n * x*x / (2 * i - 1) / (2 * i - 2);
    } while (fabs(n) >= 1e-10);
    return sum;
}

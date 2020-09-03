#include "test.h"
#include <math.h>
#define PI 3.14159265
int math_test(int argc, char *argv[])
{
    double param, result;
    param = 30.0;
    result = sin (param*PI/180);
    printf ("The sine of %f degrees is %f.\n", param, result );
    return 0;
}

#include "test.h"
#include <math.h>
#define PI 3.14159265
int math_test(int argc, char *argv[])
{
    int i;
    for (i = 0; i < 5; i++) {
        if (!fork()) {
                double param, result;
            param = 30.0;
            result = sin (param*PI/64);
            printf ("The sine of %f degrees is %f.\n", param, result );
            //sleep(1);
            return 0;
        }
    }
    return 0;
}

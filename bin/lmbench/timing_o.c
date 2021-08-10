#include <stdio.h>
#include "bench.h"

int
timing_o_main()
{
	putenv("LOOP_O=0.0");
	printf("%lu\n", (unsigned long)t_overhead());
	return (0);
}

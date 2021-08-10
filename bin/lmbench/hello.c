#include "bench.h"

int
hello_main()
{
	write(1, "Hello world\n", 12);
	return (0);
}

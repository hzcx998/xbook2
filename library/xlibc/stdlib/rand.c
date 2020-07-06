#include <stdlib.h>

static unsigned long int __next_seed = 1;

/* rand:  return pseudo-random integer on 0..32767 */
int rand()
{
	__next_seed = __next_seed * 1103515245 + 12345;
	return (unsigned int )(__next_seed / 65536) % RAND_MAX;
}
/* srand:  set seed for rand() */
void srand(unsigned long seed)
{
    __next_seed = seed;
}

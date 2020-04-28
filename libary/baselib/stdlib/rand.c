#include <stdlib.h>

unsigned long int next = 1;

/* rand:  return pseudo-random integer on 0..32767 */
int rand()
{
	next = next * 1103515245 + 12345;
	return (unsigned int )(next / 65536) % RAND_MAX;
}
/* srand:  set seed for rand() */
void srand(unsigned long seed)
{
    next = seed;
}

#include <stdint.h>

int64_t __divmoddi4(int64_t num, int64_t den, int64_t * rem_p)
{
    int64_t quot = 0, qbit = 1;

    if (den == 0) {
	asm volatile ("int $0");
	return 0;		/* If trap returns... */
    }

    /* Left-justify denominator and count shift */
    while ((int64_t) den >= 0) {
	den <<= 1;
	qbit <<= 1;
    }

    while (qbit) {
	if (den <= num) {
	    num -= den;
	    quot += qbit;
	}
	den >>= 1;
	qbit >>= 1;
    }

    if (rem_p)
	*rem_p = num;

    return quot;
}
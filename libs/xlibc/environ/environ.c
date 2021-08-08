/*
 * libc/environ/environ.c
 */

#include <stddef.h>
#include <environ.h>

struct environ_t __xenviron = {
	.content = NULL,
	.prev = &__xenviron,
	.next = &__xenviron,
};

void __environ_init()
{

}
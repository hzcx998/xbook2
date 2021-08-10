/*
 * line.c - guess the cache line size
 *
 * usage: line
 *
 * Copyright (c) 2000 Carl Staelin.
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char	*id = "$Id$\n";

#include "bench.h"

/*
 * Assumptions:
 *
 * 1) Cache lines are a multiple of pointer-size words
 * 2) Cache lines are no larger than 1/4 a page size
 * 3) Pages are an even multiple of cache lines
 */
int
line_main(int ac, char **av)
{
	int	i, j, l;
	int	verbose = 0;
	int	warmup = 0;
	int	repetitions = TRIES;
	int	c;
	size_t	maxlen = 64 * 1024 * 1024;
	struct mem_state state;
	char   *usage = "[-v] [-W <warmup>] [-N <repetitions>][-M len[K|M]]\n";

	state.line = sizeof(char*);
	state.pagesize = getpagesize();

	while (( c = getopt(ac, av, "avM:W:N:")) != EOF) {
		switch(c) {
		case 'v':
			verbose = 1;
			break;
		case 'M':
			maxlen = bytes(optarg);
			break;
		case 'W':
			warmup = atoi(optarg);
			break;
		case 'N':
			repetitions = atoi(optarg);
			break;
		default:
			lmbench_usage(ac, av, usage);
			break;
		}
	}

	if ((l = line_find(maxlen, warmup, repetitions, &state)) > 0) {
		if (verbose) {
			printf("cache line size: %d bytes\n", l);
		} else {
			printf("%d\n", l);
		}
	}

	return (0);
}

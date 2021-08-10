/*
 * lat_mem_rd.c - measure memory load latency
 *
 * usage: lat_mem_rd [-P <parallelism>] [-W <warmup>] [-N <repetitions>] [-t] size-in-MB [stride ...]
 *
 * Copyright (c) 1994 Larry McVoy.  
 * Copyright (c) 2003, 2004 Carl Staelin.
 *
 * Distributed under the FSF GPL with additional restriction that results 
 * may published only if:
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char	*id = "$Id: s.lat_mem_rd.c 1.13 98/06/30 16:13:49-07:00 lm@lm.bitmover.com $\n";

#include "bench.h"
#define STRIDE  (512/sizeof(char *))
#define	LOWER	512
static void	loads(size_t len, size_t range, size_t stride, 
	      int parallel, int warmup, int repetitions);
size_t	step(size_t k);
static void	initialize(iter_t iterations, void* cookie);

benchmp_f	fpInit = stride_initialize;

int
lat_mem_rd_main(int ac, char **av)
{
	int	i;
	int	c;
	int	parallel = 1;
	int	warmup = 0;
	int	repetitions = TRIES;
        size_t	len;
	size_t	range;
	size_t	stride;
	char   *usage = "[-P <parallelism>] [-W <warmup>] [-N <repetitions>] [-t] len [stride...]\n";

	while (( c = getopt(ac, av, "tP:W:N:")) != EOF) {
		switch(c) {
		case 't':
			fpInit = thrash_initialize;
			break;
		case 'P':
			parallel = atoi(optarg);
			if (parallel <= 0) lmbench_usage(ac, av, usage);
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
	if (optind == ac) {
		lmbench_usage(ac, av, usage);
	}

        len = atoi(av[optind]);
	len *= 1024 * 1024;

	if (optind == ac - 1) {
		fprintf(stderr, "\"stride=%d\n", STRIDE);
		for (range = LOWER; range <= len; range = step(range)) {
			loads(len, range, STRIDE, parallel, 
			      warmup, repetitions);
		}
	} else {
		for (i = optind + 1; i < ac; ++i) {
			stride = bytes(av[i]);
			fprintf(stderr, "\"stride=%d\n", stride);
			for (range = LOWER; range <= len; range = step(range)) {
				loads(len, range, stride, parallel, 
				      warmup, repetitions);
			}
			fprintf(stderr, "\n");
		}
	}
	return(0);
}

#define	ONE	p = (char **)*p;
#define	FIVE	ONE ONE ONE ONE ONE
#define	TEN	FIVE FIVE
#define	FIFTY	TEN TEN TEN TEN TEN
#define	HUNDRED	FIFTY FIFTY


static void
benchmark_loads(iter_t iterations, void *cookie)
{
	struct mem_state* state = (struct mem_state*)cookie;
	register char **p = (char**)state->p[0];
	register size_t i;
	register size_t count = state->len / (state->line * 100) + 1;

	while (iterations-- > 0) {
		for (i = 0; i < count; ++i) {
			HUNDRED;
		}
	}

	use_pointer((void *)p);
	state->p[0] = (char*)p;
}


static void
loads(size_t len, size_t range, size_t stride, 
	int parallel, int warmup, int repetitions)
{
	double result;
	size_t count;
	struct mem_state state;

	if (range < stride) return;

	state.width = 1;
	state.len = range;
	state.maxlen = len;
	state.line = stride;
	state.pagesize = getpagesize();
	count = 100 * (state.len / (state.line * 100) + 1);

#if 0
	(*fpInit)(0, &state);
	fprintf(stderr, "loads: after init\n");
	(*benchmark_loads)(2, &state);
	fprintf(stderr, "loads: after benchmark\n");
	mem_cleanup(0, &state);
	fprintf(stderr, "loads: after cleanup\n");
	settime(1);
	save_n(1);
#else
	/*
	 * Now walk them and time it.
	 */
	benchmp(fpInit, benchmark_loads, mem_cleanup, 
		100000, parallel, warmup, repetitions, &state);
#endif

	/* We want to get to nanoseconds / load. */
	save_minimum();
	result = (1000. * (double)gettime()) / (double)(count * get_n());
	fprintf(stderr, "%.5f %.3f\n", range / (1024. * 1024.), result);

}

size_t
step(size_t k)
{
	if (k < 1024) {
		k = k * 2;
        } else if (k < 4*1024) {
		k += 1024;
	} else {
		size_t s;

		for (s = 32 * 1024; s <= k; s *= 2)
			;
		k += s / 16;
	}
	return (k);
}

/*
 * tlb.c - guess the cache line size
 *
 * usage: tlb [-c] [-L <line size>] [-M len[K|M]] [-W <warmup>] [-N <repetitions>]
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

int find_tlb(int start, int maxpages, int warmup, int repetitions, 
	     double* tlb_time, double* cache_time, struct mem_state* state);
void compute_times(int pages, int warmup, int repetitions,
	     double* tlb_time, double* cache_time, struct mem_state* state);

#define THRESHOLD 1.15

/*
 * Assumptions:
 *
 * 1) Cache lines are a multiple of pointer-size words
 * 2) Cache lines no larger than 1/8 a page size
 * 3) Pages are an even multiple of cache lines
 */
int
tlb_main(int ac, char **av)
{
	int	i, l, len, tlb, maxpages;
	int	c;
	int	print_cost = 0;
	int	warmup = 0;
	int	repetitions = TRIES;
	double	tlb_time, cache_time, diff;
	struct mem_state state;
	char   *usage = "[-c] [-L <line size>] [-M len[K|M]] [-W <warmup>] [-N <repetitions>]\n";

	maxpages = 16 * 1024;
	state.width = 1;
	state.pagesize = getpagesize();
	state.line = sizeof(char*);

	tlb = 2;

	while (( c = getopt(ac, av, "cL:M:W:N:")) != EOF) {
		switch(c) {
		case 'c':
			print_cost = 1;
			break;
		case 'L':
			state.line = atoi(optarg);
			break;
		case 'M':
			maxpages = bytes(optarg);	/* max in bytes */
			maxpages /= getpagesize();	/* max in pages */
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

	/* assumption: no TLB will have less than 16 entries */
	tlb = find_tlb(8, maxpages, warmup, repetitions, &tlb_time, &cache_time, &state);

	if (tlb > 0) {
		if (print_cost) {
			compute_times(tlb * 2, warmup, repetitions, &tlb_time, &cache_time, &state);
			fprintf(stderr, "tlb: %d pages %.5f nanoseconds\n", tlb, tlb_time - cache_time);
		} else {
			fprintf(stderr, "tlb: %d pages\n", tlb);
		}
	}

	/*
	for (i = tlb<<1; i <= maxpages; i<<=1) {
		compute_times(i, warmup, repetitions, &tlb_time, &cache_time, &state);
	}
	/**/

	return(0);
}

int
find_tlb(int start, int maxpages, int warmup, int repetitions,
	 double* tlb_time, double* cache_time, struct mem_state* state)
{
	int	i, lower, upper;

	for (i = start; i <= maxpages; i<<=1) {
		compute_times(i, warmup, repetitions, tlb_time, cache_time, state);

		if (*tlb_time / *cache_time > THRESHOLD) {
			lower = i>>1;
			upper = i;
			i = lower + (upper - lower) / 2;
			break;
		}
	}

	/* we can't find any tlb effect */
	if (i >= maxpages) {
		state->len = 0;
		return (0);
	}

	/* use a binary search to locate point at which TLB effects start */
	while (lower + 1 < upper) {
		compute_times(i, warmup, repetitions, tlb_time, cache_time, state);

		if (*tlb_time / *cache_time > THRESHOLD) {
			upper = i;
		} else {
			lower = i;
		}
		i = lower + (upper - lower) / 2;
	}
	return (lower);
}

void
compute_times(int pages, int warmup, int repetitions,
	 double* tlb_time, double* cache_time, struct mem_state* state)
{
	int i;
	result_t tlb_results, cache_results, *r_save;

	r_save = get_results();
	insertinit(&tlb_results);
	insertinit(&cache_results);

	state->len = pages * state->pagesize;
	state->maxlen = pages * state->pagesize;
	tlb_initialize(0, state);
	if (state->initialized) {
		for (i = 0; i < TRIES; ++i) {
			BENCH1(mem_benchmark_0(__n, state); __n = 1;, 0);
			insertsort(gettime(), get_n(), &tlb_results);
		}
	}
	tlb_cleanup(0, state);
	
	state->len = pages * state->line;
	state->maxlen = pages * state->line;
	mem_initialize(0, state);
	if (state->initialized) {
		for (i = 0; i < TRIES; ++i) {
			BENCH1(mem_benchmark_0(__n, state); __n = 1;, 0);
			insertsort(gettime(), get_n(), &cache_results);
		}
	}
	mem_cleanup(0, state);

	/* We want nanoseconds / load. */
	set_results(&tlb_results);
	*tlb_time = (1000. * (double)gettime()) / (100. * (double)get_n());

	/* We want nanoseconds / load. */
	set_results(&cache_results);
	*cache_time = (1000. * (double)gettime()) / (100. * (double)get_n());
	set_results(r_save);

	/*
	fprintf(stderr, "%d %.5f %.5f\n", pages, *tlb_time, *cache_time);
	/**/
}


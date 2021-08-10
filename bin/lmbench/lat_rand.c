/*
 * lat_rand.c - random number generation
 *
 * usage: lat_rand [-P <parallelism>] [-W <warmup>] [-N <repetitions>]
 *
 * Copyright (c) 2002 Carl Staelin.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Hewlett-Packard is gratefully acknowledged.
 */
static char	*id = "$Id$\n";

#include "bench.h"

#ifdef HAVE_DRAND48
static void bench_drand48(iter_t iterations, void *cookie);
static void bench_lrand48(iter_t iterations, void *cookie);
#endif
#ifdef HAVE_RAND
static void bench_rand(iter_t iterations, void *cookie);
#endif
#ifdef HAVE_RANDOM
static void bench_random(iter_t iterations, void *cookie);
#endif
int 
lat_rand_main(int ac, char **av)
{
	int parallel = 1;
	int warmup = 0;
	int repetitions = TRIES;
	int c;
	char* usage = "[-P <parallelism>] [-W <warmup>] [-N <repetitions>]\n";

	while (( c = getopt(ac, av, "P:W:N:")) != EOF) {
		switch(c) {
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
	if (optind < ac) {
		lmbench_usage(ac, av, usage);
	}

#ifdef HAVE_DRAND48
	benchmp(NULL, bench_drand48, NULL,
		0, parallel, warmup, repetitions, NULL);
	nano("drand48 latency", get_n());

	benchmp(NULL, bench_lrand48, NULL,
		0, parallel, warmup, repetitions, NULL);
	nano("lrand48 latency", get_n());
#endif
#ifdef HAVE_RAND
	benchmp(NULL, bench_rand, NULL,
		0, parallel, warmup, repetitions, NULL);
	nano("rand latency", get_n());
#endif
#ifdef HAVE_RANDOM
	benchmp(NULL, bench_random, NULL,
		0, parallel, warmup, repetitions, NULL);
	nano("random latency", get_n());
#endif
	return (0);
}

#ifdef HAVE_DRAND48
static void
bench_drand48(register iter_t iterations, void *cookie)
{
	register double v = 0.0;
	while (iterations-- > 0) {
		v += drand48();
	}
	use_int((int)v);
}

static void
bench_lrand48(register iter_t iterations, void *cookie)
{
	register long v = 0.0;
	while (iterations-- > 0) {
		v += lrand48();
	}
	use_int((int)v);
}
#endif /* HAVE_DRAND48 */
#ifdef HAVE_RAND
static void
bench_rand(register iter_t iterations, void *cookie)
{
	register int v = 0.0;
	while (iterations-- > 0) {
		v += rand();
	}
	use_int((int)v);
}
#endif /* HAVE_RAND */
#ifdef HAVE_RANDOM
static void
bench_random(register iter_t iterations, void *cookie)
{
	register int v = 0.0;
	while (iterations-- > 0) {
		v += random();
	}
	use_int((int)v);
}
#endif /* HAVE_RANDOM */

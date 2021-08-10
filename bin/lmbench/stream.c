/*
 * steam.c - lmbench version of John McCalpin's STREAM benchmark
 *
 * usage: stream
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

struct _state {
	double*	a;
	double*	b;
	double*	c;
	double	scalar;
	int	len;
};

void initialize(iter_t iterations, void* cookie);
void cleanup(iter_t iterations, void* cookie);

/* These are from STREAM version 1 */
void copy(iter_t iterations, void* cookie);
void scale(iter_t iterations, void* cookie);
void add(iter_t iterations, void* cookie);
void triad(iter_t iterations, void* cookie);

/* These are from STREAM version 2 */
void fill(iter_t iterations, void* cookie);
/* NOTE: copy is the same as in version 1 */
void daxpy(iter_t iterations, void* cookie);
void sum(iter_t iterations, void* cookie);


/*
 * Assumptions:
 *
 * 1) Cache lines are a multiple of pointer-size words
 * 2) Cache lines are no larger than 1/4 a page size
 * 3) Pages are an even multiple of cache lines
 */
int
stream_main(int ac, char **av)
{
	int	i, j, l;
	int	version = 1;
	int	parallel = 1;
	int	warmup = 0;
	int	repetitions = TRIES;
	int	c;
	uint64	datasize;
	struct _state state;
	char   *p;
	char   *usage = "[-v <stream version 1|2>] [-M <len>[K|M]] [-P <parallelism>] [-W <warmup>] [-N <repetitions>]\n";

        state.len = 1000 * 1000 * 3 * sizeof(double);
	state.scalar = 3.0;

	while (( c = getopt(ac, av, "v:M:P:W:N:")) != EOF) {
		switch(c) {
		case 'v':
			version = atoi(optarg);
			if (version != 1 && version != 2) 
				lmbench_usage(ac, av, usage);
			break;
		case 'P':
			parallel = atoi(optarg);
			if (parallel <= 0) lmbench_usage(ac, av, usage);
			break;
		case 'M':
			state.len = bytes(optarg);
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

	/* ensure that we can malloc the desired space */
	while (!(p = malloc(state.len)))
		state.len /= 2;
	free(p);
		
	/* convert from bytes to array length */
	state.len /= 3 * sizeof(double);
	datasize = sizeof(double) * state.len * parallel;

	if (version == 1) {
		benchmp(initialize, copy, cleanup, 
			0, parallel, warmup, repetitions, &state);
		if (gettime() > 0) {
			if (parallel <= 1) save_minimum();
			nano("STREAM copy latency", state.len * get_n());
			fprintf(stderr, "STREAM copy bandwidth: ");
			mb(2 * datasize * get_n());
		}

		benchmp(initialize, scale, cleanup, 
			0, parallel, warmup, repetitions, &state);
		if (gettime() > 0) {
			if (parallel <= 1) save_minimum();
			nano("STREAM scale latency", state.len * get_n());
			fprintf(stderr, "STREAM scale bandwidth: ");
			mb(2 * datasize * get_n());
		}

		benchmp(initialize, sum, cleanup, 
			0, parallel, warmup, repetitions, &state);
		if (gettime() > 0) {
			if (parallel <= 1) save_minimum();
			nano("STREAM add latency", state.len * get_n());
			fprintf(stderr, "STREAM add bandwidth: ");
			mb(3 * datasize * get_n());
		}

		benchmp(initialize, triad, cleanup, 
			0, parallel, warmup, repetitions, &state);
		if (gettime() > 0) {
			if (parallel <= 1) save_minimum();
			nano("STREAM triad latency", state.len * get_n());
			fprintf(stderr, "STREAM triad bandwidth: ");
			mb(3 * datasize * get_n());
		}
	} else {
		benchmp(initialize, fill, cleanup, 
			0, parallel, warmup, repetitions, &state);
		if (gettime() > 0) {
			if (parallel <= 1) save_minimum();
			nano("STREAM2 fill latency", state.len * get_n());
			fprintf(stderr, "STREAM2 fill bandwidth: ");
			mb(datasize * get_n());
		}

		benchmp(initialize, copy, cleanup, 
			0, parallel, warmup, repetitions, &state);
		if (gettime() > 0) {
			if (parallel <= 1) save_minimum();
			nano("STREAM2 copy latency", state.len * get_n());
			fprintf(stderr, "STREAM2 copy bandwidth: ");
			mb(2 * datasize * get_n());
		}

		benchmp(initialize, daxpy, cleanup, 
			0, parallel, warmup, repetitions, &state);
		if (gettime() > 0) {
			if (parallel <= 1) save_minimum();
			nano("STREAM2 daxpy latency", state.len * get_n());
			fprintf(stderr, "STREAM2 daxpy bandwidth: ");
			mb(3 * datasize * get_n());
		}

		benchmp(initialize, sum, cleanup, 
			0, parallel, warmup, repetitions, &state);
		if (gettime() > 0) {
			if (parallel <= 1) save_minimum();
			nano("STREAM2 sum latency", state.len * get_n());
			fprintf(stderr, "STREAM2 sum bandwidth: ");
			mb(datasize * get_n());
		}
	}

	return(0);
}

void
initialize(iter_t iterations, void* cookie)
{
	int i;
	struct _state* state = (struct _state*)cookie;
	
	if (iterations) return;

	state->a = (double*)malloc(sizeof(double) * state->len);
	state->b = (double*)malloc(sizeof(double) * state->len);
	state->c = (double*)malloc(sizeof(double) * state->len);

	if (state->a == NULL || state->b == NULL || state->c == NULL) {
		exit(1);
	}

	for (i = 0; i < state->len; ++i) {
		state->a[i] = 1.;
		state->b[i] = 2.;
		state->c[i] = 0.;
	}
}

#define BODY(expr)							\
{									\
	register int i;							\
	register int N = state->len;					\
	register double* a = state->a;					\
	register double* b = state->b;					\
	register double* c = state->c;					\
	register double scalar = state->scalar;				\
									\
	state->a = state->b;						\
	state->b = state->c;						\
	state->c = a;							\
									\
	for (i = 0; i < N; ++i) {					\
		expr;							\
	}								\
}

void
copy(iter_t iterations, void *cookie)
{
	struct _state* state = (struct _state*)cookie;

	while (iterations-- > 0) {
		BODY(c[i] = a[i];)
	}
}

void
scale(iter_t iterations, void *cookie)
{
	struct _state* state = (struct _state*)cookie;

	while (iterations-- > 0) {
		BODY(b[i] = scalar * c[i];)
	}
}

void
add(iter_t iterations, void *cookie)
{
	struct _state* state = (struct _state*)cookie;

	while (iterations-- > 0) {
		BODY(c[i] = a[i] + b[i];)
	}
}

void
triad(iter_t iterations, void *cookie)
{
	struct _state* state = (struct _state*)cookie;

	while (iterations-- > 0) {
		BODY(a[i] = b[i] + scalar * c[i];)
	}
}

/*
 * STREAM version 2 benchmark kernels
 *
 * NOTE: copy is the same as version 1's benchmark
 */
void
fill(iter_t iterations, void *cookie)
{
	struct _state* state = (struct _state*)cookie;

	while (iterations-- > 0) {
		BODY(a[i] = 0;)
	}
}

void
daxpy(iter_t iterations, void *cookie)
{
	struct _state* state = (struct _state*)cookie;

	while (iterations-- > 0) {
		BODY(a[i] = a[i] + scalar * b[i];)
	}
}

void
sum(iter_t iterations, void *cookie)
{
	register double	s;
	struct _state* state = (struct _state*)cookie;

	s = 0.0;
	while (iterations-- > 0) {
		BODY(s += a[i];)
	}
	use_int((int)s);
}

void
cleanup(iter_t iterations, void* cookie)
{
	struct _state* state = (struct _state*)cookie;

	if (iterations) return;

	free(state->a);
	free(state->b);
	free(state->c);
}




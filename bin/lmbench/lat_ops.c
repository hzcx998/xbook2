/*
 * lat_ops.c - benchmark of simple operations
 *
 * Copyright (c) 1996-2004 Carl Staelin and Larry McVoy.  
 *
 * This benchmark is meant to benchmark raw arithmetic operation
 * latency for various operations on various datatypes.  Obviously,
 * not all operations make sense for all datatypes (e.g., modulus
 * on float).  The benchmarks are configured to use interlocking
 * operations, so we measure the time of an individual operation.
 * 
 * The exception to the interlocking operation guidelines are the
 * vector operations, muladd and bogomflops, for both float and
 * double data types.  In this case we are trying to determine
 * how well the CPU can schedule the various arithmetic units
 * and overlap adjacent operations to get the maximal throughput
 * from the system.  In addition, we are using relatively short
 * vectors so these operations should be going to/from L1 (or
 * possibly L2) cache, rather than lat_ops_main memory, which should
 * reduce or eliminate the memory overheads.
 *
 * The vector operations use a slightly unrolled loop because
 * this is common in scientific codes that do these sorts of
 * operations.
 */
static char	*id = "$Id$\n";

#include "bench.h"

struct _state {
	int	N;
	int	M;
	int	K;
	double*	data;
};

#define FIVE(a) a a a a a
#define TEN(a) a a a a a a a a a a
#define HUNDRED(a) TEN(TEN(a))

static void
float_initialize(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register int i;
	register float* x;

	if (iterations) return;

	x = (float*)malloc(pState->M * sizeof(float));
	pState->data = (double*)x;
	for (i = 0; i < pState->M; ++i) {
		x[i] = 3.14159265;
	}
}

static void
double_initialize(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register int i;

	if (iterations) return;

	pState->data = (double*)malloc(pState->M * sizeof(double));
	for (i = 0; i < pState->M; ++i) {
		pState->data[i] = 3.14159265;
	}
}

static void
cleanup(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;

	if (iterations) return;

	if (pState->data) 
		free(pState->data);
}

static void
do_integer_bitwise(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register int r = pState->N;
	register int s = (int)iterations;

	while (iterations-- > 0) {
		HUNDRED(r ^= iterations; s ^= r; r |= s;)
	}
	use_int(r);
}

static void
do_integer_add(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register int a = pState->N + 57;
	register int b = pState->N + 31;

	while (iterations-- > 0) {
		HUNDRED(a += b; b -= a;)
	}
	use_int(a+b);
}

static void
do_integer_mul(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register int r = pState->N + 37431;
	register int s = pState->N + 4;
	register int t = r * s * s * s * s * s * s * s * s * s * s - r;

	while (iterations-- > 0) {
		TEN(r *= s;); r -= t;
		TEN(r *= s;); r -= t;
	}
	use_int(r);
}

static void
do_integer_div(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register int r = pState->N + 36;
	register int s = (r + 1) << 20;

	while (iterations-- > 0) {
		HUNDRED(r = s / r;)
	}
	use_int(r);
}

static void
do_integer_mod(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register int r = pState->N + iterations;
	register int s = pState->N + 62;

	while (iterations-- > 0) {
		HUNDRED(r %= s; r |= s;)
	}
	use_int(r);
}

static void
do_int64_bitwise(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register int64 r = (int64)pState->N | (int64)pState->N<<32;
	register int64 s = (int64)iterations | (int64)iterations<<32;
	register int64 i = (int64)iterations<<34 - 1;

	while (iterations-- > 0) {
		HUNDRED(r ^= i; s ^= r; r |= s;)
		i--;
	}
	use_int((int)r);
}

static void
do_int64_add(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register int64 a = (int64)pState->N + 37420;
	register int64 b = (int64)pState->N + 21698324;

	a += (int64)(0xFE + pState->N)<<30;
	b += (int64)(0xFFFE + pState->N)<<29;

	while (iterations-- > 0) {
		HUNDRED(a += b; b -= a;)
	}
	use_int((int)a+(int)b);
}

static void
do_int64_mul(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register int64 r = (int64)pState->N + 37420;
	register int64 s = (int64)pState->N + 4;
	register int64 t;

	r += (int64)(pState->N + 6)<<32;
	t = r * s * s * s * s * s * s * s * s * s * s - r;

	while (iterations-- > 0) {
		TEN(r *= s;); r -= t;
		TEN(r *= s;); r -= t;
	}
	use_int((int)r);
}

static void
do_int64_div(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register int64 r = (int64)pState->N + 36;
	register int64 s;

	r += r << 33;
	s = (r + 17) << 13;

	while (iterations-- > 0) {
		HUNDRED(r = s / r;)
	}
	use_int((int)r);
}

static void
do_int64_mod(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register int64 r = iterations + (int64)iterations<<32;
	register int64 s = (int64)pState->N + (int64)pState->N<<56;

	while (iterations-- > 0) {
		HUNDRED(r %= s; r |= s;);
	}
	use_int((int)r);
}

static void
do_float_add(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register float f = (float)pState->N;
	register float g = (float)pState->K;

	while (iterations-- > 0) {
		TEN(f += (float)f;) f += (float)g;
		TEN(f += (float)f;) f += (float)g;
	}
	use_int((int)f);
	use_int((int)g);
}

static void
do_float_mul(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register float f = 8.0f * (float)pState->N;
	register float g = 0.125f * (float)pState->M / 1000.0;

	while (iterations-- > 0) {
		TEN(f *= f; f *= g;);
		TEN(f *= f; f *= g;);
	}
	use_int((int)f);
	use_int((int)g);
}

static void
do_float_div(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register float f = 1.41421356f * (float)pState->N;
	register float g = 3.14159265f * (float)pState->M / 1000.0;

	while (iterations-- > 0) {
		FIVE(TEN(f = g / f;) TEN(g = f / g;))
	}
	use_int((int)f);
	use_int((int)g);
}

static void
do_double_add(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register double f = (double)pState->N;
	register double g = (double)pState->K;

	while (iterations-- > 0) {
		TEN(f += (double)f;) f += (double)g;
		TEN(f += (double)f;) f += (double)g;
	}
	use_int((int)f);
	use_int((int)g);
}

static void
do_double_mul(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register double f = 8.0 * (double)pState->N;
	register double g = 0.125 * (double)pState->M / 1000.0;

	while (iterations-- > 0) {
		TEN(f *= f; f *= g;)
		TEN(f *= f; f *= g;)
	}
	use_int((int)f);
	use_int((int)g);
}

static void
do_double_div(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register double f = 1.41421356 * (double)pState->N;
	register double g = 3.14159265 * (double)pState->M / 1000.0;

	while (iterations-- > 0) {
		FIVE(TEN(f = g / f;) TEN(g = f / g;))
	}
	use_int((int)f);
	use_int((int)g);
}

static void
do_float_bogomflops(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register int i;
	register int M = pState->M / 10;

	while (iterations-- > 0) {
		register float *x = (float*)pState->data;
		for (i = 0; i < M; ++i) {
			x[0] = (1.0f + x[0]) * (1.5f - x[0]) / x[0];
			x[1] = (1.0f + x[1]) * (1.5f - x[1]) / x[1];
			x[2] = (1.0f + x[2]) * (1.5f - x[2]) / x[2];
			x[3] = (1.0f + x[3]) * (1.5f - x[3]) / x[3];
			x[4] = (1.0f + x[4]) * (1.5f - x[4]) / x[4];
			x[5] = (1.0f + x[5]) * (1.5f - x[5]) / x[5];
			x[6] = (1.0f + x[6]) * (1.5f - x[6]) / x[6];
			x[7] = (1.0f + x[7]) * (1.5f - x[7]) / x[7];
			x[8] = (1.0f + x[8]) * (1.5f - x[8]) / x[8];
			x[9] = (1.0f + x[9]) * (1.5f - x[9]) / x[9];
			x += 10;
		}
	}
}

static void
do_double_bogomflops(iter_t iterations, void* cookie)
{
	struct _state *pState = (struct _state*)cookie;
	register int i;
	register int M = pState->M / 10;

	while (iterations-- > 0) {
		register double *x = (double*)pState->data;
		for (i = 0; i < M; ++i) {
			x[0] = (1.0f + x[0]) * (1.5f - x[0]) / x[0];
			x[1] = (1.0f + x[1]) * (1.5f - x[1]) / x[1];
			x[2] = (1.0f + x[2]) * (1.5f - x[2]) / x[2];
			x[3] = (1.0f + x[3]) * (1.5f - x[3]) / x[3];
			x[4] = (1.0f + x[4]) * (1.5f - x[4]) / x[4];
			x[5] = (1.0f + x[5]) * (1.5f - x[5]) / x[5];
			x[6] = (1.0f + x[6]) * (1.5f - x[6]) / x[6];
			x[7] = (1.0f + x[7]) * (1.5f - x[7]) / x[7];
			x[8] = (1.0f + x[8]) * (1.5f - x[8]) / x[8];
			x[9] = (1.0f + x[9]) * (1.5f - x[9]) / x[9];
			x += 10;
		}
	}
}

int
lat_ops_main(int ac, char **av)
{
	int	__n = 1;
	int	c, i, j;
	int	warmup = 0;
	int	parallel = 1;
	int	repetitions = TRIES;
	uint64	iop_time;
	uint64	iop_N;
	struct _state state;
	char   *usage = "[-W <warmup>] [-N <repetitions>] [-P <parallel>] \n";

	state.N = 1;
	state.M = 1000;
	state.K = -1023;
	state.data = NULL;

	while (( c = getopt(ac, av, "W:N:P:")) != EOF) {
		switch(c) {
		case 'W':
			warmup = atoi(optarg);
			break;
		case 'N':
			repetitions = atoi(optarg);
			break;
		case 'P':
			parallel = atoi(optarg);
			if (parallel <= 0) lmbench_usage(ac, av, usage);
			break;
		default:
			lmbench_usage(ac, av, usage);
			break;
		}
	}

	benchmp(NULL, do_integer_bitwise, NULL, 
		0, 1, warmup, repetitions, &state);
	nano("integer bit", get_n() * 100 * 3);
	
	benchmp(NULL, do_integer_add, NULL, 
		0, 1, warmup, repetitions, &state);
	nano("integer add", get_n() * 100 * 2);
	iop_time = gettime();
	iop_N = get_n() * 100 * 2;
	
	benchmp(NULL, do_integer_mul, NULL, 
		0, 1, warmup, repetitions, &state);
	settime(gettime() - (get_n() * 2 * iop_time) / iop_N);
	nano("integer mul", get_n() * 10 * 2);
	
	benchmp(NULL, do_integer_div, NULL, 
		0, 1, warmup, repetitions, &state);
	nano("integer div", get_n() * 100);
	
	benchmp(NULL, do_integer_mod, NULL, 
		0, 1, warmup, repetitions, &state);
	settime(gettime() - (get_n() *  100 * iop_time) / iop_N);
	nano("integer mod", get_n() * 100);
	
	benchmp(NULL, do_int64_bitwise, NULL, 
		0, 1, warmup, repetitions, &state);
	nano("int64 bit", get_n() * 100 * 3);
	iop_time = gettime();
	iop_N = get_n() * 100 * 3;

	benchmp(NULL, do_int64_add, NULL, 
		0, 1, warmup, repetitions, &state);
	nano("int64 add", get_n() * 100 * 2);
	
	benchmp(NULL, do_int64_mul, NULL, 
		0, 1, warmup, repetitions, &state);
	settime(gettime() - (get_n() * 2 * iop_time) / iop_N);
	nano("int64 mul", get_n() * 10 * 2);
	
	benchmp(NULL, do_int64_div, NULL, 
		0, 1, warmup, repetitions, &state);
	nano("int64 div", get_n() * 100);
	
	benchmp(NULL, do_int64_mod, NULL, 
		0, 1, warmup, repetitions, &state);
	settime(gettime() - (get_n() * 100 * iop_time) / iop_N);
	nano("int64 mod", get_n() * 100);
	
	benchmp(NULL, do_float_add, NULL, 
		0, 1, warmup, repetitions, &state);
	nano("float add", get_n() * (10 + 1) * 2);
	
	benchmp(NULL, do_float_mul, NULL, 
		0, 1, warmup, repetitions, &state);
	nano("float mul", get_n() * 10 * 2 * 2);
	
	benchmp(NULL, do_float_div, NULL, 
		0, 1, warmup, repetitions, &state);
	nano("float div", get_n() * 100);

	benchmp(NULL, do_double_add, NULL, 
		0, 1, warmup, repetitions, &state);
	nano("double add", get_n() * (10 + 1) * 2);
	
	benchmp(NULL, do_double_mul, NULL, 
		0, 1, warmup, repetitions, &state);
	nano("double mul", get_n() * 10 * 2 * 2);
	
	benchmp(NULL, do_double_div, NULL, 
		0, 1, warmup, repetitions, &state);
	nano("double div", get_n() * 100);

	benchmp(float_initialize, do_float_bogomflops, cleanup, 
		0, parallel, warmup, repetitions, &state);
	nano("float bogomflops", get_n() * state.M);
	fflush(stdout); fflush(stderr);

	benchmp(double_initialize, do_double_bogomflops, cleanup, 
		0, parallel, warmup, repetitions, &state);
	nano("double bogomflops", get_n() * state.M);
	fflush(stdout); fflush(stderr);

	return(0);
}


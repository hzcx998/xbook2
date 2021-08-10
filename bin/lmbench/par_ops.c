/*
 * par_ops.c - benchmark of simple operation parallelism
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
	int	N;
	int	M;
	int	K;
	int*	int_data;
	double*	double_data;
};

static void	initialize(iter_t iterations, void* cookie);
static void	cleanup(iter_t iterations, void* cookie);

#define	FIVE(m)		m m m m m
#define	TEN(m)		FIVE(m) FIVE(m)
#define	FIFTY(m)	TEN(m) TEN(m) TEN(m) TEN(m) TEN(m)
#define	HUNDRED(m)	FIFTY(m) FIFTY(m)

#define MAX_LOAD_PARALLELISM 16

double
max_parallelism(benchmp_f* benchmarks, 
		int warmup, int repetitions, void* cookie)
{
	int		i, j, k;
	double		baseline, max_load_parallelism, load_parallelism;
	result_t	*results, *r_save;

	max_load_parallelism = 1.;

	for (i = 0; i < MAX_LOAD_PARALLELISM; ++i) {
		benchmp(initialize, benchmarks[i], cleanup, 
			0, 1, warmup, repetitions, cookie);
		save_minimum();

		if (gettime() == 0)
			return -1.;

		if (i == 0) {
			baseline = (double)gettime() / (double)get_n();
		} else {
			load_parallelism = baseline;
			load_parallelism /= (double)gettime();
			load_parallelism *= (double)((i + 1) * get_n());
			if (load_parallelism > max_load_parallelism) {
				max_load_parallelism = load_parallelism;
			}
		}
	}
	return max_load_parallelism;
}

#define REPEAT_0(m)	m(0)
#define REPEAT_1(m)	REPEAT_0(m) m(1)
#define REPEAT_2(m)	REPEAT_1(m) m(2)
#define REPEAT_3(m)	REPEAT_2(m) m(3)
#define REPEAT_4(m)	REPEAT_3(m) m(4)
#define REPEAT_5(m)	REPEAT_4(m) m(5)
#define REPEAT_6(m)	REPEAT_5(m) m(6)
#define REPEAT_7(m)	REPEAT_6(m) m(7)
#define REPEAT_8(m)	REPEAT_7(m) m(8)
#define REPEAT_9(m)	REPEAT_8(m) m(9)
#define REPEAT_10(m)	REPEAT_9(m) m(10)
#define REPEAT_11(m)	REPEAT_10(m) m(11)
#define REPEAT_12(m)	REPEAT_11(m) m(12)
#define REPEAT_13(m)	REPEAT_12(m) m(13)
#define REPEAT_14(m)	REPEAT_13(m) m(14)
#define REPEAT_15(m)	REPEAT_14(m) m(15)

#define BENCHMARK(benchmark,N,repeat)					\
void benchmark##_##N(iter_t iterations, void *cookie) 			\
{									\
	register iter_t i = iterations;					\
	struct _state* state = (struct _state*)cookie;			\
	repeat(DECLARE);						\
									\
	repeat(INIT);							\
	while (i-- > 0) {						\
		repeat(PREAMBLE);					\
		TEN(repeat(BODY));					\
	}								\
									\
	repeat(SAVE);							\
}

#define PARALLEL_BENCHMARKS(benchmark)					\
	BENCHMARK(benchmark, 0, REPEAT_0)				\
	BENCHMARK(benchmark, 1, REPEAT_1)				\
	BENCHMARK(benchmark, 2, REPEAT_2)				\
	BENCHMARK(benchmark, 3, REPEAT_3)				\
	BENCHMARK(benchmark, 4, REPEAT_4)				\
	BENCHMARK(benchmark, 5, REPEAT_5)				\
	BENCHMARK(benchmark, 6, REPEAT_6)				\
	BENCHMARK(benchmark, 7, REPEAT_7)				\
	BENCHMARK(benchmark, 8, REPEAT_8)				\
	BENCHMARK(benchmark, 9, REPEAT_9)				\
	BENCHMARK(benchmark, 10, REPEAT_10)				\
	BENCHMARK(benchmark, 11, REPEAT_11)				\
	BENCHMARK(benchmark, 12, REPEAT_12)				\
	BENCHMARK(benchmark, 13, REPEAT_13)				\
	BENCHMARK(benchmark, 14, REPEAT_14)				\
	BENCHMARK(benchmark, 15, REPEAT_15)				\
									\
	benchmp_f benchmark##_benchmarks[] = {				\
		benchmark##_0,						\
		benchmark##_1,						\
		benchmark##_2,						\
		benchmark##_3,						\
		benchmark##_4,						\
		benchmark##_5,						\
		benchmark##_6,						\
		benchmark##_7,						\
		benchmark##_8,						\
		benchmark##_9,						\
		benchmark##_10,						\
		benchmark##_11,						\
		benchmark##_12,						\
		benchmark##_13,						\
		benchmark##_14,						\
		benchmark##_15						\
	};

#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		r##N ^= s##N; s##N ^= r##N; r##N |= s##N;
#define DECLARE(N)	register int r##N, s##N;
#define INIT(N)		r##N = state->int_data[N] + 1; s##N = (N+1) + r##N;
#define PREAMBLE(N)	
#define SAVE(N)		use_int((int)r##N);
PARALLEL_BENCHMARKS(integer_bit)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		a##N += b##N; b##N -= a##N;
#define DECLARE(N)	register int a##N, b##N;
#define INIT(N)		a##N = state->int_data[N] + 57; \
			a##N = state->int_data[N] + 31;
#define PREAMBLE(N)
#define SAVE(N)		use_int(a##N + b##N);
PARALLEL_BENCHMARKS(integer_add)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		r##N *= s##N;
#define DECLARE(N)	register int r##N, s##N, t##N;
#define INIT(N)		r##N = state->int_data[N] - N + 1 + 37431; \
			s##N = state->int_data[N] - N + 1 + 4; \
			t##N = r##N * s##N * s##N * s##N * s##N * s##N * \
				s##N * s##N * s##N * s##N * s##N - r##N; \
			r##N += t##N;
#define PREAMBLE(N)	r##N -= t##N;
#define SAVE(N)		use_int((int)r##N);
PARALLEL_BENCHMARKS(integer_mul)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		r##N = (s##N / r##N);
#define DECLARE(N)	register int r##N, s##N;
#define INIT(N)		r##N = state->int_data[N] - N + 1 + 36; \
			s##N = (r##N + 1) << 20;
#define PREAMBLE(N)	
#define SAVE(N)		use_int((int)r##N);
PARALLEL_BENCHMARKS(integer_div)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		r##N %= s##N; r##N |= s##N;
#define DECLARE(N)	register int r##N, s##N;
#define INIT(N)		r##N = state->int_data[N] - N + 1 + iterations; \
			s##N = state->int_data[N] - N + 1 + 62;
#define PREAMBLE(N)	
#define SAVE(N)		use_int((int)r##N);
PARALLEL_BENCHMARKS(integer_mod)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		r##N ^= i##N; s##N ^= r##N; r##N |= s##N;
#define DECLARE(N)	register int64 r##N, s##N, i##N;
#define INIT(N)		r##N = state->int_data[N] - N + 1; \
			r##N |= r##N << 32; \
			s##N = iterations + state->int_data[N] - N + 1; \
			s##N |= s##N << 32; \
			i##N = (s##N << 2) - (int64)1;
#define PREAMBLE(N)	i##N -= 1;
#define SAVE(N)		use_int((int)r##N);
PARALLEL_BENCHMARKS(int64_bit)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		a##N += b##N; b##N -= a##N;
#define DECLARE(N)	register int64 a##N, b##N;
#define INIT(N)		a##N = state->int_data[N] - N + 1 + 37420; \
			a##N += (int64)(0xFE + state->int_data[N] - N + 1)<<30; \
			b##N = state->int_data[N] - N + 1 + 21698324; \
			b##N += (int64)(0xFFFE + state->int_data[N] - N + 1)<<29;
#define PREAMBLE(N)
#define SAVE(N)		use_int((int)a##N + (int)b##N);
PARALLEL_BENCHMARKS(int64_add)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		r##N = (r##N * s##N);
#define DECLARE(N)	register int64 r##N, s##N, t##N;
#define INIT(N)		r##N = state->int_data[N] - N + 1 + 37420; \
			r##N += (int64)(state->int_data[N] - N + 1 + 6)<<32; \
			s##N = state->int_data[N] - N + 1 + 4; \
			t##N = r##N * s##N * s##N * s##N * s##N * s##N * \
				s##N * s##N * s##N * s##N * s##N - r##N; \
			r##N += t##N;
#define PREAMBLE(N)	r##N -= t##N;
#define SAVE(N)		use_int((int)r##N);
PARALLEL_BENCHMARKS(int64_mul)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		r##N = (s##N / r##N);
#define DECLARE(N)	register int64 r##N, s##N;
#define INIT(N)		r##N = state->int_data[N] - N + 37; \
			r##N += r##N << 33; \
			s##N = (r##N + 17) << 13;
#define PREAMBLE(N)	
#define SAVE(N)		use_int((int)r##N);
PARALLEL_BENCHMARKS(int64_div)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		r##N = (s##N % r##N) ^ r##N;
#define DECLARE(N)	register int64 r##N, s##N;
#define INIT(N)		r##N = (int64)state->int_data[N]; s##N = 0;
#define PREAMBLE(N)	s##N++;	
#define SAVE(N)		use_int((int)r##N);
PARALLEL_BENCHMARKS(int64_mod)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		r##N += r##N;
#define DECLARE(N)	register float r##N, s##N;
#define INIT(N)		r##N = (float)state->double_data[N] + 1023.0; \
			s##N = (float)state->K;
#define PREAMBLE(N)	r##N += s##N;
#define SAVE(N)		use_int((int)r##N);
PARALLEL_BENCHMARKS(float_add)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		r##N *= r##N; r##N *= s##N;
#define DECLARE(N)	register float r##N, s##N;
#define INIT(N)		r##N = 8.0f * (float)state->double_data[N]; \
			s##N = 0.125 * (float)state->M * state->double_data[N] / 1000.0;
#define PREAMBLE(N)
#define SAVE(N)		use_int((int)r##N); use_int((int)s##N);
PARALLEL_BENCHMARKS(float_mul)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		r##N = s##N / r##N;
#define DECLARE(N)	register float r##N, s##N;
#define INIT(N)		r##N = 1.41421356f * (float)state->double_data[N]; \
			s##N = 3.14159265f * (float)(state->int_data[N] - N + 1);
#define PREAMBLE(N)
#define SAVE(N)		use_int((int)r##N); use_int((int)s##N);
PARALLEL_BENCHMARKS(float_div)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		r##N += r##N;
#define DECLARE(N)	register double r##N, s##N;
#define INIT(N)		r##N = state->double_data[N] + 1023.; \
			s##N = (double)state->K;
#define PREAMBLE(N)	r##N += s##N;
#define SAVE(N)		use_int((int)r##N);
PARALLEL_BENCHMARKS(double_add)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		r##N *= r##N; r##N *= s##N;
#define DECLARE(N)	register double r##N, s##N;
#define INIT(N)		r##N = 8.0f * state->double_data[N]; \
			s##N = 0.125 * (double)state->M * state->double_data[N] / 1000.0;
#define PREAMBLE(N)	
#define SAVE(N)		use_int((int)r##N); use_int((int)s##N);
PARALLEL_BENCHMARKS(double_mul)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE

#define BODY(N)		r##N = s##N / r##N;
#define DECLARE(N)	register double r##N, s##N;
#define INIT(N)		r##N = 1.41421356 * state->double_data[N]; \
			s##N = 3.14159265 * (double)(state->int_data[N] - N + 1);
#define PREAMBLE(N)	
#define SAVE(N)		use_int((int)r##N); use_int((int)s##N);
PARALLEL_BENCHMARKS(double_div)
#undef	BODY
#undef	DECLARE
#undef	INIT
#undef	PREAMBLE
#undef	SAVE


static void
initialize(iter_t iterations, void* cookie)
{
	struct _state *state = (struct _state*)cookie;
	register int i;

	if (iterations) return;

	state->int_data = (int*)malloc(MAX_LOAD_PARALLELISM * sizeof(int));
	state->double_data = (double*)malloc(MAX_LOAD_PARALLELISM * sizeof(double));

	for (i = 0; i < MAX_LOAD_PARALLELISM; ++i) {
		state->int_data[i] = i+1;
		state->double_data[i] = 1.;
	}
}

static void
cleanup(iter_t iterations, void* cookie)
{
	struct _state *state = (struct _state*)cookie;

	if (iterations) return;

	free(state->int_data);
	free(state->double_data);
}


int
par_ops_main(int ac, char **av)
{
	int	c;
	int	warmup = 0;
	int	repetitions = TRIES;
	double	par;
	struct _state	state;
	char   *usage = "[-W <warmup>] [-N <repetitions>]\n";

	state.N = 1;
	state.M = 1000;
	state.K = -1023;

	while (( c = getopt(ac, av, "W:N:")) != EOF) {
		switch(c) {
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

	par = max_parallelism(integer_bit_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "integer bit parallelism: %.2f\n", par);

	par = max_parallelism(integer_add_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "integer add parallelism: %.2f\n", par);

	par = max_parallelism(integer_mul_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "integer mul parallelism: %.2f\n", par);

	par = max_parallelism(integer_div_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "integer div parallelism: %.2f\n", par);

	par = max_parallelism(integer_mod_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "integer mod parallelism: %.2f\n", par);

	par = max_parallelism(int64_bit_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "int64 bit parallelism: %.2f\n", par);

	par = max_parallelism(int64_add_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "int64 add parallelism: %.2f\n", par);

	par = max_parallelism(int64_mul_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "int64 mul parallelism: %.2f\n", par);

	par = max_parallelism(int64_div_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "int64 div parallelism: %.2f\n", par);

	par = max_parallelism(int64_mod_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "int64 mod parallelism: %.2f\n", par);

	par = max_parallelism(float_add_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "float add parallelism: %.2f\n", par);

	par = max_parallelism(float_mul_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "float mul parallelism: %.2f\n", par);

	par = max_parallelism(float_div_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "float div parallelism: %.2f\n", par);

	par = max_parallelism(double_add_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "double add parallelism: %.2f\n", par);

	par = max_parallelism(double_mul_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "double mul parallelism: %.2f\n", par);

	par = max_parallelism(double_div_benchmarks, 
			      warmup, repetitions, &state);
	if (par > 0.)
		fprintf(stderr, "double div parallelism: %.2f\n", par);


	return(0);
}


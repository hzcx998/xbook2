#ifndef LMBENCH_MEM_H
#define LMBENCH_MEM_H


#define MAX_MEM_PARALLELISM 16
#define MEM_BENCHMARK_DECL(N) \
	void mem_benchmark_##N(iter_t iterations, void* cookie);

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

struct mem_state {
	char*	addr;	/* raw pointer returned by malloc */
	char*	base;	/* page-aligned pointer */
	char*	p[MAX_MEM_PARALLELISM];
	int	initialized;
	int	width;
	size_t	len;
	size_t	maxlen;
	size_t	line;
	size_t	pagesize;
	size_t	nlines;
	size_t	npages;
	size_t	nwords;
	size_t*	pages;
	size_t*	lines;
	size_t*	words;
};

void stride_initialize(iter_t iterations, void* cookie);
void thrash_initialize(iter_t iterations, void* cookie);
void mem_initialize(iter_t iterations, void* cookie);
void line_initialize(iter_t iterations, void* cookie);
void tlb_initialize(iter_t iterations, void* cookie);
void mem_cleanup(iter_t iterations, void* cookie);
void tlb_cleanup(iter_t iterations, void* cookie);

REPEAT_15(MEM_BENCHMARK_DECL)
extern benchmp_f mem_benchmarks[];

size_t	line_find(size_t l, int warmup, int repetitions, struct mem_state* state);
double	line_test(size_t l, int warmup, int repetitions, struct mem_state* state);
double	par_mem(size_t l, int warmup, int repetitions, struct mem_state* state);

#endif /* LMBENCH_MEM_H */


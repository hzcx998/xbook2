/*
 * lib_mem.c - library of routines used to analyze the memory hierarchy
 *
 * @(#)lib_mem.c 1.15 staelin@hpliclu2.hpli.hpl.hp.com
 *
 * Copyright (c) 2000 Carl Staelin.
 * Copyright (c) 1994 Larry McVoy.  
 * Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */

#include "bench.h"

#define	FIVE(m)		m m m m m
#define	TEN(m)		FIVE(m) FIVE(m)
#define	FIFTY(m)	TEN(m) TEN(m) TEN(m) TEN(m) TEN(m)
#define	HUNDRED(m)	FIFTY(m) FIFTY(m)

#define DEREF(N)	p##N = (char**)*p##N;
#define DECLARE(N)	static char **sp##N; register char **p##N;
#define INIT(N)		p##N = (mem_benchmark_rerun && addr_save==state->addr) ? sp##N : (char**)state->p[N];
#define SAVE(N)		sp##N = p##N;

#define MEM_BENCHMARK_F(N) mem_benchmark_##N,
benchmp_f mem_benchmarks[] = {REPEAT_15(MEM_BENCHMARK_F)};

static int mem_benchmark_rerun = 0;

#define MEM_BENCHMARK_DEF(N,repeat,body) 				\
void									\
mem_benchmark_##N(iter_t iterations, void *cookie)			\
{									\
	struct mem_state* state = (struct mem_state*)cookie;		\
	static char *addr_save = NULL;					\
	repeat(DECLARE);						\
									\
	repeat(INIT);							\
	while (iterations-- > 0) {					\
		HUNDRED(repeat(body));					\
	}								\
									\
	repeat(SAVE);							\
	addr_save = state->addr;					\
	mem_benchmark_rerun = 1;					\
}

MEM_BENCHMARK_DEF(0, REPEAT_0, DEREF)
MEM_BENCHMARK_DEF(1, REPEAT_1, DEREF)
MEM_BENCHMARK_DEF(2, REPEAT_2, DEREF)
MEM_BENCHMARK_DEF(3, REPEAT_3, DEREF)
MEM_BENCHMARK_DEF(4, REPEAT_4, DEREF)
MEM_BENCHMARK_DEF(5, REPEAT_5, DEREF)
MEM_BENCHMARK_DEF(6, REPEAT_6, DEREF)
MEM_BENCHMARK_DEF(7, REPEAT_7, DEREF)
MEM_BENCHMARK_DEF(8, REPEAT_8, DEREF)
MEM_BENCHMARK_DEF(9, REPEAT_9, DEREF)
MEM_BENCHMARK_DEF(10, REPEAT_10, DEREF)
MEM_BENCHMARK_DEF(11, REPEAT_11, DEREF)
MEM_BENCHMARK_DEF(12, REPEAT_12, DEREF)
MEM_BENCHMARK_DEF(13, REPEAT_13, DEREF)
MEM_BENCHMARK_DEF(14, REPEAT_14, DEREF)
MEM_BENCHMARK_DEF(15, REPEAT_15, DEREF)


size_t*	words_initialize(size_t max, int scale);


void
mem_reset()
{
	mem_benchmark_rerun = 0;
}

void
mem_cleanup(iter_t iterations, void* cookie)
{
	struct mem_state* state = (struct mem_state*)cookie;

	if (iterations) return;

	if (state->addr) {
		free(state->addr);
		state->addr = NULL;
	}
	if (state->lines) {
		free(state->lines);
		state->lines = NULL;
	}
	if (state->pages) {
		free(state->pages);
		state->pages = NULL;
	}
	if (state->words) {
		free(state->words);
		state->words = NULL;
	}
}

void
tlb_cleanup(iter_t iterations, void* cookie)
{
	size_t i;
	struct mem_state* state = (struct mem_state*)cookie;
	char **addr = (char**)state->addr;

	if (iterations) return;

	if (addr) {
		for (i = 0; i < state->npages; ++i) {
			if (addr[i]) free(addr[i]);
		}
		free(addr);
		state->addr = NULL;
	}
	if (state->pages) {
		free(state->pages);
		state->pages = NULL;
	}
	if (state->lines) {
		free(state->lines);
		state->lines = NULL;
	}
}

void
base_initialize(iter_t iterations, void* cookie)
{
	size_t	nwords, nlines, nbytes, npages, nmpages;
	size_t *pages;
	size_t *lines;
	size_t *words;
	struct mem_state* state = (struct mem_state*)cookie;
	register char *p = 0 /* lint */;

	if (iterations) return;

	state->initialized = 0;

	nbytes = state->len;
	nwords = state->line / sizeof(char*);
	nlines = state->pagesize / state->line;
	npages = (nbytes + state->pagesize - 1) / state->pagesize;
	nmpages= (state->maxlen + state->pagesize - 1) / state->pagesize;

	srand(getpid());

	words = NULL;
	lines = NULL;
	pages = permutation(nmpages, state->pagesize);
	p = state->addr = (char*)malloc(state->maxlen + 2 * state->pagesize);

	if (p == NULL) {
            printf(" memory allocation failure, current memory size used for\
testing is %llu bytes, please choose smaller memory size for testing\n ", state->maxlen);
            return;
	}

	state->nwords = nwords;
	state->nlines = nlines;
	state->npages = npages;
	state->lines = lines;
	state->pages = pages;
	state->words = words;

	if (state->addr == NULL || pages == NULL)
		return;

	if ((unsigned long)p % state->pagesize) {
		p += state->pagesize - (unsigned long)p % state->pagesize;
	}
	state->base = p;
	state->initialized = 1;
	mem_reset();
}

/*
 * Create a circular list of pointers using a simple striding
 * algorithm.  
 * 
 * This access pattern corresponds to many array/matrix
 * algorithms.  It should be easily and correctly predicted
 * by any decent hardware prefetch algorithm.
 */
void
stride_initialize(iter_t iterations, void* cookie)
{
	struct mem_state* state = (struct mem_state*)cookie;
	size_t	i;
	size_t	range = state->len;
	size_t	stride = state->line;
	char*	addr;

	base_initialize(iterations, cookie);
	if (!state->initialized) return;
	addr = state->base;

	for (i = stride; i < range; i += stride) {
		*(char **)&addr[i - stride] = (char*)&addr[i];
	}
	*(char **)&addr[i - stride] = (char*)&addr[0];
	state->p[0] = addr;
	mem_reset();
}

void
thrash_initialize(iter_t iterations, void* cookie)
{
	struct mem_state* state = (struct mem_state*)cookie;
	size_t	i;
	size_t	j;
	size_t	cur;
	size_t	next;
	size_t	cpage;
	size_t	npage;
	char*	addr;

	base_initialize(iterations, cookie);
	if (!state->initialized) return;
	addr = state->base;

	/*
	 * Create a circular list of pointers with a random access
	 * pattern.
	 *
	 * This stream corresponds more closely to linked list
	 * memory access patterns.  For large data structures each
	 * access will likely cause both a cache miss and a TLB miss.
	 * 
	 * Access a different page each time.  This will eventually
	 * cause a tlb miss each page.  It will also cause maximal
	 * thrashing in the cache between the user data stream and
	 * the page table entries.
	 */
	if (state->len % state->pagesize) {
		state->nwords = state->len / state->line;
		state->words = words_initialize(state->nwords, state->line);
		for (i = 0; i < state->nwords - 1; ++i) {
			*(char **)&addr[state->words[i]] = (char*)&addr[state->words[i+1]];
		}
		*(char **)&addr[state->words[i]] = addr;
		state->p[0] = addr;
	} else {
		state->nwords = state->pagesize / state->line;
		state->words = words_initialize(state->nwords, state->line);

		for (i = 0; i < state->npages - 1; ++i) {
			cpage = state->pages[i];
			npage = state->pages[i + 1];
			for (j = 0; j < state->nwords; ++j) {
				cur = cpage + state->words[(i + j) % state->nwords];
				next = npage + state->words[(i + j + 1) % state->nwords];
				*(char **)&addr[cur] = (char*)&addr[next];
			}
		}
		cpage = state->pages[i];
		npage = state->pages[0];
		for (j = 0; j < state->nwords; ++j) {
			cur = cpage + state->words[(i + j) % state->nwords];
			next = npage + state->words[(j + 1) % state->nwords];
			*(char **)&addr[cur] = (char*)&addr[next];
		}
		state->p[0] = (char*)&addr[state->pages[0]];
	}
	mem_reset();
}

/*
 * mem_initialize
 *
 * Create a circular pointer chain that runs through memory.
 *
 * The chain threads through each cache line on a page before
 * moving to the next page.  The order of cache line accesses
 * is randomized to defeat cache prefetching algorithms.  In
 * addition, the order of page accesses is randomized.  Finally,
 * to reduce the impact of incorrect line-size estimates on
 * machines with direct-mapped caches, we randomize which 
 * word in the cache line is used to hold the pointer.
 *
 * It initializes state->width pointers to elements evenly
 * spaced through the chain.
 */
void
mem_initialize(iter_t iterations, void* cookie)
{
	int i, j, k, l, np, nw, nwords, nlines, nbytes, npages, npointers;
	unsigned int r;
	size_t    *pages;
	size_t    *lines;
	size_t    *words;
	struct mem_state* state = (struct mem_state*)cookie;
	register char *p = 0 /* lint */;

	if (iterations) return;

	base_initialize(iterations, cookie);
	if (!state->initialized) return;
	state->initialized = 0;

	npointers = state->len / state->line;
	nwords = state->nwords;
	nlines = state->nlines;
	npages = state->npages;
	words = state->words = words_initialize(nwords, sizeof(char*));
	lines = state->lines = words_initialize(nlines, state->line);
	pages = state->pages;
	p = state->base;

	if (state->addr == NULL \
	    || pages == NULL || lines == NULL || words == NULL) {
		return;
	}

	/* setup the run through the pages */
	l = 0;
	for (i = 0; i < npages; ++i) {
		for (j = 0; j < nlines - 1 && l < npointers - 1; ++j, ++l) {
			for (k = 0; k < state->line; k += sizeof(char*)) {
				*(char**)(p + pages[i] + lines[j] + k) =
					p + pages[i] + lines[j+1] + k;
			}
			if (l % (npointers/state->width) == 0
			    && l / (npointers/state->width) < MAX_MEM_PARALLELISM) {
				k = l / (npointers/state->width);
				state->p[k] = p + pages[i] + lines[j] + words[k % nwords];
			}
		}

		if (i < npages - 1) {
			for (k = 0; k < nwords; ++k) 
				*(char**)(p + pages[i] + lines[j] + words[k]) =
					p + pages[i+1] + lines[0] + words[k];
		}
	}
	for (k = 0; k < nwords; ++k) {
		nw = (k == nwords - 1) ? 0 : k + 1;
		*(char**)(p + pages[npages-1] + lines[j] + words[k]) =
			p + pages[0] + lines[0] + words[nw];
	}

	/* now, run through the chain once to clear the cache */
	mem_reset();
	(*mem_benchmarks[state->width-1])((nwords * npointers + 100) / 100, state);

	state->initialized = 1;
}

/*
 * line_initialize
 *
 * This is very similar to mem_initialize, except that we always use
 * the first element of the cache line to hold the pointer.
 *
 */
void
line_initialize(iter_t iterations, void* cookie)
{
	int i, j, k, line, nlines, npages;
	unsigned int r;
	size_t    *pages;
	size_t    *lines;
	struct mem_state* state = (struct mem_state*)cookie;
	register char *p = 0 /* lint */;

	if (iterations) return;

	base_initialize(iterations, cookie);
	if (!state->initialized) return;
	state->initialized = 0;

	nlines = state->nlines;
	npages = state->npages;
	lines = state->lines = words_initialize(nlines, state->line);
	pages = state->pages;
	p = state->base;

	state->width = 1;
	
	if (state->addr == NULL || lines == NULL || pages == NULL)
		return;

	/* new setup runs through the lines */
	for (i = 0; i < npages; ++i) {
		/* sequence through the first word of each line */
		for (j = 0; j < nlines - 1; ++j) {
			*(char**)(p + pages[i] + lines[j]) = 
				p + pages[i] + lines[j+1];
		}

		/* jump to the fist word of the first line on next page */
		*(char**)(p + pages[i] + lines[j]) = 
			p + pages[(i < npages-1) ? i+1 : 0] + lines[0];
	}
	state->p[0] = p + pages[0] + lines[0];

	/* now, run through the chain once to clear the cache */
	mem_reset();
	mem_benchmark_0((nlines * npages + 100) / 100, state);

	state->initialized = 1;
}

/*
 * tlb_initialize
 *
 * Build a pointer chain which accesses one word per page, for a total
 * of (line * pages) bytes of data loaded into cache.  
 *
 * If the number of elements in the chain (== #pages) is larger than the
 * number of pages addressed by the TLB, then each access should cause
 * a TLB miss (certainly as the number of pages becomes much larger than
 * the TLB-addressed space).
 *
 * In addition, if we arrange the chain properly, each word we access
 * will be in the cache.
 *
 * This means that the average access time for each pointer dereference
 * should be a cache hit plus a TLB miss.
 *
 */
void
tlb_initialize(iter_t iterations, void* cookie)
{
	int i, j, nwords, nlines, npages, pagesize;
	unsigned int r;
	char **pages = NULL;
	char **addr = NULL;
	size_t    *lines = NULL;
	struct mem_state* state = (struct mem_state*)cookie;
	register char *p = 0 /* lint */;

	if (iterations) return;

	state->initialized = 0;

	pagesize = state->pagesize;
	nwords   = 0;
	nlines   = pagesize / sizeof(char*);
	npages   = state->len / pagesize;

	srand(getpid() ^ (getppid()<<7));

	lines = words_initialize(nlines, sizeof(char*));
	pages = (char**)malloc(npages * sizeof(char**));
	addr = (char**)malloc(npages * sizeof(char**));

	state->nwords = 1;
	state->nlines = nlines;
	state->npages = npages;
	state->words = NULL;
	state->lines = lines;
	state->pages = (size_t*)pages;
	state->addr = (char*)addr;
	if (addr) bzero(addr, npages * sizeof(char**));
	if (pages) bzero(pages, npages * sizeof(char**));

	if (addr == NULL || pages == NULL || lines == NULL) {
		return;
	}

	/* first, layout the sequence of page accesses */
	for (i = 0; i < npages; ++i) {
		p = addr[i] = (char*)valloc(pagesize);
		if (p == NULL) return;
		if ((unsigned long)p % pagesize) {
			free(p);
			p = addr[i] = (char*)valloc(2 * pagesize);
			if (p == NULL) return;
			p += pagesize - (unsigned long)p % pagesize;
		}
		pages[i] = (char*)p;
	}

	/* randomize the page sequences (except for zeroth page) */
	r = (rand() << 15) ^ rand();
	for (i = npages - 2; i > 0; --i) {
		char* l;
		r = (r << 1) ^ (rand() >> 4);
		l = pages[(r % i) + 1];
		pages[(r % i) + 1] = pages[i + 1];
		pages[i + 1] = l;
	}

	/* now setup run through the pages */
	for (i = 0; i < npages - 1; ++i) {
		*(char**)(pages[i] + lines[i%nlines]) = 
			pages[i+1] + lines[(i+1)%nlines];
	}
	*(char**)(pages[i] + lines[i%nlines]) = pages[0] + lines[0];
	state->p[0] = pages[0] + lines[0];

	/* run through the chain once to clear the cache */
	mem_reset();
	mem_benchmark_0((npages + 100) / 100, state);

	state->initialized = 1;
}

/*
 * words_initialize
 *
 * This is supposed to create the order in which the words in a 
 * "cache line" are used.  Since we rarely know the cache line
 * size with any real reliability, we need to jump around so
 * as to maximize the number of potential cache misses, and to
 * minimize the possibility of re-using a cache line.
 */
size_t*
words_initialize(size_t max, int scale)
{
	size_t	i, j, nbits;
	size_t*	words = (size_t*)malloc(max * sizeof(size_t));

	if (!words) return NULL;

	bzero(words, max * sizeof(size_t));
	for (i = max>>1, nbits = 0; i != 0; i >>= 1, nbits++)
		;
	for (i = 0; i < max; ++i) {
		/* now reverse the bits */
		for (j = 0; j < nbits; j++) {
			if (i & (1<<j)) {
				words[i] |= (1<<(nbits-j-1));
			}
		}
		words[i] *= scale;
	}
	return words;
}


size_t
line_find(size_t len, int warmup, int repetitions, struct mem_state* state)
{
	size_t 	i, j, big_jump, line;
	size_t	maxline = getpagesize() / 16;
	double	baseline, t;

	big_jump = 0;
	line = 0;

	/*
	fprintf(stderr, "line_find(%d, ...): entering\n", len);
	/**/

	state->width = 1;
	state->line = sizeof(char*);
	for (state->addr = NULL; !state->addr && len; ) {
		state->len = state->maxlen = len;
		line_initialize(0, state);
		if (state->addr == NULL) len >>= 1;
	}
	if (state->addr == NULL) return -1;

	for (i = sizeof(char*); i <= maxline; i<<=1) {
		t = line_test(i, warmup, repetitions, state);

		if (t == 0.) break;

		if (i > sizeof(char*)) {
			if (t > 1.3 * baseline) {
				big_jump = 1;
			} else if (big_jump && t < 1.15 * baseline) {
				line = (i>>1);
				break;
			}
		}
		baseline = t;
	}
	mem_cleanup(0, state);
	/*
	fprintf(stderr, "line_find(%d, ...): returning %d\n", len, line);
	/**/
	return line;
}

double
line_test(size_t line, int warmup, int repetitions, struct mem_state* state)
{
	size_t	i;
	size_t	npages = state->npages;
	size_t	nlines = state->pagesize / line;
	double	t;
	char*	p = state->base;
	char*	first = p + state->pages[0] + state->lines[0];
	char*	last = p + state->pages[npages-1] + state->lines[nlines-1];
	result_t *r, *r_save;


	/* only visit a subset of the lines in each page */
	if (nlines < state->nlines) {
		p = state->base;
		for (i = 0; i < npages - 1; ++i) {
			*(char**)(p + state->pages[i] + state->lines[nlines-1]) =
				p + state->pages[i+1] + state->lines[0];
		}
		*(char**)(p + state->pages[npages-1] + state->lines[nlines-1]) =
			p + state->pages[0] + state->lines[0];
	}

	r_save = get_results();
	r = (result_t*)malloc(sizeof_result(repetitions));
	insertinit(r);
	p = first;
	for (i = 0; i < repetitions; ++i) {
		BENCH1(HUNDRED(p = *(char**)p;),0);
		/*
		fprintf(stderr, "%d\t%d\t%d\n", line, (int)gettime(), (int)get_n()); 
		/**/
		insertsort(gettime(), get_n(), r);
	}
	use_pointer(p);
	set_results(r);
	t = 10. * (double)gettime() / (double)get_n();
	set_results(r_save);
	free(r);
	
	/*
	fprintf(stderr, "%d\t%.5f\t%d\n", line, t, state->len); 
	/**/

	/* fixup full path again */
	if (nlines < state->nlines) {
		p = state->base;
		for (i = 0; i < npages - 1; ++i) {
			*(char**)(p + 
				  state->pages[i] + 
				  state->lines[nlines-1]) =
				p + 
				state->pages[i] + 
				state->lines[nlines];
		}
		*(char**)(p + 
			  state->pages[npages-1] + 
			  state->lines[nlines-1]) =
			p + 
			state->pages[npages-1] + 
			state->lines[nlines];
	}

	return (t);
}

double
par_mem(size_t len, int warmup, int repetitions, struct mem_state* state)
{
	int	i, j, k, n, __n;
	double	baseline, max_par, par;

	state->width = 1;
	max_par = 1.;
	__n = 1;

	for (state->addr = NULL; !state->addr && len; ) {
		state->len = state->maxlen = len;
		mem_initialize(0, state);
		if (state->addr == NULL) len >>= 1;
	}
	if (state->addr == NULL) return -1.;

	for (i = 0; i < MAX_MEM_PARALLELISM; ++i) {
		n = len / state->line;
		for (j = 0; j <= i; j++) {
			size_t nlines = len / state->line;
			size_t lines_per_chunk = nlines / (i + 1);
			size_t lines_per_page = state->pagesize / state->line;
			size_t words_per_chunk = state->nwords / (i + 1);
			size_t line = j * lines_per_chunk;
			size_t word = (j * state->nwords) / (i + 1);

			/*
			if (state->len == 32768 && i == 7) {
				fprintf(stderr, "\tj=%d, line=%d, word=%d, page=%d, _line=%d, _word=%d\n", j, line, word, line / lines_per_page, line % lines_per_page, word % state->nwords);
			}
			/**/
			state->p[j] = state->base + 
				state->pages[line / lines_per_page] + 
				state->lines[line % lines_per_page] + 
				state->words[word % state->nwords];
		}
		mem_reset();
		(*mem_benchmarks[i])((len / sizeof(char*) + 100) / 100, state);
		BENCH((*mem_benchmarks[i])(__n, state); __n = 1;, 0);
		if (i == 0) {
			baseline = (double)gettime() / (double)get_n();
		} else if (gettime() > 0) {
			par = baseline;
			par /= (double)gettime() / (double)((i + 1) * get_n());
			/*
			fprintf(stderr, "par_mem(%d): i=%d, p=%5.2f, l=%d, lpp=%d, lpc=%d, nl=%d, wpc=%d\n", len, i, par, state->line, state->pagesize / state->line, (len / state->line) / (i + 1), len / state->line, state->nwords / (i + 1));
			/**/
			if (par > max_par) {
				max_par = par;
			}
		}
	}
	mem_cleanup(0, state);

	return max_par;
}



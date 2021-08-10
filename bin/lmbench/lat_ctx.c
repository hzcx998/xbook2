/*
 * lat_ctx.c - context switch timer 
 *
 * usage: lat_ctx [-P parallelism] [-W <warmup>] [-N <repetitions>] [-s size] #procs [#procs....]
 *
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char	*id = "$Id$\n";

#include "bench.h"


#define	MAXPROC	2048
#define	CHUNK	(4<<10)
#define	TRIPS	5
#ifndef	max
#define	max(a, b)	((a) > (b) ? (a) : (b))
#endif

static void	doit(int rd, int wr, int process_size);
int	create_pipes(int **p, int procs);
int	create_daemons(int **p, pid_t *pids, int procs, int process_size);
static void	initialize_overhead(iter_t iterations, void* cookie);
static void	cleanup_overhead(iter_t iterations, void* cookie);
static void	benchmark_overhead(iter_t iterations, void* cookie);
static void	initialize(iter_t iterations, void* cookie);
static void	cleanup(iter_t iterations, void* cookie);
static void	benchmark(iter_t iterations, void* cookie);

struct _state {
	int	process_size;
	double	overhead;
	int	procs;
	pid_t*	pids;
	int	**p;
	void*	data;
};

int
lat_ctx_main(int ac, char **av)
{
	int	i, maxprocs;
	int	c;
	int	parallel = 1;
	int	warmup = 0;
	int	repetitions = TRIES;
	struct _state state;
	char *usage = "[-P <parallelism>] [-W <warmup>] [-N <repetitions>] [-s kbytes] processes [processes ...]\n";
	double	time;

	/*
	 * Need 4 byte ints.
	 */
	if (sizeof(int) != 4) {
		fprintf(stderr, "Fix sumit() in ctx.c.\n");
		exit(1);
	}

	state.process_size = 0;
	state.overhead = 0.0;
	state.pids = NULL;

	/*
	 * If they specified a context size, or parallelism level, get them.
	 */
	while (( c = getopt(ac, av, "s:P:W:N:")) != EOF) {
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
		case 's':
			state.process_size = atoi(optarg) * 1024;
			break;
		default:
			lmbench_usage(ac, av, usage);
			break;
		}
	}

	if (optind > ac - 1)
		lmbench_usage(ac, av, usage);

	/* compute pipe + sumit overhead */
	maxprocs = atoi(av[optind]);
	for (i = optind; i < ac; ++i) {
		state.procs = atoi(av[i]);
		if (state.procs > maxprocs)
			maxprocs = state.procs;
	}
	state.procs = maxprocs;
	benchmp(initialize_overhead, benchmark_overhead, cleanup_overhead, 
		0, 1, warmup, repetitions, &state);
	if (gettime() == 0) return(0);
	state.overhead = gettime();
	state.overhead /= get_n();
	fprintf(stderr, "\n\"size=%dk ovr=%.2f\n", 
		state.process_size/1024, state.overhead);

	/* compute the context switch cost for N processes */
	for (i = optind; i < ac; ++i) {
		state.procs = atoi(av[i]);
		benchmp(initialize, benchmark, cleanup, 0, parallel, 
			warmup, repetitions, &state);

		time = gettime();
		time /= get_n();
		time /= state.procs;
		time -= state.overhead;

		if (time > 0.0)
			fprintf(stderr, "%d %.2f\n", state.procs, time);
	}

	return (0);
}

static void
initialize_overhead(iter_t iterations, void* cookie)
{
	int i;
	int procs;
	int* p;
	struct _state* pState = (struct _state*)cookie;

	if (iterations) return;

	pState->pids = NULL;
	pState->p = (int**)malloc(pState->procs * (sizeof(int*) + 2 * sizeof(int)));
	p = (int*)&pState->p[pState->procs];
	for (i = 0; i < pState->procs; ++i) {
		pState->p[i] = p;
		p += 2;
	}

	pState->data = (pState->process_size > 0) ? malloc(pState->process_size) : NULL;
	if (pState->data)
		bzero(pState->data, pState->process_size);

	procs = create_pipes(pState->p, pState->procs);
	if (procs < pState->procs) {
		cleanup_overhead(0, cookie);
		exit(1);
	}
}

static void
cleanup_overhead(iter_t iterations, void* cookie)
{
	int i;
	struct _state* pState = (struct _state*)cookie;

	if (iterations) return;

     	for (i = 0; i < pState->procs; ++i) {
		close(pState->p[i][0]);
		close(pState->p[i][1]);
	}

	free(pState->p);
	if (pState->data) free(pState->data);
}

static void
benchmark_overhead(iter_t iterations, void* cookie)
{
	struct _state* pState = (struct _state*)cookie;
	int	i = 0;
	int	msg = 1;

	while (iterations-- > 0) {
		if (write(pState->p[i][1], &msg, sizeof(msg)) != sizeof(msg)) {
			/* perror("read/write on pipe"); */
			exit(1);				
		}
		if (read(pState->p[i][0], &msg, sizeof(msg)) != sizeof(msg)) {
			/* perror("read/write on pipe"); */
			exit(1);
		}
		if (++i == pState->procs) {
			i = 0;
		}
		bread(pState->data, pState->process_size);
	}
}

static void
initialize(iter_t iterations, void* cookie)
{
	int procs;
	struct _state* pState = (struct _state*)cookie;

	if (iterations) return;

	initialize_overhead(iterations, cookie);

	pState->pids = (pid_t*)malloc(pState->procs * sizeof(pid_t));
	if (pState->pids == NULL)
		exit(1);
	bzero((void*)pState->pids, pState->procs * sizeof(pid_t));
	procs = create_daemons(pState->p, pState->pids, 
			       pState->procs, pState->process_size);
	if (procs < pState->procs) {
		cleanup(0, cookie);
		exit(1);
	}
}

static void
cleanup(iter_t iterations, void* cookie)
{
	int i;
	struct _state* pState = (struct _state*)cookie;

	if (iterations) return;

	/*
	 * Close the pipes and kill the children.
	 */
	cleanup_overhead(iterations, cookie);
     	for (i = 1; pState->pids && i < pState->procs; ++i) {
		if (pState->pids[i] > 0) {
			kill(pState->pids[i], SIGKILL);
			waitpid(pState->pids[i], NULL, 0);
		}
	}
	if (pState->pids)
		free(pState->pids);
	pState->pids = NULL;
}

static void
benchmark(iter_t iterations, void* cookie)
{
	struct _state* pState = (struct _state*)cookie;
	int	msg;

	/*
	 * Main process - all others should be ready to roll, time the
	 * loop.
	 */
	while (iterations-- > 0) {
		if (write(pState->p[0][1], &msg, sizeof(msg)) !=
		    sizeof(msg)) {
			/* perror("read/write on pipe"); */
			exit(1);
		}
		if (read(pState->p[pState->procs-1][0], &msg, sizeof(msg)) != sizeof(msg)) {
			/* perror("read/write on pipe"); */
			exit(1);
		}
		bread(pState->data, pState->process_size);
	}
}


static void
doit(int rd, int wr, int process_size)
{
	int	msg;
	void*	data = NULL;

	if (process_size) {
		data = malloc(process_size);
		if (data) bzero(data, process_size);
	}
	for ( ;; ) {
		if (read(rd, &msg, sizeof(msg)) != sizeof(msg)) {
			/* perror("read/write on pipe"); */
			break;
		}
		bread(data, process_size);
		if (write(wr, &msg, sizeof(msg)) != sizeof(msg)) {
			/* perror("read/write on pipe"); */
			break;
		}
	}
	exit(1);
}


int
create_daemons(int **p, pid_t *pids, int procs, int process_size)
{
	int	i, j;
	int	msg;

	/*
	 * Use the pipes as a ring, and fork off a bunch of processes
	 * to pass the byte through their part of the ring.
	 *
	 * Do the sum in each process and get that time before moving on.
	 */
	handle_scheduler(benchmp_childid(), 0, procs-1);
     	for (i = 1; i < procs; ++i) {
		switch (pids[i] = fork()) {
		    case -1:	/* could not fork, out of processes? */
			return i;

		    case 0:	/* child */
			handle_scheduler(benchmp_childid(), i, procs-1);
			for (j = 0; j < procs; ++j) {
				if (j != i - 1) close(p[j][0]);
				if (j != i) close(p[j][1]);
			}
			doit(p[i-1][0], p[i][1], process_size);
			/* NOTREACHED */

		    default:	/* parent */
			;
	    	}
	}

	/*
	 * Go once around the loop to make sure that everyone is ready and
	 * to get the token in the pipeline.
	 */
	if (write(p[0][1], &msg, sizeof(msg)) != sizeof(msg) ||
	    read(p[procs-1][0], &msg, sizeof(msg)) != sizeof(msg)) {
		/* perror("write/read/write on pipe"); */
		exit(1);
	}
	return procs;
}

int
create_pipes(int **p, int procs)
{
	int	i;
	/*
	 * Get a bunch of pipes.
	 */
	morefds();
     	for (i = 0; i < procs; ++i) {
		if (pipe(p[i]) == -1) {
			return i;
		}
	}
	return procs;
}

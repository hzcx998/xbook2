/*
 * lat_pmake.c - time to complete N jobs which each do usecs worth of work
 *
 * usage: lat_pipe [-P <parallelism>] [-W <warmup>] [-N <repetitions>] jobs usecs
 *
 * Copyright (c) 1994 Larry McVoy.  
 * Copyright (c) 2002 Carl Staelin. Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char	*id = "$Id$\n";

#include "bench.h"

static void setup(iter_t iterations, void* cookie);
static void bench(iter_t iterations, void *cookie);
static void cleanup(iter_t iterations, void *cookie);
static void work(iter_t iterations, void *cookie);

typedef struct _state {
	int	jobs;		/* number of jobs to create */
	iter_t	iterations;	/* how long each job should work */
	long*	x;		/* used by work() */
	long**	p;
	pid_t*	pids;
} state_t;

int 
lat_pmake_main(int ac, char **av)
{
	state_t state;
	int parallel = 1;
	int warmup = 0;
	int repetitions = TRIES;
	int c;
	double time;
	uint64	usecs;
	char buf[1024];
	char* usage = "[-P <parallelism>] [-W <warmup>] [-N <repetitions>] Njobs usecs...\n";

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
	if (ac < optind + 2) {
		lmbench_usage(ac, av, usage);
	}
	state.jobs = atoi(av[optind]);
	state.pids = NULL;
	fprintf(stderr, "\"pmake jobs=%d\n", state.jobs);
	while (++optind < ac) {
		usecs = bytes(av[optind]);
		benchmp(setup, work, NULL, 0, 1, 0, TRIES, &state);
		if (gettime() == 0) exit(1);
		state.iterations = (iter_t)((usecs * get_n()) / gettime());

		benchmp(setup, bench, NULL, 0, parallel, 
			warmup, repetitions, &state);
		time = gettime();
		time /= get_n();
		if (time > 0.0)
			fprintf(stderr, "%llu %.2f\n", usecs, time);
	}
	return (0);
}

static void
setup(iter_t iterations, void* cookie)
{
	state_t *state = (state_t *) cookie;

	if (iterations) return;

	state->x = (long*)malloc(sizeof(long*));
	*(long**)state->x = state->x;
	state->p = (long**)state->x;

	handle_scheduler(benchmp_childid(), 0, state->jobs);
}

static void
bench(register iter_t iterations, void *cookie)
{
	int	i;
	int	status;
	state_t *state = (state_t *) cookie;
	
	state->pids = (pid_t*)malloc(state->jobs * sizeof(pid_t));

	/* 
	 * This design has one buglet --- we cannot detect if the 
	 * worker process died prematurely.  I.e., we don't have
	 * a handshake step to collect "I finished correctly"
	 * messages.
	 */
	while (iterations-- > 0) {
		for (i = 0; i < state->jobs; ++i) {
			if ((state->pids[i] = fork()) == 0) {
				handle_scheduler(benchmp_childid(), i+1, state->jobs);
				work(state->iterations, state);
				exit(0);
			}
		}
		for (i = 0; i < state->jobs; ++i) {
			waitpid(state->pids[i], &status, 0);
			state->pids[i] = -1;

			/* child died badly */
			if (!WIFEXITED(status)) {
				cleanup(0, cookie);
				exit(1);
			}
		}
	}
}

static void
cleanup(register iter_t iterations, void *cookie)
{
	int	i;
	state_t *state = (state_t *) cookie;

	for (i = 0; i < state->jobs; ++i) {
		if (state->pids[i] > 0) {
			kill(state->pids[i], SIGKILL);
			waitpid(state->pids[i], NULL, 0);
			state->pids[i] = -1;
		}
	}
}

static void
work(register iter_t iterations, void* cookie)
{
	state_t *state = (state_t *) cookie;
	register long** p = state->p;

#define	WORK_TEN(one)	one one one one one one one one one one
	while (iterations-- > 0) {
		WORK_TEN(p = (long**) *p;);
	}
	state->p = p;
}

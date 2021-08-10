/*
 * lat_pipe.c - pipe transaction test
 *
 * usage: lat_pipe [-P <parallelism>] [-W <warmup>] [-N <repetitions>]
 *
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char	*id = "$Id$\n";

#include "bench.h"

static void initialize(iter_t iterations, void *cookie);
static void cleanup(iter_t iterations, void *cookie);
static void doit(iter_t iterations, void *cookie);
static void writer(int w, int r);

typedef struct _state {
	int	pid;
	int	p1[2];
	int	p2[2];
} state_t;

int 
lat_pipe_main(int ac, char **av)
{
	state_t state;
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

	state.pid = 0;

	benchmp(initialize, doit, cleanup, SHORT, parallel, 
		warmup, repetitions, &state);
	micro("Pipe latency", get_n());
	return (0);
}

static void
initialize(iter_t iterations, void* cookie)
{
	char	c;
	state_t * state = (state_t *)cookie;

	if (iterations) return;

	if (pipe(state->p1) == -1) {
		perror("pipe");
		exit(1);
	}
	if (pipe(state->p2) == -1) {
		perror("pipe");
		exit(1);
	}
	handle_scheduler(benchmp_childid(), 0, 1);
	switch (state->pid = fork()) {
	    case 0:
		handle_scheduler(benchmp_childid(), 1, 1);
		signal(SIGTERM, exit);
		close(state->p1[1]);
		close(state->p2[0]);
		writer(state->p2[1], state->p1[0]);
		return;

	    case -1:
		perror("fork");
		return;

	    default:
		close(state->p1[0]);
		close(state->p2[1]);
		break;
	}

	/*
	 * One time around to make sure both processes are started.
	 */
	if (write(state->p1[1], &c, 1) != 1 || read(state->p2[0], &c, 1) != 1){
		perror("(i) read/write on pipe");
		exit(1);
	}
}

static void
cleanup(iter_t iterations, void* cookie)
{
	state_t * state = (state_t *)cookie;

	if (iterations) return;

	if (state->pid) {
		kill(state->pid, SIGKILL);
		waitpid(state->pid, NULL, 0);
		state->pid = 0;
	}
}

static void
doit(register iter_t iterations, void *cookie)
{
	state_t *state = (state_t *) cookie;
	char		c;
	register int	w = state->p1[1];
	register int	r = state->p2[0];
	register char	*cptr = &c;

	while (iterations-- > 0) {
		if (write(w, cptr, 1) != 1 ||
		    read(r, cptr, 1) != 1) {
			perror("(r) read/write on pipe");
			exit(1);
		}
	}
}

static void
writer(register int w, register int r)
{
	char		c;
	register char	*cptr = &c;

	for ( ;; ) {
		if (read(r, cptr, 1) != 1 ||
			write(w, cptr, 1) != 1) {
			    perror("(w) read/write on pipe");
		}
	}
}

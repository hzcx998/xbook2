/*
 * lat_fifo.c - named pipe transaction test
 *
 * usage: lat_fifo [-P <parallelism>] [-W <warmup>] [-N <repetitions>]
 *
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 */
static char	*id = "$Id$\n";

#include "bench.h"

#define	F1	"/tmp/lmbench_f1.%d"
#define	F2	"/tmp/lmbench_f2.%d"

static void initialize(iter_t iterations, void *cookie);
static void cleanup(iter_t iterations, void *cookie);
static void doit(iter_t iterations, void *cookie);
static void writer(int wr, int rd);

typedef struct _state {
	char	filename1[256];
	char	filename2[256];
	int	pid;
	int	wr;
	int	rd;
} state_t;

int 
lat_fifo_main(int ac, char **av)
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
	micro("Fifo latency", get_n());
	return (0);
}

static void
initialize(iter_t iterations, void *cookie)
{
	char	c;
	state_t * state = (state_t *)cookie;

	if (iterations) return;

	state->pid = 0;
	sprintf(state->filename1,F1,getpid());
	sprintf(state->filename2,F2,getpid());
	
	unlink(state->filename1); unlink(state->filename2);
	if (mknod(state->filename1, S_IFIFO|0664, 0) ||
	    mknod(state->filename2, S_IFIFO|0664, 0)) {
		perror("mknod");
		exit(1);
	}
	handle_scheduler(benchmp_childid(), 0, 1);
	switch (state->pid = fork()) {
	    case 0:
		handle_scheduler(benchmp_childid(), 1, 1);
		state->rd = open(state->filename1, O_RDONLY);
		state->wr = open(state->filename2, O_WRONLY);
		writer(state->wr, state->rd);
		return;

	    case -1:
		perror("fork");
		return;

	    default:
		state->wr = open(state->filename1, O_WRONLY);
		state->rd = open(state->filename2, O_RDONLY);
		break;
	}

	/*
	 * One time around to make sure both processes are started.
	 */
	if (write(state->wr, &c, 1) != 1 || read(state->rd, &c, 1) != 1) {
		perror("(i) read/write on pipe");
		exit(1);
	}
}

static void
cleanup(iter_t iterations, void * cookie)
{
	state_t * state = (state_t *)cookie;

	if (iterations) return;

	unlink(state->filename1);
	unlink(state->filename2);
	close(state->wr);
	close(state->rd);

	if (state->pid > 0) {
		kill(state->pid, 15);
		waitpid(state->pid, NULL, 0);
		state->pid = 0;
	}
}

static void
doit(register iter_t iterations, void *cookie)
{
	state_t *state = (state_t *) cookie;
	char		c;
	register int	w = state->wr;
	register int	r = state->rd;
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

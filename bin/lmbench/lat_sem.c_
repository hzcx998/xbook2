/*
 * lat_sem.c - semaphore test
 *
 * usage: lat_sem [-P <parallelism>] [-W <warmup>] [-N <repetitions>]
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
#include <sys/sem.h>

static void initialize(iter_t iterations, void *cookie);
static void cleanup(iter_t iterations, void *cookie);
static void doit(iter_t iterations, void *cookie);
static void writer(int sid);

typedef struct _state {
	int	pid;
	int	semid;
} state_t;

int 
lat_sem_main(int ac, char **av)
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
	micro("Semaphore latency", get_n() * 2);
	return (0);
}

static void
initialize(iter_t iterations, void* cookie)
{
	char	c;
	state_t * state = (state_t *)cookie;

	if (iterations) return;

	state->semid = semget(IPC_PRIVATE, 2, IPC_CREAT | IPC_EXCL | 0600);
	semctl(state->semid, 0, SETVAL, 0);
	semctl(state->semid, 1, SETVAL, 0);

	handle_scheduler(benchmp_childid(), 0, 1);
	switch (state->pid = fork()) {
	    case 0:
		signal(SIGTERM, exit);
		handle_scheduler(benchmp_childid(), 1, 1);
		writer(state->semid);
		return;

	    case -1:
		perror("fork");
		return;

	    default:
		break;
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
	/* free the semaphores */
	semctl(state->semid, 0, IPC_RMID);
}

static void
doit(register iter_t iterations, void *cookie)
{
	state_t *state = (state_t *) cookie;
	struct sembuf sop[2];

	sop[0].sem_num = 1;
	sop[0].sem_op = -1;
	sop[0].sem_flg = 0;

	sop[1].sem_num = 0;
	sop[1].sem_op = 1;
	sop[1].sem_flg = 0;

	while (iterations-- > 0) {
		if (semop(state->semid, sop, 2) < 0) {
			perror("(r) error on semaphore");
			exit(1);
		}
	}
}

static void
writer(register int sid)
{
	struct sembuf sop[2];

	sop[0].sem_num = 1;
	sop[0].sem_op = 1;
	sop[0].sem_flg = 0;

	if (semop(sid, sop, 1) < 0) {
		perror("(w) error on initial semaphore");
		exit(1);
	}

	sop[0].sem_num = 0;
	sop[0].sem_op = -1;
	sop[0].sem_flg = 0;

	sop[1].sem_num = 1;
	sop[1].sem_op = 1;
	sop[1].sem_flg = 0;

	for ( ;; ) {
		if (semop(sid, sop, 2) < 0) {
			perror("(w) error on semaphore");
			exit(1);
		}
	}
}

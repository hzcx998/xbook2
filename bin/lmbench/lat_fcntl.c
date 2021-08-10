#include "bench.h"

/*
 * lat_fcntl.c - file locking test
 *
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char	*id = "$Id: lat_pipe.c,v 1.8 1997/06/16 05:38:58 lm Exp $\n";

#include "bench.h"

struct	flock lock, unlock;
struct	flock s1, s2;

/*
 * Create two files, use them as a ping pong test.
 * Process A:
 *	lock(1)
 *	unlock(2)
 * Process B:
 *	unlock(1)
 *	lock(2)
 * Initial state:
 *	lock is locked
 *	lock2 is locked
 */

#define	waiton(fd)	fcntl(fd, F_SETLKW, &lock)
#define	release(fd)	fcntl(fd, F_SETLK, &unlock)

struct _state {
	char filename1[2048];
	char filename2[2048];
	int	pid;
	int	fd1;
	int	fd2;
};

static void initialize(iter_t iterations, void* cookie);
static void benchmark(iter_t iterations, void* cookie);
static void cleanup(iter_t iterations, void* cookie);

static void
procA(struct _state *state)
{
	if (waiton(state->fd1) == -1) {
		perror("lock of fd1 failed\n");
		cleanup(0, state);
		exit(1);
	}
	if (release(state->fd2) == -1) {
		perror("unlock of fd2 failed\n");
		cleanup(0, state);
		exit(1);
	}
	if (waiton(state->fd2) == -1) {
		perror("lock of fd2 failed\n");
		cleanup(0, state);
		exit(1);
	}
	if (release(state->fd1) == -1) {
		perror("unlock of fd1 failed\n");
		cleanup(0, state);
		exit(1);
	}
}

static void
procB(struct _state *state)
{
	if (release(state->fd1) == -1) {
		perror("unlock of fd1 failed\n");
		cleanup(0, state);
		exit(1);
	}
	if (waiton(state->fd2) == -1) {
		perror("lock of fd2 failed\n");
		cleanup(0, state);
		exit(1);
	}
	if (release(state->fd2) == -1) {
		perror("unlock of fd2 failed\n");
		cleanup(0, state);
		exit(1);
	}
	if (waiton(state->fd1) == -1) {
		perror("lock of fd1 failed\n");
		cleanup(0, state);
		exit(1);
	}
}

static void
initialize(iter_t iterations, void* cookie)
{
	char	buf[10000];
	struct _state* state = (struct _state*)cookie;

	if (iterations) return;

	sprintf(state->filename1, "/tmp/lmbench-fcntl%d.1", getpid());
	sprintf(state->filename2, "/tmp/lmbench-fcntl%d.2", getpid());
	state->pid = 0;
	state->fd1 = -1;
	state->fd2 = -1;

	unlink(state->filename1);
	unlink(state->filename2);
	if ((state->fd1 = open(state->filename1, O_CREAT|O_RDWR, 0666)) == -1) {
		perror("create");
		exit(1);
	}
	if ((state->fd2 = open(state->filename2, O_CREAT|O_RDWR, 0666)) == -1) {
		perror("create");
		exit(1);
	}
	unlink(state->filename1);
	unlink(state->filename2);
	write(state->fd1, buf, sizeof(buf));
	write(state->fd2, buf, sizeof(buf));
	lock.l_type = F_WRLCK;
	lock.l_whence = 0;
	lock.l_start = 0;
	lock.l_len = 1;
	unlock = lock;
	unlock.l_type = F_UNLCK;
	if (waiton(state->fd1) == -1) {
		perror("lock1");
		exit(1);
	}
	if (waiton(state->fd2) == -1) {
		perror("lock2");
		exit(1);
	}
	handle_scheduler(benchmp_childid(), 0, 1);
	switch (state->pid = fork()) {
	case -1:
		perror("fork");
		exit(1);
	case 0:
		handle_scheduler(benchmp_childid(), 1, 1);
		for ( ;; ) {
			procB(state);
		}
		exit(0);
	default:
		break;
	}
}

static void
benchmark(iter_t iterations, void* cookie)
{
	struct _state* state = (struct _state*)cookie;
	
	while (iterations-- > 0) {
		procA(state);
	}
}

static void
cleanup(iter_t iterations, void* cookie)
{
	int i;
	struct _state* state = (struct _state*)cookie;

	if (iterations) return;

	if (state->fd1 >= 0) close(state->fd1);
	if (state->fd2 >= 0) close(state->fd2);
	state->fd1 = -1;
	state->fd2 = -1;

	if (state->pid) {
		kill(state->pid, SIGKILL);
		waitpid(state->pid, NULL, 0);
	}
	state->pid = 0;
}

int
lat_fcntl_main(int ac, char **av)
{
	int	i;
	int	c;
	int	parallel = 1;
	int	warmup = 0;
	int	repetitions = TRIES;
	struct _state state;
	char *usage = "[-P <parallelism>] [-W <warmup>] [-N <repetitions>]\n";

	/*
	 * If they specified a parallelism level, get it.
	 */
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

	state.pid = 0;

	benchmp(initialize, benchmark, cleanup, 0, parallel, 
		warmup, repetitions, &state);
	micro("Fcntl lock latency", 2 * get_n());

	return (0);
}

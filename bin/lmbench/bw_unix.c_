/*
 * bw_unix.c - simple Unix stream socket bandwidth test
 *
 * Usage: bw_unix [-m <message size>] [-M <total bytes>] \
 *		[-P <parallelism>] [-W <warmup>] [-N <repetitions>]
 *
 * Copyright (c) 1994 Larry McVoy.  
 * Copyright (c) 2002 Carl Staelin.
 * Distributed under the FSF GPL with additional restriction that results 
 * may published only if:
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char	*id = "$Id$\n";

#include "bench.h"

static void	reader(iter_t iterations, void * cookie);
static void	writer(int controlfd, int writefd, char* buf, void* cookie);

static size_t	XFER	= 10*1024*1024;

struct _state {
	int	pid;
	size_t	xfer;	/* bytes to read/write per "packet" */
	size_t	bytes;	/* bytes to read/write in one iteration */
	char	*buf;	/* buffer memory space */
	int	pipes[2];
	int	control[2];
	int	initerr;
};

static void
initialize(iter_t iterations, void* cookie)
{
	struct _state* state = (struct _state*)cookie;

	if (iterations) return;

	state->buf = valloc(XFERSIZE);
	touch(state->buf, XFERSIZE);
	state->initerr = 0;
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, state->pipes) == -1) {
		perror("socketpair");
		state->initerr = 1;
		return;
	}
	if (pipe(state->control) == -1) {
		perror("pipe");
		state->initerr = 2;
		return;
	}
	handle_scheduler(benchmp_childid(), 0, 1);
	switch (state->pid = fork()) {
	    case 0:
	      handle_scheduler(benchmp_childid(), 1, 1);
		close(state->control[1]);
		close(state->pipes[0]);
		writer(state->control[0], state->pipes[1], state->buf, state);
		return;
		/*NOTREACHED*/
	    
	    case -1:
		perror("fork");
		state->initerr = 3;
		return;
		/*NOTREACHED*/

	    default:
		break;
	}
	close(state->control[0]);
	close(state->pipes[1]);
}
static void
cleanup(iter_t iterations, void*  cookie)
{
	struct _state* state = (struct _state*)cookie;

	if (iterations) return;

	close(state->control[1]);
	close(state->pipes[0]);
	if (state->pid > 0) {
		kill(state->pid, SIGKILL);
		waitpid(state->pid, NULL, 0);
	}
	state->pid = 0;
}

static void
reader(iter_t iterations, void* cookie)
{
	struct _state* state = (struct _state*)cookie;
	size_t	done, n;
	size_t	todo = state->bytes;

	while (iterations-- > 0) {
		write(state->control[1], &todo, sizeof(todo));
		for (done = 0; done < todo; done += n) {
			if ((n = read(state->pipes[0], state->buf, state->xfer)) <= 0) {
				/* error! */
				exit(1);
			}
		}
	}
}

static void
writer(int controlfd, int writefd, char* buf, void* cookie)
{
	size_t	todo, n, done;
	struct _state* state = (struct _state*)cookie;

	for ( ;; ) {
		read(controlfd, &todo, sizeof(todo));
		for (done = 0; done < todo; done += n) {
#ifdef TOUCH
			touch(buf, XFERSIZE);
#endif
			if ((n = write(writefd, buf, state->xfer)) < 0) {
				/* error! */
				exit(1);
			}
		}
	}
}

int
bw_unix_main(int argc, char *argv[])
{
	struct _state state;
	int parallel = 1;
	int warmup = 0;
	int repetitions = TRIES;
	int c;
	char* usage = "[-m <message size>] [-M <total bytes>] [-P <parallelism>] [-W <warmup>] [-N <repetitions>]\n";

	state.xfer = XFERSIZE;	/* per-packet size */
	state.bytes = XFER;	/* total bytes per call */

	while (( c = getopt(argc,argv,"m:M:P:W:N:")) != EOF) {
		switch(c) {
		case 'm':
			state.xfer = bytes(optarg);
			break;
		case 'M':
			state.bytes = bytes(optarg);
			break;
		case 'P':
			parallel = atoi(optarg);
			if (parallel <= 0) lmbench_usage(argc, argv, usage);
			break;
		case 'W':
			warmup = atoi(optarg);
			break;
		case 'N':
			repetitions = atoi(optarg);
			break;
		default:
			lmbench_usage(argc, argv, usage);
			break;
		}
	}
	if (optind == argc - 1) {
		state.bytes = bytes(argv[optind]);
	} else if (optind < argc - 1) {
		lmbench_usage(argc, argv, usage);
	}

	state.pid = 0;

	/* round up total byte count to a multiple of xfer */
	if (state.bytes % state.xfer) {
		state.bytes += state.bytes - state.bytes % state.xfer;
	}

	benchmp(initialize, reader, cleanup, MEDIUM, parallel, 
		warmup, repetitions, &state);

	if (gettime() > 0) {
		fprintf(stderr, "AF_UNIX sock stream bandwidth: ");
		mb(get_n() * parallel * XFER);
	}
	return(0);
}




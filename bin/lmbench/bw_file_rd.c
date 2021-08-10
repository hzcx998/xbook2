/*
 * bw_file_rd.c - time reading & summing of a file
 *
 * Usage: bw_file_rd [-C] [-P <parallelism] [-W <warmup>] [-N <repetitions>] size file
 *
 * The intent is that the file is in memory.
 * Disk benchmarking is done with lmdd.
 *
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char	*id = "$Id$\n";

#include "bench.h"

#define	CHK(x)		if ((int)(x) == -1) { perror(#x); exit(1); }
#ifndef	MIN
#define	MIN(a, b)	((a) < (b) ? (a) : (b))
#endif

#define	TYPE	int
#define	MINSZ	(sizeof(TYPE) * 128)

static void	*buf;		/* do the I/O here */
static size_t	xfersize;	/* do it in units of this */
static size_t	count;		/* bytes to move (can't be modified) */

typedef struct _state {
	char filename[256];
	int fd;
	int clone;
} state_t;

void doit(int fd)
{
	int	sum = 0;
	size_t	size, chunk;

	size = count;
	chunk = xfersize;
	while (size >= 0) {
		if (size < chunk) chunk = size;
		if (read(fd, buf, MIN(size, chunk)) <= 0) {
			break;
		}
		bread(buf, MIN(size, xfersize));
		size -= chunk;
	}
}

static void
initialize(iter_t iterations, void* cookie)
{
	state_t	*state = (state_t *) cookie;

	if (iterations) return;

	state->fd = -1;
	if (state->clone) {
		char buf[128];
		char* s;

		/* copy original file into a process-specific one */
		sprintf(buf, "%d", (int)getpid());
		s = (char*)malloc(strlen(state->filename) + strlen(buf) + 1);
		sprintf(s, "%s%d", state->filename, (int)getpid());
		if (cp(state->filename, s, S_IREAD|S_IWRITE) < 0) {
			perror("creating private tempfile");
			unlink(s);
			exit(1);
		}
		strcpy(state->filename, s);
	}
}

void static
init_open(iter_t iterations, void * cookie)
{
	state_t	*state = (state_t *) cookie;
	int	ofd;

	if (iterations) return;

	initialize(0, cookie);
	CHK(ofd = open(state->filename, O_RDONLY));
	state->fd = ofd;
}

void static
time_with_open(iter_t iterations, void * cookie)
{
	state_t	*state = (state_t *) cookie;
	char	*filename = state->filename;
	int	fd;

	while (iterations-- > 0) {
		fd= open(filename, O_RDONLY);
		doit(fd);
		close(fd);
	}
}

void static
time_io_only(iter_t iterations,void * cookie)
{
	state_t *state = (state_t *) cookie;
	int fd = state->fd;

	while (iterations-- > 0) {
		lseek(fd, 0, 0);
		doit(fd);
	}
}

void static
cleanup(iter_t iterations, void * cookie)
{
	state_t *state = (state_t *) cookie;

	if (iterations) return;

	if (state->fd >= 0) close(state->fd);
	if (state->clone) unlink(state->filename);
}

int
bw_file_rd_main(int ac, char **av)
{
	int	fd;
	state_t state;
	int	parallel = 1;
	int	warmup = 0;
	int	repetitions = TRIES;
	int	c;
	char	usage[1024];
	
	sprintf(usage,"[-C] [-P <parallelism>] [-W <warmup>] [-N <repetitions>] <size> open2close|io_only <filename>"
		"\nmin size=%d\n",(int) (XFERSIZE>>10)) ;

	state.clone = 0;

	while (( c = getopt(ac, av, "P:W:N:C")) != EOF) {
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
		case 'C':
			state.clone = 1;
			break;
		default:
			lmbench_usage(ac, av, usage);
			break;
		}
	}

	if (optind + 3 != ac) { /* should have three arguments left */
		lmbench_usage(ac, av, usage);
	}

	strcpy(state.filename,av[optind+2]);
	count = bytes(av[optind]);
	if (count < MINSZ) {
		exit(1);	/* I want this to be quiet */
	}
	if (count < XFERSIZE) {
		xfersize = count;
	} else {
		xfersize = XFERSIZE;
	}
	buf = (void *)valloc(XFERSIZE);
	bzero(buf, XFERSIZE);

	if (!strcmp("open2close", av[optind+1])) {
		benchmp(initialize, time_with_open, cleanup,
			0, parallel, warmup, repetitions, &state);
	} else if (!strcmp("io_only", av[optind+1])) {
		benchmp(init_open, time_io_only, cleanup,
			0, parallel, warmup, repetitions, &state);
	} else lmbench_usage(ac, av, usage);
	bandwidth(count, get_n() * parallel, 0);
	return (0);
}

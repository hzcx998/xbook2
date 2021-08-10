/*
 * bw_mmap_rd.c - time reading & summing of a file using mmap
 *
 * Usage: bw_mmap_rd [-C] [-P <parallelism>] [-W <warmup>] [-N <repetitions>] size file
 *
 * Sizes less than 2m are not recommended.  Memory is read by summing it up
 * so the numbers include the cost of the adds.  If you use sizes large
 * enough, you can compare to bw_mem_rd and get the cost of TLB fills 
 * (very roughly).
 *
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char	*id = "$Id$\n";

#include "bench.h"
#ifdef MAP_FILE
#	define	MMAP_FLAGS	MAP_FILE|MAP_SHARED
#else
#	define	MMAP_FLAGS	MAP_SHARED
#endif

#define	TYPE	int
#define	MINSZ	(sizeof(TYPE) * 128)
#define	CHK(x)	if ((long)(x) == -1) { perror("x"); exit(1); }

typedef struct _state {
	size_t	nbytes;
	char	filename[256];
	int	fd;
	int	clone;
	void	*buf;
} state_t;

static void time_no_open(iter_t iterations, void * cookie);
static void time_with_open(iter_t iterations, void * cookie);
static void initialize(iter_t iterations, void *cookie);
static void init_open(iter_t iterations, void *cookie);
static void cleanup(iter_t iterations, void *cookie);

int
bw_mmap_rd_main(int ac, char **av)
{
	int	fd;
	struct	stat sbuf;
	void	*buf;
	int	parallel = 1;
	int	warmup = 0;
	int	repetitions = TRIES;
	size_t	nbytes;
	state_t	state;
	int	c;
	char	*usage = "[-C] [-P <parallelism>] [-W <warmup>] [-N <repetitions>] <size> open2close|mmap_only <filename>";

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

	/* should have three arguments left (bytes type filename) */
	if (optind + 3 != ac) {
		lmbench_usage(ac, av, usage);
	}

	nbytes = state.nbytes = bytes(av[optind]);
	strcpy(state.filename,av[optind+2]);
	CHK(stat(state.filename, &sbuf));
	if ((S_ISREG(sbuf.st_mode) && nbytes > sbuf.st_size) 
	    || (nbytes < MINSZ)) {
		fprintf(stderr,"<size> out of range!\n");
		exit(1);
	}

	if (!strcmp("open2close", av[optind+1])) {
		benchmp(initialize, time_with_open, cleanup,
			0, parallel, warmup, repetitions, &state);
	} else if (!strcmp("mmap_only", av[optind+1])) {
		benchmp(init_open, time_no_open, cleanup,
			0, parallel, warmup, repetitions, &state);
	} else {
		lmbench_usage(ac, av, usage);
	}
	bandwidth(nbytes, get_n() * parallel, 0);
	return (0);
}

static void
initialize(iter_t iterations, void* cookie)
{
	state_t	*state = (state_t *) cookie;

	if (iterations) return;

	state->fd = -1;
	state->buf = NULL;

	if (state->clone) {
		char buf[8192];
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

static void
init_open(iter_t iterations, void *cookie)
{
	state_t *state = (state_t *) cookie;

	if (iterations) return;

	initialize(0, cookie);
	CHK(state->fd = open(state->filename, 0));
	CHK(state->buf = mmap(0, state->nbytes, PROT_READ,
				     MMAP_FLAGS, state->fd, 0));
}

static void
cleanup(iter_t iterations, void *cookie)
{
	state_t *state = (state_t *) cookie;

	if (iterations) return;
	if (state->buf) munmap(state->buf, state->nbytes);
	if (state->fd >= 0) close(state->fd);
	if (state->clone) unlink(state->filename);
}

static void
time_no_open(iter_t iterations, void * cookie)
{
	state_t *state = (state_t *) cookie;

	while (iterations-- > 0) {
	    bread(state->buf, state->nbytes);
	}
}

static void
time_with_open(iter_t iterations, void *cookie)
{
	state_t *state    = (state_t *) cookie;
	char 	*filename = state->filename;
	size_t	nbytes    = state->nbytes;
	int 	fd;
	void	*p;

	while (iterations-- > 0) {
	    CHK(fd = open(filename, 0));
	    CHK(p = mmap(0, nbytes, PROT_READ, MMAP_FLAGS, fd, 0));
	    bread(p, nbytes);
	    close(fd);
	    munmap(p, nbytes);
	}
}

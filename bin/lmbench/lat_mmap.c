/*
 * lat_mmap.c - time how fast a mapping can be made and broken down
 *
 * Usage: mmap [-r] [-C] [-P <parallelism>] [-W <warmup>] [-N <repetitions>] size file
 *
 * XXX - If an implementation did lazy address space mapping, this test
 * will make that system look very good.  I haven't heard of such a system.
 *
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char	*id = "$Id$\n";

#include "bench.h"

#define	PSIZE	(16<<10)
#define	N	10
#define	STRIDE	(10*PSIZE)
#define	MINSIZE	(STRIDE*2)

#define	CHK(x)	if ((x) == -1) { perror("x"); exit(1); }


typedef struct _state {
	size_t	size;
	int	fd;
	int	random;
	int	clone;
	char	*name;
} state_t;

static void	init(iter_t iterations, void *cookie);
static void	cleanup(iter_t iterations, void *cookie);
static void	domapping(iter_t iterations, void * cookie);

int
lat_mmap_main(int ac, char **av)
{
	state_t state;
	int	parallel = 1;
	int	warmup = 0;
	int	repetitions = TRIES;
	char	buf[256];
	int	c;
	char	*usage = "[-r] [-C] [-P <parallelism>] [-W <warmup>] [-N <repetitions>] size file\n";
	

	state.random = 0;
	state.clone = 0;
	while (( c = getopt(ac, av, "rP:W:N:C")) != EOF) {
		switch(c) {
		case 'P':
			parallel = atoi(optarg);
			if (parallel <= 0)
				lmbench_usage(ac, av, usage);
			break;
		case 'W':
			warmup = atoi(optarg);
			break;
		case 'N':
			repetitions = atoi(optarg);
			break;
		case 'r':
			state.random = 1;
			break;
		case 'C':
			state.clone = 1;
			break;
		default:
			lmbench_usage(ac, av, usage);
			break;
		}
	}

	if (optind + 2 != ac) {
		lmbench_usage(ac, av, usage);
	}

	state.size = bytes(av[optind]);
	if (state.size < MINSIZE) {
		return (1);
	}
	state.name = av[optind+1];

	benchmp(init, domapping, cleanup, 0, parallel, 
		warmup, repetitions, &state);

	if (gettime() > 0) {
		micromb(state.size, get_n());
	}
	return (0);
}

static void
init(iter_t iterations, void* cookie)
{
	state_t *state = (state_t *) cookie;
	
	if (iterations) return;

	if (state->clone) {
		char buf[128];
		char* s;

		/* copy original file into a process-specific one */
		sprintf(buf, "%d", (int)getpid());
		s = (char*)malloc(strlen(state->name) + strlen(buf) + 1);
		sprintf(s, "%s%d", state->name, (int)getpid());
		if (cp(state->name, s, S_IREAD|S_IWRITE) < 0) {
			perror("Could not copy file");
			unlink(s);
			exit(1);
		}
		state->name = s;
	}
	CHK(state->fd = open(state->name, O_RDWR));
	if (state->clone) unlink(state->name);
	if (lseek(state->fd, 0, SEEK_END) < state->size) {
		fprintf(stderr, "Input file too small\n");
		exit(1);
	}
}

static void
cleanup(iter_t iterations, void* cookie)
{
	state_t *state = (state_t *) cookie;

	if (iterations) return;

	close(state->fd);
}

/*
 * This alg due to Linus.  The goal is to have both sparse and full
 * mappings reported.
 */
static void
domapping(iter_t iterations, void *cookie)
{
	state_t *state = (state_t *) cookie;
	register int fd = state->fd;
	register size_t size = state->size;
	register int random = state->random;
	register char	*p, *where, *end;
	register char	c = size & 0xff;

	while (iterations-- > 0) {

#ifdef	MAP_FILE
		where = mmap(0, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);
#else
		where = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
#endif
		if ((long)where == -1) {
			perror("mmap");
			exit(1);
		}
		if (random) {
			end = where + size;
			for (p = where; p < end; p += STRIDE) {
				*p = c;
			}
		} else {
			end = where + (size / N);
			for (p = where; p < end; p += PSIZE) {
				*p = c;
			}
		}
		munmap(where, size);
	}
}

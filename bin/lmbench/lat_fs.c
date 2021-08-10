/*
 * Benchmark creates & deletes.
 */

static char	*id = "$Id$\n";

#include "bench.h"


struct _state {
	char	*tmpdir;
	int	max;
	int	n;
	char**	names;
	int	ndirs;
	char**	dirs;
	size_t	size;
};
static void	measure(size_t size, 
		int parallel, int warmup, int repetitions, void* cookie);
static void	mkfile(char* s, size_t size);
static void	setup_names(iter_t iterations, void* cookie);
static void	cleanup_names(iter_t iterations, void* cookie);
static void	setup_rm(iter_t iterations, void* cookie);
static void	cleanup_mk(iter_t iterations, void* cookie);
static void	benchmark_mk(iter_t iterations, void* cookie);
static void	benchmark_rm(iter_t iterations, void* cookie);

int
lat_fs_main(int ac, char **av)
{
	int i;
	int parallel = 1;
	int warmup = 0;
	int repetitions = TRIES;
	static	int	sizes[] = { 0, 1024, 4096, 10*1024 };
	struct _state state;
	int c;
	char* usage = "[-s <file size>] [-n <max files per dir>] [-P <parallelism>] [-W <warmup>] [-N <repetitions>] [<dir>]\n";

	state.size = 0;
	state.max = 100;
	state.tmpdir = NULL;

	while (( c = getopt(ac, av, "s:n:P:W:N:")) != EOF) {
		switch(c) {
		case 's':
			state.size = bytes(optarg);
			break;
		case 'n':
			state.max = bytes(optarg);
			break;
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
	if (optind < ac - 1) {
		lmbench_usage(ac, av, usage);
	}
	if (optind == ac - 1) {
		state.tmpdir = av[1];
	}

	if (state.size) {
		measure(state.size, parallel, warmup, repetitions, &state);
	} else {
		for (i = 0; i < sizeof(sizes)/sizeof(int); ++i) {
			state.size = sizes[i];
			measure(state.size, 
				parallel, warmup, repetitions, &state);
		}
	}
	return(0);
}

static void
measure(size_t size, int parallel, int warmup, int repetitions, void* cookie)
{
	fprintf(stderr, "%luk", size>>10);
	benchmp(setup_names, benchmark_mk, cleanup_mk, 0, parallel,
		warmup, repetitions, cookie);
	if (gettime()) {
		fprintf(stderr, "\t%lu\t%.0f", (unsigned long)get_n(), 
			(double)(1000000. * get_n() / (double)gettime()));
	} else {
		fprintf(stderr, "\t-1\t-1");
	}

	benchmp(setup_rm, benchmark_rm, cleanup_names, 0, parallel,
		warmup, repetitions, cookie);
	if (gettime()) {
		fprintf(stderr, "\t%.0f", 
			(double)(1000000. * get_n() / (double)gettime()));
	} else {
		fprintf(stderr, "\t-1");
	}
	fprintf(stderr, "\n");
}

static void
mkfile(char *name, size_t size)
{
	size_t	chunk;
	int	fd = creat(name, 0666);
	char	buf[128*1024];		/* XXX - track sizes */

	while (size > 0) {
		chunk = ((size > (128*1024)) ? (128*1024) : size);
		write(fd, buf, chunk);
		size -= chunk;
	}
	close(fd);
}

static void
setup_names_recurse(iter_t* foff, iter_t* doff, int depth, struct _state* state)
{
	long	i, ndirs, count;
	char*	basename = state->dirs[*doff];
	char	name[L_tmpnam + 8192];

	if (depth > 0) {
		for (count = state->max, i = 1; i < depth; ++i) {
			count *= state->max;
		}
		ndirs = (state->n - *foff) / count + 1;
		for (i = 0; i < state->max && i < ndirs && *foff < state->n; ++i) {
			sprintf(name, "%s/%ld", basename, i);
			state->dirs[++(*doff)] = strdup(name);
			mkdir(name, 0777);
			setup_names_recurse(foff, doff, depth-1, state);
		}
	} else {
		for (i = 0; i < state->max && *foff < state->n; ++i) {
			sprintf(name, "%s/%ld", basename, i);
			state->names[(*foff)++] = strdup(name);
		}
	}
}

static void
setup_names(iter_t iterations, void* cookie)
{
	long	i, ndirs, depth;
	iter_t	foff;
	iter_t	doff;
	char	dirname_tmpl[L_tmpnam + 256];
	char*	dirname;
	struct _state* state = (struct _state*)cookie;

	if (!iterations) return;

	depth = 0;
	state->n = iterations;
	state->ndirs = iterations / state->max;
	if (iterations % state->max) state->ndirs++;
	for (ndirs = state->ndirs; ndirs > 1; ) {
		ndirs = ndirs / state->max + ((ndirs % state->max) ? 1 : 0);
		state->ndirs += ndirs;
		depth++;
	}

	state->names = (char**)malloc(iterations * sizeof(char*));
	for (i = 0; i < iterations; ++i) {
		state->names[i] = NULL;
	}

	state->dirs = (char**)malloc(state->ndirs * sizeof(char*));
	for (i = 0; i < state->ndirs; ++i) {
		state->dirs[i] = NULL;
	}

	sprintf(dirname_tmpl, "lat_fs_%d_XXXXXX", getpid());
	dirname = tempnam(state->tmpdir, dirname_tmpl);
	if (!dirname) {
		perror("tempnam failed");
		exit(1);
	}
	if (mkdir(dirname, S_IRUSR|S_IWUSR|S_IXUSR)) {
		perror("mkdir failed");
		exit(1);
	}
	state->dirs[0] = dirname;
	foff = 0;
	doff = 0;
	setup_names_recurse(&foff, &doff, depth, state);
	if (foff != iterations || doff != state->ndirs - 1) {
		fprintf(stderr, "setup_names: ERROR: foff=%lu, iterations=%lu, doff=%lu, ndirs=%lu, depth=%d\n", (unsigned long)foff, (unsigned long)iterations, (unsigned long)doff, (unsigned long)state->ndirs, depth);
	}
}

static void
cleanup_names(iter_t iterations, void* cookie)
{
	long	i;
	struct _state* state = (struct _state*)cookie;

	if (!iterations) return;

	for (i = 0; i < state->n; ++i) {
		if (state->names[i]) free(state->names[i]);
	}
	free(state->names);
	state->n = 0;

	for (i = state->ndirs - 1; i >= 0; --i) {
		if (state->dirs[i]) {
			rmdir(state->dirs[i]);
			free(state->dirs[i]);
		}
	}
	free(state->dirs);
	state->ndirs = 0;
}

static void
setup_rm(iter_t iterations, void* cookie)
{
	if (!iterations) return;

	setup_names(iterations, cookie);
	benchmark_mk(iterations, cookie);
}

static void
cleanup_mk(iter_t iterations, void* cookie)
{
	if (!iterations) return;

	benchmark_rm(iterations, cookie);
	cleanup_names(iterations, cookie);
}

static void
benchmark_mk(iter_t iterations, void* cookie)
{
	struct _state* state = (struct _state*)cookie;

	while (iterations-- > 0) {
		if (!state->names[iterations]) {
			fprintf(stderr, "benchmark_mk: null filename at %lu of %lu\n", iterations, state->n);
			continue;
		}
		mkfile(state->names[iterations], state->size);
	}
}

static void
benchmark_rm(iter_t iterations, void* cookie)
{
	struct _state* state = (struct _state*)cookie;

	while (iterations-- > 0) {
		if (!state->names[iterations]) {
			fprintf(stderr, "benchmark_rm: null filename at %lu of %lu\n", iterations, state->n);
			continue;
		}
		unlink(state->names[iterations]);
	}
}


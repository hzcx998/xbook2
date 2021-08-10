/*
 * lat_sig.c - signal handler test
 *
 * XXX - this benchmark requires the POSIX sigaction interface.  The reason
 * for that is that the signal handler stays installed with that interface.
 * The more portable signal() interface may or may not stay installed and
 * reinstalling it each time is expensive.
 *
 * XXX - should really do a two process version.
 *
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 */
static char	*id = "$Id$\n";

#include "bench.h"
#include <setjmp.h>

static uint64	caught, n;
double	adj;
void	handler() { }
jmp_buf	prot_env;

static void
do_send(iter_t iterations, void* cookie)
{
	int	me = getpid();

	while (--iterations > 0) {
		kill(me, 0);
	}
}

static void
do_install(iter_t iterations, void* cookie)
{
	struct	sigaction sa, old;

	while (iterations-- > 0) {
		sa.sa_handler = handler;
		sigemptyset(&sa.sa_mask);	
		sa.sa_flags = 0;
		sigaction(SIGUSR1, &sa, &old);
	}
}

static void
do_catch(iter_t iterations, void* cookie)
{
	int	me = getpid();
	struct	sigaction sa, old;
	double	u;

	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);	
	sa.sa_flags = 0;
	sigaction(SIGUSR1, &sa, &old);

	while (--iterations > 0) {
		kill(me, SIGUSR1);
	}
}

struct _state {
	char*	fname;
	char*	where;
};

static void
prot() {
	if (++caught == n) {
		caught = 0;
		n = benchmp_interval(benchmp_getstate());
	}
}

static void
initialize(iter_t iterations, void* cookie)
{
	struct _state* state = (struct _state*)cookie;
	int	fd;
	struct	sigaction sa;

	if (iterations) return;

	fd = open(state->fname, 0);
	state->where = mmap(0, 4096, PROT_READ, MAP_SHARED, fd, 0);
	if ((long)state->where == -1) {
		perror("mmap");
		exit(1);
	}

	sa.sa_handler = prot;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGSEGV, &sa, 0);
	sigaction(SIGBUS, &sa, 0);
}

static void
do_prot(iter_t iterations, void* cookie)
{
	struct _state* state = (struct _state*)cookie;

	n = iterations;
	caught = 0;

	/* start the first timing interval */
	start(0);

	/* trigger the page fault, causing us to jump to prot() */
	*state->where = 1;
}

/*
 * Cost of catching the signal less the cost of sending it
 */
static void
bench_catch(int parallel, int warmup, int repetitions)
{
	uint64 t, send_usecs, send_n;

	/* measure cost of sending signal */
	benchmp(NULL, do_send, NULL, 0, parallel, 
		warmup, repetitions, NULL);
	send_usecs = gettime();
	send_n = get_n();

	/* measure cost of sending & catching signal */
	benchmp(NULL, do_catch, NULL, 0, parallel, 
		warmup, repetitions, NULL);

	/* subtract cost of sending signal */
	if (gettime() > (send_usecs * get_n()) / send_n) {
		settime(gettime() - (send_usecs * get_n()) / send_n);
	} else {
		settime(0);
	}
}

static void
bench_prot(char* fname, int parallel, int warmup, int repetitions)
{
	uint64 catch_usecs, catch_n;
	struct _state state;

	state.fname = fname;

	/*
	 * Catch protection faults.
	 * Assume that they will cost the same as a normal catch.
	 */
	bench_catch(parallel, warmup, repetitions);
	catch_usecs = gettime();
	catch_n = get_n();

	benchmp(initialize, do_prot, NULL, 0, parallel, 
		warmup, repetitions, &state);
	if (gettime() > (catch_usecs * get_n()) / catch_n) {
		settime(gettime() - (catch_usecs * get_n()) / catch_n);
	} else {
		settime(0);
	}
}


int
lat_sig_main(int ac, char **av)
{
	int parallel = 1;
	int warmup = 0;
	int repetitions = TRIES;
	int c;
	char* usage = "[-P <parallelism>] [-W <warmup>] [-N <repetitions>] install|catch|prot [file]\n";

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
	if (optind != ac - 1 && optind != ac - 2) {
		lmbench_usage(ac, av, usage);
	}

	if (!strcmp("install", av[optind])) {
		benchmp(NULL, do_install, NULL, 0, parallel, 
			warmup, repetitions, NULL);
		micro("Signal handler installation", get_n());
	} else if (!strcmp("catch", av[optind])) {
		bench_catch(parallel, warmup, repetitions);
		micro("Signal handler overhead", get_n());
	} else if (!strcmp("prot", av[optind]) && optind == ac - 2) {
		bench_prot(av[optind+1], parallel, warmup, repetitions);
		micro("Protection fault", get_n());
	} else {
		lmbench_usage(ac, av, usage);
	}
	return(0);
}

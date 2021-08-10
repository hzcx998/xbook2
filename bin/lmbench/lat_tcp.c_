/*
 * lat_tcp.c - simple TCP transaction latency test
 *
 * Three programs in one -
 *	server usage:	tcp_xact -s
 *	client usage:	tcp_xact [-m <message size>] [-P <parallelism>] [-W <warmup>] [-N <repetitions>] hostname
 *	shutdown:	tcp_xact -S hostname
 *
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char	*id = "$Id$\n";

#include "bench.h"

typedef struct _state {
	int	msize;
	int	sock;
	char	*server;
	char	*buf;
} state_t;

static void	init(iter_t iterations, void* cookie);
static void	cleanup(iter_t iterations, void* cookie);
static void	doclient(iter_t iterations, void* cookie);
static void	server_lat_tcp_main();
static void	doserver(int sock);

int
lat_tcp_main(int ac, char **av)
{
	state_t state;
	int	parallel = 1;
	int	warmup = 0;
	int	repetitions = TRIES;
	int 	c;
	char	buf[256];
	char	*usage = "-s\n OR [-m <message size>] [-P <parallelism>] [-W <warmup>] [-N <repetitions>] server\n OR -S server\n";

	state.msize = 1;

	while (( c = getopt(ac, av, "sS:m:P:W:N:")) != EOF) {
		switch(c) {
		case 's': /* Server */
			if (fork() == 0) {
				server_lat_tcp_main();
			}
			exit(0);
		case 'S': /* shutdown serverhost */
			state.sock = tcp_connect(optarg,
						 TCP_XACT,
						 SOCKOPT_NONE);
			close(state.sock);
			exit(0);
		case 'm':
			state.msize = atoi(optarg);
			break;
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
		default:
			lmbench_usage(ac, av, usage);
			break;
		}
	}

	if (optind != ac - 1) {
		lmbench_usage(ac, av, usage);
	}

	state.server = av[optind];
	benchmp(init, doclient, cleanup, MEDIUM, parallel, 
		warmup, repetitions, &state);

	sprintf(buf, "TCP latency using %s", state.server);
	micro(buf, get_n());

	exit(0);
}

static void
init(iter_t iterations, void* cookie)
{
	state_t *state = (state_t *) cookie;
	int	msize  = htonl(state->msize);

	if (iterations) return;

	state->sock = tcp_connect(state->server, TCP_XACT, SOCKOPT_NONE);
	state->buf = malloc(state->msize);

	write(state->sock, &msize, sizeof(int));
}

static void
cleanup(iter_t iterations, void* cookie)
{
	state_t *state = (state_t *) cookie;

	if (iterations) return;

	close(state->sock);
	free(state->buf);
}

static void
doclient(iter_t iterations, void* cookie)
{
	state_t *state = (state_t *) cookie;
	int 	sock   = state->sock;

	while (iterations-- > 0) {
		write(sock, state->buf, state->msize);
		read(sock, state->buf, state->msize);
	}
}

static void
server_lat_tcp_main()
{
	int     newsock, sock;

	GO_AWAY;
	signal(SIGCHLD, sigchld_wait_handler);
	sock = tcp_server(TCP_XACT, SOCKOPT_REUSE);
	for (;;) {
		newsock = tcp_accept(sock, SOCKOPT_NONE);
		switch (fork()) {
		    case -1:
			perror("fork");
			break;
		    case 0:
			doserver(newsock);
			exit(0);
		    default:
			close(newsock);
			break;
		}
	}
	/* NOTREACHED */
}

static void
doserver(int sock)
{
	int	n;

	if (read(sock, &n, sizeof(int)) == sizeof(int)) {
		int	msize = ntohl(n);
		char*   buf = (char*)malloc(msize);

		for (n = 0; read(sock, buf, msize) > 0; n++) {
			write(sock, buf, msize);
		}
		free(buf);
	} else {
		/*
		 * A connection with no data means shut down.
		 */
		tcp_done(TCP_XACT);
		kill(getppid(), SIGTERM);
		exit(0);
	}
}

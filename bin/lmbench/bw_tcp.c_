/*
 * bw_tcp.c - simple TCP bandwidth test
 *
 * Three programs in one -
 *	server usage:	bw_tcp -s
 *	client usage:	bw_tcp [-m <message size>] [-M <total bytes>] [-P <parallelism>] [-W <warmup>] [-N <repetitions>] hostname 
 *	shutdown:	bw_tcp -hostname
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

typedef struct _state {
	int	sock;
	uint64	move;
	int	msize;
	char	*server;
	int	fd;
	char	*buf;
} state_t;

static void	server_bw_tcp_main();
static void	client_bw_tcp_main(int parallel, state_t *state);
static void	source(int data);

static void	initialize(iter_t iterations, void* cookie);
static void	loop_transfer(iter_t iterations, void *cookie);
static void	cleanup(iter_t iterations, void* cookie);

int
bw_tcp_main(int ac, char **av)
{
	int	parallel = 1;
	int	warmup = LONGER;
	int	repetitions = TRIES;
	int	shutdown = 0;
	state_t state;
	char	*usage = "-s\n OR [-m <message size>] [-M <bytes to move>] [-P <parallelism>] [-W <warmup>] [-N <repetitions>] server\n OR -S serverhost\n";
	int	c;
	
	state.msize = 0;
	state.move = 0;

	/* Rest is client argument processing */
	while (( c = getopt(ac, av, "sS:m:M:P:W:N:")) != EOF) {
		switch(c) {
		case 's': /* Server */
			if (fork() == 0) {
				server_bw_tcp_main();
			}
			exit(0);
			break;
		case 'S': /* shutdown serverhost */
		{
			int	conn;
			conn = tcp_connect(optarg, TCP_DATA, SOCKOPT_NONE);
			write(conn, "0", 1);
			exit(0);
		}
		case 'm':
			state.msize = bytes(optarg);
			break;
		case 'M':
			state.move = bytes(optarg);
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

	if (optind < ac - 2 || optind >= ac) {
		lmbench_usage(ac, av, usage);
	}

	state.server = av[optind++];

	if (state.msize == 0 && state.move == 0) {
		state.msize = state.move = XFERSIZE;
	} else if (state.msize == 0) {
		state.msize = state.move;
	} else if (state.move == 0) {
		state.move = state.msize;
	}

	/* make the number of bytes to move a multiple of the message size */
	if (state.move % state.msize) {
		state.move += state.msize - state.move % state.msize;
	}

	/*
	 * Default is to warmup the connection for seven seconds, 
	 * then measure performance over each timing interval.
	 * This minimizes the effect of opening and initializing TCP 
	 * connections.
	 */
	benchmp(initialize, loop_transfer, cleanup, 
		0, parallel, warmup, repetitions, &state);
	if (gettime() > 0) {
		fprintf(stderr, "%.6f ", state.msize / (1000. * 1000.));
		mb(state.move * get_n() * parallel);
	}
}

static void
initialize(iter_t iterations, void *cookie)
{
	int	c;
	char	buf[100];
	state_t *state = (state_t *) cookie;

	if (iterations) return;

	state->buf = valloc(state->msize);
	if (!state->buf) {
		perror("valloc");
		exit(1);
	}
	touch(state->buf, state->msize);

	state->sock = tcp_connect(state->server, TCP_DATA, SOCKOPT_READ|SOCKOPT_WRITE|SOCKOPT_REUSE);
	if (state->sock < 0) {
		perror("socket connection");
		exit(1);
	}
	sprintf(buf, "%lu", state->msize);
	if (write(state->sock, buf, strlen(buf) + 1) != strlen(buf) + 1) {
		perror("control write");
		exit(1);
	}
}

void 
loop_transfer(iter_t iterations, void *cookie)
{
	int	c;
	uint64	todo;
	state_t *state = (state_t *) cookie;

	while (iterations-- > 0) {
		for (todo = state->move; todo > 0; todo -= c) {
			if ((c = read(state->sock, state->buf, state->msize)) <= 0) {
				exit(1);
			}
			if (c > todo) c = todo;
		}
	}
}

static void
cleanup(iter_t iterations, void* cookie)
{
	state_t *state = (state_t *) cookie;

	if (iterations) return;

	/* close connection */
	(void)close(state->sock);
}

static void
server_bw_tcp_main()
{
	int	data, newdata;

	GO_AWAY;

	data = tcp_server(TCP_DATA, SOCKOPT_WRITE|SOCKOPT_REUSE);
	if (data < 0) {
		perror("server socket creation");
		exit(1);
	}

	signal(SIGCHLD, sigchld_wait_handler);
	for ( ;; ) {
		newdata = tcp_accept(data, SOCKOPT_WRITE);
		switch (fork()) {
		    case -1:
			perror("fork");
			break;
		    case 0:
			source(newdata);
			exit(0);
		    default:
			close(newdata);
			break;
		}
	}
}

/*
 * Read the message size.  Keep transferring 
 * data in message-size sized packets until
 * the socket goes away.
 */
static void
source(int data)
{
	size_t	count, m;
	unsigned long	nbytes;
	char	*buf, scratch[100];

	/*
	 * read the message size
	 */
	bzero(scratch, 100);
	if (read(data, scratch, 100) <= 0) {
		perror("control nbytes");
		exit(7);
	}
	sscanf(scratch, "%lu", &nbytes);
	m = nbytes;

	/*
	 * A hack to allow turning off the absorb daemon.
	 */
     	if (m == 0) {
		tcp_done(TCP_DATA);
		kill(getppid(), SIGTERM);
		exit(0);
	}

	buf = valloc(m);
	bzero(buf, m);

	/*
	 * Keep sending messages until the connection is closed
	 */
	while (write(data, buf, m) == m) {
#ifdef	TOUCH
		touch(buf, m);
#endif
	}
	free(buf);
}

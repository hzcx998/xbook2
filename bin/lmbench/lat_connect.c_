/*
 * lat_connect.c - simple TCP connection latency test
 *
 * Three programs in one -
 *	server usage:	lat_connect -s
 *	client usage:	lat_connect [-N <repetitions>] hostname
 *	shutdown:	lat_connect -hostname
 *
 * lat_connect may not be parallelized because of idiosyncracies
 * with TCP connection creation.  Basically, if the client tries
 * to create too many connections too quickly, the system fills
 * up the set of available connections with TIME_WAIT connections.
 * We can only measure the TCP connection cost accurately if we
 * do just a few connections.  Since the parallel harness needs
 * each child to run for a second, this guarantees that the 
 * parallel version will generate inaccurate results.
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
	char	*server;
} state_t;

static void	doclient(iter_t iterations, void * cookie);
static void	server_lat_connect_main();

int
lat_connect_main(int ac, char **av)
{
	state_t state;
	int	repetitions = TRIES;
	int 	c;
	char	buf[256];
	char	*usage = "-s\n OR [-S] [-N <repetitions>] server\n";

	while (( c = getopt(ac, av, "sSP:W:N:")) != EOF) {
		switch(c) {
		case 's': /* Server */
			if (fork() == 0) {
				server_lat_connect_main();
			}
			exit(0);
		case 'S': /* shutdown serverhost */
		{
			int sock = tcp_connect(av[optind],
					       TCP_CONNECT,
					       SOCKOPT_NONE);
			write(sock, "0", 1);
			close(sock);
			exit(0);
		}
		case 'N':
			repetitions = atoi(optarg);
			break;
		default:
			lmbench_usage(ac, av, usage);
			break;
		}
	}

	if (optind + 1 != ac) {
		lmbench_usage(ac, av, usage);
	}

	state.server = av[optind];
	benchmp(NULL, doclient, NULL, 0, 1, 0, repetitions, &state);

	sprintf(buf, "TCP/IP connection cost to %s", state.server);
	micro(buf, get_n());
	exit(0);
}

static void
doclient(iter_t iterations, void *cookie)
{
	state_t *state = (state_t *) cookie;
	register char	*server = state->server;
	register int 	sock;
	
	while (iterations-- > 0) {
		sock = tcp_connect(server, TCP_CONNECT, SOCKOPT_REUSE);
		close(sock);
	}
}

static void
server_lat_connect_main()
{
	int     newsock, sock;
	char	c ='1';

	GO_AWAY;
	sock = tcp_server(TCP_CONNECT, SOCKOPT_NONE|SOCKOPT_REUSE);
	for (;;) {
		newsock = tcp_accept(sock, SOCKOPT_NONE);
		if (read(newsock, &c, 1) > 0) {
			tcp_done(TCP_CONNECT);
			exit(0);
		}
		close(newsock);
	}
	/* NOTREACHED */
}

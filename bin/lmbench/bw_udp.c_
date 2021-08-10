/*
 * bw_udp.c - simple UDP bandwidth test
 *
 * Three programs in one -
 *	server usage:	bw_tcp -s
 *	client usage:	bw_tcp [-m <message size>] [-P <parallelism>] [-W <warmup>] [-N <repetitions>] hostname [bytes]
 *	shutdown:	bw_tcp -S hostname
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

#define MAX_MSIZE (10 * 1024 * 1024)

typedef struct _state {
	int	sock;
	int	seq;
	long	move;
	long	msize;
	char	*server;
	int	fd;
	char	*buf;
} state_t;

static void	server_main();
static void	client_main(int parallel, state_t *state);
static void	init(iter_t iterations, void *cookie);
static void	cleanup(iter_t iterations, void *cookie);
static void	loop_transfer(iter_t iterations, void *cookie);

int
bw_udp_main(int ac, char **av)
{
	int	parallel = 1;
	int	warmup = 0;
	int	repetitions = TRIES;
	int	server = 0;
	state_t state;
	char	*usage = "-s\n OR [-m <message size>] [-W <warmup>] [-N <repetitions>] server [size]\n OR -S serverhost\n";
	int	c;
	uint64	usecs;
	
	state.msize = 0;
	state.move = 10*1024*1024;

	/* Rest is client argument processing */
	while (( c = getopt(ac, av, "sS:m:W:N:")) != EOF) {
		switch(c) {
		case 's': /* Server */
			if (fork() == 0) {
				server_main();
			}
			exit(0);
		case 'S': /* shutdown serverhost */
		{
			int seq, n;
			int sock = udp_connect(optarg,
					       UDP_XACT,
					       SOCKOPT_NONE);
			for (n = -1; n > -5; --n) {
				seq = htonl(n);
				(void) send(sock, &seq, sizeof(int), 0);
			}
			close(sock);
			exit (0);
		}
		case 'm':
			state.msize = atoi(optarg);
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
	if (optind < ac) {
		state.move = bytes(av[optind]);
	}
	if (state.msize == 0) {
		state.msize = state.move;
	}
	/* make the number of bytes to move a multiple of the message size */
	if (state.move % state.msize) {
		state.move += state.move - state.move % state.msize;
	}

	state.buf = valloc(state.msize);
	if (!state.buf) {
		perror("valloc");
		exit(1);
	}
	touch(state.buf, state.msize);

	/*
	 * Make one run take at least 5 seconds.
	 * This minimizes the effect of connect & reopening TCP windows.
	 */
	benchmp(init, loop_transfer, cleanup, LONGER, parallel, warmup, repetitions, &state );

out:	(void)fprintf(stderr, "socket UDP bandwidth using %s: ", state.server);
	mb(state.move * get_n() * parallel);
}

static void
init(iter_t iterations, void* cookie)
{
	state_t *state = (state_t *) cookie;

	if (iterations) return;

	state->sock = udp_connect(state->server, UDP_XACT, SOCKOPT_NONE);
	state->seq = 0;
	state->buf = (char*)malloc(state->msize);
}

static void
loop_transfer(iter_t iterations, void *cookie)
{
	state_t *state = (state_t *) cookie;
	char	*server = state->server;
	int	sock = state->sock;
	long	control[2], nbytes;

	nbytes = state->move;
	control[0] = state->move;
	control[1] = state->msize;

	while (iterations-- > 0) {
		if (send(sock, control, 2 * sizeof(long), 0) != 2 * sizeof(long)) {
			perror("bw_udp client: send failed");
			exit(5);
		}
		while (nbytes > 0) {
			if (recv(sock, state->buf, state->msize, 0) != state->msize) {
				perror("bw_udp client: recv failed");
				exit(5);
			}
			nbytes -= state->msize;
		}
	}
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
server_main()
{
	char	*buf = (char*)valloc(MAX_MSIZE);
	int     sock, namelen, seq = 0;
	long	nbytes, msize;
	struct sockaddr_in it;

	GO_AWAY;

	sock = udp_server(UDP_XACT, SOCKOPT_NONE);

	while (1) {
		namelen = sizeof(it);
		if (recvfrom(sock, (void*)buf, 2 * sizeof(long), 0, 
		    (struct sockaddr*)&it, &namelen) < 0) {
			fprintf(stderr, "bw_udp server: recvfrom: got wrong size\n");
			exit(9);
		}
		nbytes = ntohl(*(long*)buf);
		msize = ntohl(*((long*)buf + 1));
		while (nbytes > 0) {
			if (sendto(sock, (void*)buf, msize, 0, 
				   (struct sockaddr*)&it, sizeof(it)) < 0) {
				perror("bw_udp sendto");
				exit(9);
			}
			nbytes -= msize;
		}
	}
}


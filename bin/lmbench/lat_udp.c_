/*
 * udp_xact.c - simple UDP transaction latency test
 *
 * Three programs in one -
 *	server usage:	lat_udp -s
 *	client usage:	lat_udp [-P <parallelism>] [-W <warmup>] [-N <repetitions>] hostname
 *	shutdown:	lat_udp -S hostname
 *
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char	*id = "$Id$\n";
#include "bench.h"

#define MAX_MSIZE (10 * 1024 * 1024)

static void	client_lat_udp_main(int ac, char **av);
static void	server_lat_udp_main();
static void	timeout();
static void	init(iter_t iterations, void* cookie);
static void	cleanup(iter_t iterations, void* cookie);
static void    doit(iter_t iterations, void* cookie);

typedef struct _state {
	int	sock;
	int	seq;
	int	msize;
	char	*server;
	char	*buf;
} state_t;


int
lat_udp_main(int ac, char **av)
{
	state_t state;
	int	c;
	int	parallel = 1;
	int	warmup = 0;
	int	repetitions = TRIES;
	int	server = 0;
	int	shutdown = 0;
	int	msize = 4;
 	char	buf[256];
	char	*usage = "-s\n OR [-S] [-m <message size>] [-P <parallelism>] [-W <warmup>] [-N <repetitions>] server\n NOTE: message size must be >= 4\n";

	if (sizeof(int) != 4) {
		fprintf(stderr, "lat_udp: Wrong sequence size\n");
		return(1);
	}

	while (( c = getopt(ac, av, "sS:m:P:W:N:")) != EOF) {
		switch(c) {
		case 's': /* Server */
			if (fork() == 0) {
				server_lat_udp_main();
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
			msize = atoi(optarg);
			if (msize < sizeof(int)) {
				lmbench_usage(ac, av, usage);
				msize = 4;
			}
			if (msize > MAX_MSIZE) {
				lmbench_usage(ac, av, usage);
				msize = MAX_MSIZE;
			}
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

	if (optind + 1 != ac) {
		lmbench_usage(ac, av, usage);
	}

	state.server = av[optind];
	state.msize = msize;
	benchmp(init, doit, cleanup, SHORT, parallel, 
		warmup, repetitions, &state);
	sprintf(buf, "UDP latency using %s", state.server);
	micro(buf, get_n());
	exit(0);
}

static void
init(iter_t iterations, void* cookie)
{
	state_t *state = (state_t *) cookie;

	if (iterations) return;

	state->sock = udp_connect(state->server, UDP_XACT, SOCKOPT_NONE);
	state->seq = 0;
	state->buf = (char*)malloc(state->msize);
	
	signal(SIGALRM, timeout);
	alarm(15);
}

static void
doit(iter_t iterations, void *cookie)
{
	state_t *state = (state_t *) cookie;
	int seq = state->seq;
	int net = htonl(seq);
	int sock = state->sock;
	int ret;

	alarm(15);
	while (iterations-- > 0) {
		*(int*)state->buf = htonl(seq++);
		if (send(sock, state->buf, state->msize, 0) != state->msize) {
			perror("lat_udp client: send failed");
			exit(5);
		}
		if (recv(sock, state->buf, state->msize, 0) != state->msize) {
			perror("lat_udp client: recv failed");
			exit(5);
		}
	}
	state->seq = seq;
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
timeout()
{
	fprintf(stderr, "Recv timed out\n");
	exit(1);
}

static void
server_lat_udp_main()
{
	char	*buf = (char*)valloc(MAX_MSIZE);
	int     sock, sent, namelen, seq = 0;
	struct sockaddr_in it;

	GO_AWAY;

	sock = udp_server(UDP_XACT, SOCKOPT_REUSE);

	while (1) {
		int nbytes;
		namelen = sizeof(it);
		if ((nbytes = recvfrom(sock, (void*)buf, MAX_MSIZE, 0, 
		    (struct sockaddr*)&it, &namelen)) < 0) {
			fprintf(stderr, "lat_udp server: recvfrom: got wrong size\n");
			exit(9);
		}
		sent = ntohl(*(int*)buf);
		if (sent < 0) {
			udp_done(UDP_XACT);
			exit(0);
		}
		if (sent != ++seq) {
			seq = sent;
		}
		*(int*)buf = htonl(seq);
		if (sendto(sock, (void*)buf, nbytes, 0, 
		    (struct sockaddr*)&it, sizeof(it)) < 0) {
			perror("lat_udp sendto");
			exit(9);
		}
	}
}

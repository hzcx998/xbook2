/*
 * lat_http.c - simple HTTP transaction latency test
 *
 * usage: lat_http hostname [port] < filelist
 *
 * Copyright (c) 1994-6 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char	*id = "$Id$\n";

#include "bench.h"

static char	*buf;
static int	debug;
static int	echo;

static int
http(char *server, char *file, int prog)
{
	int     sock;
	int     n;
	int	b = 0;

	sock = tcp_connect(server, prog, SOCKOPT_REUSE);
	sprintf(buf, "GET /%s HTTP/1.0\r\n\r\n\n", file);
	if (debug) {
		printf(buf);
	}
	write(sock, buf, strlen(buf));
	while ((n = read(sock, buf, XFERSIZE)) > 0) {
		b += n;
		if (echo) {
			write(1, buf, n);
		}
	}
	close(sock);
	if (debug) {
		printf("Got %d\n", b);
	}
	return (b);
}

static void
killhttp(char *server, int prog)
{
	int     sock;

	sock = tcp_connect(server, prog, SOCKOPT_REUSE);
	write(sock, "EXIT", 4);
	close(sock);
}

static void chop(register char *s) { while (*s && *s != '\n') s++; *s = 0; }

int
lat_http_main(int ac, char **av)
{
	char	*server;
	int     i, prog;
	int	c;
	int	shutdown = 0;
	uint64	total = 0;
	uint64	usecs = 0;
	double	avg;
	char	*name = av[0];
	char	file[1024];
	char	*usage = "[-d] [-e] [-S] serverhost [port] < list\n";

	while (( c = getopt(ac, av, "deS")) != EOF) {
		switch(c) {
		case 'd':
			debug++;
			break;
		case 'e':
			echo++;
			break;
		case 'S': /* shutdown serverhost */
			shutdown = 1;
			break;
		default:
			lmbench_usage(ac, av, usage);
			break;
		}
	}
	
	if (optind >= ac || optind < ac - 2) {
		lmbench_usage(ac, av, usage);
		exit(0);
	}
	server = av[optind++];

	if (optind < ac && atoi(av[optind]) != 0) {
		prog = -atoi(av[optind]);
	} else {
		prog = -80;
	}

	if (shutdown) {
		killhttp(server, prog);
		exit(0);
	}

	i = 0;
	buf = valloc(XFERSIZE);
	bzero(buf, XFERSIZE);
	while (fgets(file, sizeof(file), stdin)) {
		chop(file);
		start(0);
		total += http(server, file, prog);
		usecs += stop(0,0);
		i++;
	}
	avg = total;
	avg /= (i - 1);
	if (avg > 1000) {
		avg /= 1000;
		fprintf(stderr, "Avg xfer: %.1fKB, ", avg);
	} else {
		fprintf(stderr, "Avg xfer %d, ", (int)avg);
	}
	settime(usecs);
	latency((uint64)1, total);
	exit(0);
}


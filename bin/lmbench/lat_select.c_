/*
 * lat_select.c - time select system call
 *
 * usage: lat_select [-P <parallelism>] [-W <warmup>] [-N <repetitions>] [n]
 *
 * Copyright (c) 1996 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 */
static char	*id = "$Id$\n";

#include "bench.h"

static void initialize(iter_t iterations, void *cookie);
static void cleanup(iter_t iterations, void *cookie);
static void doit(iter_t iterations, void *cookie);
static void writer(int w, int r);
static void server(void* cookie);

typedef int (*open_f)(void* cookie);
int  open_file(void* cookie);
int  open_socket(void* cookie);

typedef struct _state {
	char	fname[L_tmpnam];
	open_f	fid_f;
	pid_t	pid;
	int	sock;
	int	fid;
	int	num;
	int	max;
	fd_set  set;
} state_t;

int
lat_select_main(int ac, char **av)
{
	state_t state;
	int parallel = 1;
	int warmup = 0;
	int repetitions = TRIES;
	int c;
	char* usage = "[-n <#descriptors>] [-P <parallelism>] [-W <warmup>] [-N <repetitions>] file|tcp\n";
	char	buf[256];

	morefds();  /* bump fd_cur to fd_max */
	state.num = 200;
	while (( c = getopt(ac, av, "P:W:N:n:")) != EOF) {
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
		case 'n':
			state.num = bytes(optarg);
			break;
		default:
			lmbench_usage(ac, av, usage);
			break;
		}
	}

	if (optind + 1 != ac) {
		lmbench_usage(ac, av, usage);
	}

	if (streq("tcp", av[optind])) {
		state.fid_f = open_socket;
		server(&state);
		benchmp(initialize, doit, cleanup, 0, parallel, 
			warmup, repetitions, &state);
		sprintf(buf, "Select on %d tcp fd's", state.num);
		kill(state.pid, SIGKILL);
		waitpid(state.pid, NULL, 0);
		micro(buf, get_n());
	} else if (streq("file", av[optind])) {
		state.fid_f = open_file;
		server(&state);
		benchmp(initialize, doit, cleanup, 0, parallel, 
			warmup, repetitions, &state);
		unlink(state.fname);
		sprintf(buf, "Select on %d fd's", state.num);
		micro(buf, get_n());
	} else {
		lmbench_usage(ac, av, usage);
	}

	exit(0);
}

static void
server(void* cookie)
{
	int pid;
	state_t* state = (state_t*)cookie;

	pid = getpid();
	state->pid = 0;

	if (state->fid_f == open_file) {
		/* Create a temporary file for clients to open */
		sprintf(state->fname, "lat_selectXXXXXX");
		state->fid = mkstemp(state->fname);
		if (state->fid <= 0) {
			char buf[L_tmpnam+128];
			sprintf(buf, "lat_select: Could not create temp file %s", state->fname);
			perror(buf);
			exit(1);
		}
		close(state->fid);
		return;
	}

	/* Create a socket for clients to connect to */
	state->sock = tcp_server(TCP_SELECT, SOCKOPT_REUSE);
	if (state->sock <= 0) {
		perror("lat_select: Could not open tcp server socket");
		exit(1);
	}

	/* Start a server process to accept client connections */
	switch(state->pid = fork()) {
	case 0:
		/* child server process */
		while (pid == getppid()) {
			int newsock = tcp_accept(state->sock, SOCKOPT_NONE);
			read(newsock, &state->fid, 1);
			close(newsock);
		}
		exit(0);
	case -1:
		/* error */
		perror("lat_select::server(): fork() failed");
		exit(1);
	default:
		break;
	}
}

int
open_socket(void* cookie)
{
	return tcp_connect("localhost", TCP_SELECT, SOCKOPT_NONE);
}

int
open_file(void* cookie)
{
	state_t* state = (state_t*)cookie;

	return open(state->fname, O_RDONLY);
}

static void
doit(iter_t iterations, void * cookie)
{
	state_t * 	state = (state_t *)cookie;
	fd_set		nosave;
	static struct timeval tv;
	static count = 0;
	
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	while (iterations-- > 0) {
		nosave = state->set;
		select(state->num, 0, &nosave, 0, &tv);
	}
}

static void
initialize(iter_t iterations, void *cookie)
{
	char	c;
	state_t * state = (state_t *)cookie;
	int	n, last = 0 /* lint */;
	int	N = state->num, fid, fd;

	if (iterations) return;

	fid = (*state->fid_f)(cookie);
	if (fid <= 0) {
		perror("Could not open device");
		exit(1);
	}
	state->max = 0;
	FD_ZERO(&(state->set));
	for (n = 0; n < N; n++) {
		fd = dup(fid);
		if (fd == -1) break;
		if (fd > state->max)
			state->max = fd;
		FD_SET(fd, &(state->set));
	}
	state->max++;
	close(fid);
	if (n != N)
		exit(1);
}

static void
cleanup(iter_t iterations, void *cookie)
{
	int	i;
	state_t * state = (state_t *)cookie;

	if (iterations) return;

	for (i = 0; i <= state->max; ++i) {
		if (FD_ISSET(i, &(state->set)))
			close(i);
	}
	FD_ZERO(&(state->set));
}

	     

/*
 * lat_rpc.c - simple RPC transaction latency test
 *
 * Four programs in one -
 *	server usage:	lat_rpc -s
 *	client usage:	lat_rpc hostname
 *	client usage:	lat_rpc -p tcp hostname
 *	client usage:	lat_rpc -p udp hostname
 *	shutdown:	lat_rpc -S hostname
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

#ifdef HAVE_RPC_H

static void	client_lat_rpc_main(int ac, char **av);
static void	server_lat_rpc_main();
static void	benchmark(iter_t iterations, void* _state);
char	*client_rpc_xact_1(char *argp, CLIENT *clnt);

static void
doit(CLIENT *cl, char *server)
{
	char	c = 1;
	char	*resp;
	
	resp = client_rpc_xact_1(&c, cl);
	if (!resp) {
		clnt_perror(cl, server);
		exit(1);
	}
	if (*resp != 123) {
		fprintf(stderr, "lat_rpc: got bad data\n");
		exit(1);
	}
}


/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 0, 25000 };

char	*proto[] = { "tcp", "udp", 0 };

typedef struct state_ {
	int	msize;
	char	*server;
	char	*protocol;
	CLIENT	*cl;
} state_t;

static void
initialize(iter_t iterations, void* cookie)
{
	struct	timeval tv;
	state_t *state = (state_t*)cookie;

	if (iterations) return;

	state->cl = clnt_create(state->server, XACT_PROG, XACT_VERS, 
				state->protocol);
	if (!state->cl) {
		clnt_pcreateerror(state->server);
		exit(1);
	}
	if (strcasecmp(state->protocol, proto[1]) == 0) {
		tv.tv_sec = 0;
		tv.tv_usec = 2500;
		if (!clnt_control(state->cl, CLSET_RETRY_TIMEOUT, (char *)&tv)) {
			clnt_perror(state->cl, "setting timeout");
			exit(1);
		}
	}
}

static void
benchmark(iter_t iterations, void* _state)
{
	state_t* state = (state_t*)_state;
	char	buf[256];

	while (iterations-- > 0) {
		doit(state->cl, state->server);
	}
}

int
lat_rpc_main(int ac, char **av)
{
	int	i;
	int 	c;
	int	parallel = 1;
	int	warmup = 0;
	int	repetitions = TRIES;
	int	server = 0;
	int	shutdown = 0;
	state_t	state;
	CLIENT	*cl;
	char	buf[1024];
	char	*protocol = NULL;
	char	*usage = "-s\n OR [-p <tcp|udp>] [-P parallel] [-W <warmup>] [-N <repetitions>] serverhost\n OR -S serverhost\n";

	state.msize = 1;

	while (( c = getopt(ac, av, "sS:m:p:P:W:N:")) != EOF) {
		switch(c) {
		case 's': /* Server */
			if (fork() == 0) {
				server_lat_rpc_main();
			}
			exit(0);
		case 'S': /* shutdown serverhost */
		{
			cl = clnt_create(optarg, XACT_PROG, XACT_VERS, "udp");
			if (!cl) {
				clnt_pcreateerror(state.server);
				exit(1);
			}
			clnt_call(cl, RPC_EXIT, (xdrproc_t)xdr_void, 0, 
				  (xdrproc_t)xdr_void, 0, TIMEOUT);
			exit(0);
		}
		case 'm':
			state.msize = atoi(optarg);
			break;
		case 'p':
			protocol = optarg;
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

	state.server = av[optind++];

	if (protocol == NULL || !strcasecmp(protocol, proto[0])) {
		state.protocol = proto[0];
		benchmp(initialize, benchmark, NULL, MEDIUM, parallel, 
			warmup, repetitions, &state);
		sprintf(buf, "RPC/%s latency using %s", proto[0], state.server);
		micro(buf, get_n());
	}

	if (protocol == NULL || !strcasecmp(protocol, proto[1])) {
		state.protocol = proto[1];
		benchmp(initialize, benchmark, NULL, MEDIUM, parallel, 
			warmup, repetitions, &state);
		sprintf(buf, "RPC/%s latency using %s", proto[1], state.server);
		micro(buf, get_n());
	}
		
	exit(0);
}

char *
client_rpc_xact_1(char *argp, CLIENT *clnt)
{
	static char res;

	bzero((void*)&res, sizeof(res));
	if (clnt_call(clnt, RPC_XACT, (xdrproc_t)xdr_char,
	    argp, (xdrproc_t)xdr_char, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}

/*
 * The remote procedure[s] that will be called
 */
/* ARGSUSED */
char	*
rpc_xact_1(msg, transp)
     	char	*msg;
	register SVCXPRT *transp;
{
	static char r = 123;

	return &r;
}

static void xact_prog_1();

static void
server_lat_rpc_main()
{
	register SVCXPRT *transp;

	GO_AWAY;

	(void) pmap_unset(XACT_PROG, XACT_VERS);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		fprintf(stderr, "cannot create udp service.\n");
		exit(1);
	}
	if (!svc_register(transp, XACT_PROG, XACT_VERS, xact_prog_1, IPPROTO_UDP)) {
		fprintf(stderr, "unable to register (XACT_PROG, XACT_VERS, udp).\n");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		fprintf(stderr, "cannot create tcp service.\n");
		exit(1);
	}
	if (!svc_register(transp, XACT_PROG, XACT_VERS, xact_prog_1, IPPROTO_TCP)) {
		fprintf(stderr, "unable to register (XACT_PROG, XACT_VERS, tcp).\n");
		exit(1);
	}

	svc_run();
	fprintf(stderr, "svc_run returned\n");
	exit(1);
	/* NOTREACHED */
}

static void
xact_prog_1(rqstp, transp)
	struct svc_req *rqstp;
	register SVCXPRT *transp;
{
	union {
		char rpc_xact_1_arg;
	} argument;
	char *result;
	bool_t (*xdr_argument)(), (*xdr_result)();
	char *(*local)();

	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void) svc_sendreply(transp, (xdrproc_t)xdr_void, (char *)NULL);
		return;

	case RPC_XACT:
		xdr_argument = xdr_char;
		xdr_result = xdr_char;
		local = (char *(*)()) rpc_xact_1;
		break;

	case RPC_EXIT:
		(void) svc_sendreply(transp, (xdrproc_t)xdr_void, (char *)NULL);
		(void) pmap_unset(XACT_PROG, XACT_VERS);
		exit(0);

	default:
		svcerr_noproc(transp);
		return;
	}
	bzero((char *)&argument, sizeof(argument));
	if (!svc_getargs(transp, (xdrproc_t)xdr_argument, (char*)&argument)) {
		svcerr_decode(transp);
		return;
	}
	result = (*local)(&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, (xdrproc_t)xdr_result, result)) {
		svcerr_systemerr(transp);
	}
	if (!svc_freeargs(transp, (xdrproc_t)xdr_argument, (char*)&argument)) {
		fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
	return;
}
#endif 
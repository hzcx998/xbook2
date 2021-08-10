/*
 * lat_usleep.c - usleep duration/latency
 *
 * The APIs for usleep(3), nanosleep(2), select(2), pselect(2),
 * getitimer(2) and setitimer(2) support resolutions down to 
 * a micro-second.  However, many implementations do not support 
 * such resolution.  Most current implementations (as of Fall 
 * 2002) simply put the process back on the run queue and the 
 * process may get run on the next scheduler time slice (10-20 
 * milli-second resolution).
 *
 * This benchmark measures the true latency from the timer/sleep
 * call to the resumption of program execution.  If the timers
 * actually worked properly, then the latency would be identical
 * to the requested duration, or a little longer, so the input
 * and output figures would be nearly identical.  In most current
 * implementations the value is rounded up to the next scheduler
 * timeslice (e.g., a resolution of 20 milli-seconds, with all
 * values rounded up).
 *
 * usage: lat_usleep [-u | -i] [-P <parallelism>] [-W <warmup>] \
 *		[-N <repetitions>] usecs
 *
 * Copyright (c) 2002 Carl Staelin.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char           *id = "$Id$\n";

#include "bench.h"
#include <sched.h>

typedef     enum {USLEEP, NANOSLEEP, SELECT, PSELECT, ITIMER} timer_e;

static uint64          caught,
                n;
struct itimerval value;

typedef struct _state {
    unsigned long usecs;
} state_t;

static void
bench_usleep(iter_t iterations, void *cookie)
{
    state_t        *state = (state_t*)cookie;

    while (iterations-- > 0) {
	usleep(state->usecs);
    }
}

static void
bench_nanosleep(iter_t iterations, void *cookie)
{
    state_t        *state = (state_t*)cookie;
    struct timespec req;
    struct timespec rem;

    req.tv_sec = 0;
    req.tv_nsec = state->usecs * 1000;

    while (iterations-- > 0) {
	if (nanosleep(&req, &rem) < 0) {
	    while (nanosleep(&rem, &rem) < 0)
		;
	}
    }
}

static void
bench_select(iter_t iterations, void *cookie)
{
    state_t        *state = (state_t*)cookie;
    struct timeval  tv;

    while (iterations-- > 0) {
	tv.tv_sec = 0;
	tv.tv_usec = state->usecs;
	select(0, NULL, NULL, NULL, &tv);
    }
}

#ifdef _POSIX_SELECT
static void
bench_pselect(iter_t iterations, void *cookie)
{
    state_t        *state = (state_t*)cookie;
    struct timespec ts;

    while (iterations-- > 0) {
	ts.tv_sec = 0;
	ts.tv_nsec = state->usecs * 1000;
	pselect(0, NULL, NULL, NULL, &ts, NULL);
    }
}
#endif /* _POSIX_SELECT */

static void
interval()
{
    if (++caught == n) {
	caught = 0;
	n = benchmp_interval(benchmp_getstate());
    }

    setitimer(ITIMER_REAL, &value, NULL);
}

static void
initialize(iter_t iterations, void *cookie)
{
    state_t        *state = (state_t*)cookie;
    struct sigaction sa;

    if (iterations) return;

    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = state->usecs;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = state->usecs;

    sa.sa_handler = interval;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, 0);
}

static void
bench_itimer(iter_t iterations, void *cookie)
{
    n = iterations;
    caught = 0;

    /*
     * start the first timing interval 
     */
    start(0);

    /*
     * create the first timer, causing us to jump to interval() 
     */
    setitimer(ITIMER_REAL, &value, NULL);

    while (1) {
	sleep(100000);
    }
}
/*
int
set_realtime()
{
    struct sched_param sp;

    sp.sched_priority = sched_get_priority_max(SCHED_RR);
    if (sched_setscheduler(0, SCHED_RR, &sp) >= 0) return TRUE;
    perror("sched_setscheduler");
    return FALSE;
}
*/
int
lat_usleep_main(int ac, char **av)
{
    int             realtime = 0;
    int		    parallel = 1;
    int             warmup = 0;
    int             repetitions = TRIES;
    int             c;
    char            buf[512];
    timer_e	    what = USLEEP;
    state_t         state;
    char           *scheduler = "";
    char           *mechanism = "usleep";
    char           *usage = "[-r] [-u <method>] [-P <parallelism>] [-W <warmup>] [-N <repetitions>] usecs\nmethod=usleep|nanosleep|select|pselect|itimer\n";

    realtime = 0;

    while ((c = getopt(ac, av, "ru:W:N:")) != EOF) {
	switch (c) {
	case 'r':
	    realtime = 1;
	    break;
	case 'u':
	    if (strcmp(optarg, "usleep") == 0) {
		what = USLEEP;
		mechanism = "usleep";
	    } else if (strcmp(optarg, "nanosleep") == 0) {
		what = NANOSLEEP;
		mechanism = "nanosleep";
	    } else if (strcmp(optarg, "select") == 0) {
		what = SELECT;
		mechanism = "select";
#ifdef _POSIX_SELECT
	    } else if (strcmp(optarg, "pselect") == 0) {
		what = PSELECT;
		mechanism = "pselect";
#endif /* _POSIX_SELECT */
	    } else if (strcmp(optarg, "itimer") == 0) {
		what = ITIMER;
		mechanism = "itimer";
	    } else {
		lmbench_usage(ac, av, usage);
	    }
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
    if (optind != ac - 1) {
	lmbench_usage(ac, av, usage);
    }

    state.usecs = bytes(av[optind]);
    // if (realtime && set_realtime()) scheduler = "realtime ";

    switch (what) {
    case USLEEP:
	benchmp(NULL, bench_usleep, NULL, 
		0, parallel, warmup, repetitions, &state);
	break;
    case NANOSLEEP:
	benchmp(NULL, bench_nanosleep, NULL, 
		0, parallel, warmup, repetitions, &state);
	break;
    case SELECT:
	benchmp(NULL, bench_select, NULL, 
		0, parallel, warmup, repetitions, &state);
	break;
#ifdef _POSIX_SELECT
    case PSELECT:
	benchmp(NULL, bench_pselect, NULL, 
		0, parallel, warmup, repetitions, &state);
	break;
#endif /* _POSIX_SELECT */
    case ITIMER:
	benchmp(initialize, bench_itimer, NULL, 
		0, parallel, warmup, repetitions, &state);
	break;
    default:
	lmbench_usage(ac, av, usage);
	break;
    }
    sprintf(buf, "%s%s %lu microseconds", scheduler, mechanism, state.usecs);
    micro(buf, get_n());
    return (0);
}

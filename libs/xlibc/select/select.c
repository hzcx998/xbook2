#include <sys/select.h>
#include <signal.h>
#include <stdint.h>
#include <errno.h>
#include <sys/syscall.h>

int select(int n, fd_set *restrict rfds, fd_set *restrict wfds, fd_set *restrict efds, struct timeval *restrict tv)
{
	time_t s = tv ? tv->tv_sec : 0;
	suseconds_t us = tv ? tv->tv_usec : 0;
	long ns;
	const time_t max_time = (1ULL<<8*sizeof(time_t)-1)-1;

	if (s<0 || us<0) return (-EINVAL);
	if (us/1000000 > max_time - s) {
		s = max_time;
		us = 999999;
		ns = 999999999;
	} else {
		s += us/1000000;
		us %= 1000000;
		ns = us*1000;
	}

	return syscall(SYS_SELECT, n, rfds, wfds, efds,
		tv ? ((long[]){s, us}) : 0);
}

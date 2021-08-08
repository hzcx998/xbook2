#include <poll.h>
#include <time.h>
#include <signal.h>

int poll(struct pollfd *fds, nfds_t n, int timeout)
{
#ifdef SYS_POLL
	return syscall3(int, SYS_POLL, fds, n, timeout);
#endif
	return -1;
}

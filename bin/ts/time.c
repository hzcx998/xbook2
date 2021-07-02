#include "test.h"
#include <sys/times.h>
#include <sys/timex.h>
#include <sys/sysinfo.h>

int test_time(int argc, char *argv[])
{
    struct timespec tp;
    printf("clock_gettime: %d\n", clock_gettime(CLOCK_REALTIME, &tp));
    printf("clock_gettime: sec: %d, nsec: %d\n", tp.tv_sec, tp.tv_nsec);
    tp.tv_sec += 100;
    printf("clock_settime: %d\n", clock_settime(CLOCK_REALTIME, &tp));
    printf("clock_gettime: %d\n", clock_gettime(CLOCK_REALTIME, &tp));
    printf("clock_gettime: sec: %d, nsec: %d\n", tp.tv_sec, tp.tv_nsec);
    tp.tv_sec += 100;
    printf("clock_settime: %d\n", clock_settime(CLOCK_REALTIME, &tp));
    printf("clock_gettime: %d\n", clock_gettime(CLOCK_REALTIME, &tp));
    printf("clock_gettime: sec: %d, nsec: %d\n", tp.tv_sec, tp.tv_nsec);

    struct tms tmsbuf;
    printf("%s: %d\n", $(times), times(&tmsbuf));
    printf("%s: %d %d %d %d\n", $(times), tmsbuf.tms_cstime, tmsbuf.tms_cutime, tmsbuf.tms_stime, tmsbuf.tms_utime);
    
    struct timex tmx;
    printf("%s: %d\n", $(adjtimex), adjtimex(&tmx));

    struct sysinfo info;

	if (sysinfo(&info) < 0) {
		printf("sysinfo failed");
		return -1;
	}

	printf("uptime    %d\n", info.uptime);
	printf("loads %d, %d, %d\n",
	       info.loads[0] / (1 << SI_LOAD_SHIFT),
	       info.loads[1] / (1 << SI_LOAD_SHIFT),
	       info.loads[2] / (1 << SI_LOAD_SHIFT));
	printf("totalram  %d\n", info.totalram );
	printf("freeram   %d\n", info.freeram  );
	printf("sharedram %d\n", info.sharedram);
	printf("bufferram %d\n", info.bufferram);
	printf("totalswap %d\n", info.totalswap);
	printf("freeswap  %d\n", info.freeswap );
	printf("procs     %d\n",  info.procs    );
	printf("totalhigh %d\n", info.totalhigh);
	printf("freehigh  %d\n", info.freehigh );
	printf("mem_unit  %d\n",  info.mem_unit );

    return 0;
}

#include <time.h>
#include <sys/time.h>
#include <utime.h>
#include <string.h>
#include <sys/srvcall.h>
#include <srv/filesrv.h>
#include <sys/dir.h>

time_t time(time_t *t)
{
    time_t time;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    time = ts.tv_sec;
    if (t)
        *t = time;
    return time;
}

int utime(const char *path, const struct utimbuf *times)
{
    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);

    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_UTIME, 0);
    SETSRV_ARG(&srvarg, 1, full_path, strlen(full_path) + 1);
    SETSRV_ARG(&srvarg, 2, times->actime, 0);
    SETSRV_ARG(&srvarg, 3, times->modtime, 0);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}
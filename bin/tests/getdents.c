#include "test.h"

char buf[512];
int getdents_test(int argc, char* argv[])
{
    TEST_START(__func__);
    int fd, nread;
    struct linux_dirent64 *dirp64;
    dirp64 = (struct linux_dirent64 *)buf;
    //fd = open(".", O_DIRECTORY);
    fd = open(".", O_RDONLY);
    printf("open fd:%d\n", fd);

	nread = getdents(fd, dirp64, 512);
	printf("getdents fd:%d\n", nread);
	assert(nread != -1);
	printf("getdents success.\n%s\n", dirp64->d_name);

	/*
	for(int bpos = 0; bpos < nread;){
	    d = (struct dirent *)(buf + bpos);
	    printf(  "%s\t", d->d_name);
	    bpos += d->d_reclen;
	}
	*/

    printf("\n");
    close(fd);
    TEST_END(__func__);
    return 0;
}

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define BUF_SIZE 1024

int getdents_test2(int argc, char* argv[])
{
    int fd;
    long nread;
    char buf[BUF_SIZE];
    struct linux_dirent64 *d;
    char d_type;

    fd = open(argc > 1 ? argv[1] : "./bin", O_RDONLY | O_DIRECTORY);
    if (fd == -1)
        handle_error("open");

    for (;;) {
        nread = getdents(fd, (struct linux_dirent64 *)buf, BUF_SIZE);
        if (nread == -1)
            handle_error("getdents");

        if (nread == 0)
            break;

        printf("--------------- nread=%d ---------------\n", nread);
        printf("inode#    file type  d_reclen  d_off   d_name\n");
        for (long bpos = 0; bpos < nread;) {
            d = (struct linux_dirent64 *) (buf + bpos);
            printf("%8ld  ", d->d_ino);
            d_type = d->d_type;
            printf("%-10s ", (d_type == DT_REG) ?  "regular" :
                            (d_type == DT_DIR) ?  "directory" :
                            (d_type == DT_FIFO) ? "FIFO" :
                            (d_type == DT_SOCK) ? "socket" :
                            (d_type == DT_LNK) ?  "symlink" :
                            (d_type == DT_BLK) ?  "block dev" :
                            (d_type == DT_CHR) ?  "char dev" : "???");
            printf("%4d %10jd  %s\n", d->d_reclen,
                    (intmax_t) d->d_off, d->d_name);
            bpos += d->d_reclen;
        }
    }

    exit(EXIT_SUCCESS);
    return 0;
}

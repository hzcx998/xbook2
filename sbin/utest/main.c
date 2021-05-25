#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/walltime.h>

void t()
{
    char b;
    int c[30];
    while (1) {
        
    }
}

/* 栈溢出测试 */
int fib(int n)
{
    char buf[4096] = {0};
    if (n < 2)
        return 1;
    return fib(n - 1) + fib(n - 2);
}

static inline void sys_err(char *str)
{
    printf("sys err: %s\n", str);
    exit(-1);
}

int file_test1()
{
    int fd = open("tmp.txt", O_CREAT | O_RDWR);
    if (fd < 0)
        sys_err("open file failed!");
    printf("open file %d ok\n", fd);
    pid_t pid = fork();
    if (pid < 0)
        sys_err("fork failed!");
    if (pid > 0) {
        char *str1 = "hello, parent!\n";
        int i = 0;
        while (i < 10) {
            i++;
            printf("%d ready write %s\n", getpid(), str1);
            if (write(fd, str1, strlen(str1)) > 0)
                printf("parent wirte:%s\n", str1);
        }
        close(fd);
        printf("parent write done\n");
    } else {
        char *str2 = "hello, child!\n";
        int j = 0;
        while (j < 10) {
            j++;
            printf("%d ready write %s\n", getpid(), str2);
            if (write(fd, str2, strlen(str2)) > 0)
                printf("child wirte:%s\n", str2);
        }
        close(fd);
        printf("child write done\n");
        _exit(0);
    }
    while (1);
}

int file_test2()
{
    char *buf = malloc(64*1024);
    if (buf == NULL) {
        printf("malloc for test failed!\n");
        return -1;
    }
    memset(buf, 0, 64*1024);
    while (1)
    {
        int fd = open("/bin/sh", O_RDONLY);
        if (fd < 0) {
            printf("open file failed!\n");
            break;
        }
        while (1)
        {
            int rd = read(fd, buf, 64*1024);
            printf("read %d.\n", rd);
            if (rd <= 0)
                break;
        }
        close(fd);
        printf("read done.\n");
        break;
    }
    free(buf);
    printf("test end\n");
    return 0;
}

int file_test3()
{
    int fd = open("tmp.txt", O_CREAT | O_RDWR);
    if (fd < 0)
        sys_err("open file failed!");
    char *buf = (char *) 0x1000;
    int wr = write(fd, buf, 4096*5);
    printf("write: %d\n", wr);
    close(fd);
}

int file_test4()
{
    FILE *fp = NULL;
    fp = fopen("/test2.txt", "wb");
    if (!fp) {
        printf("fopen failed\n");
        return -1;
    }
    fwrite("hello", 5, 1, fp);
    fclose(fp);


    printf("test dir\n");

    rmdir("/testdir/abc/def");
    rmdir("/testdir/abc");
    rmdir("/testdir/qwq");
    rmdir("/testdir2");
    rmdir("/testdir");
    
    assert(!mkdir("/testdir", 0666));
    assert(!mkdir("/testdir2", 0666));
    assert(!mkdir("/testdir/abc", 0666));
    assert(!mkdir("/testdir/abc/def", 0666));
    assert(mkdir("/testdir/def/efg", 0666));
    
    char buf[32] = {0};
    getcwd(buf, 32);
    printf("cwd: %s\n", buf);
    printf("chdir: %d\n", chdir("/testdir"));
    getcwd(buf, 32);
    printf("cwd: %s\n", buf);
    assert(!mkdir("qwq", 0666));
    printf("chdir: %d\n", chdir("qwq"));
    getcwd(buf, 32);
    printf("cwd: %s\n", buf);
    printf("chdir: %d\n", chdir(".."));
    getcwd(buf, 32);
    printf("cwd: %s\n", buf);
    printf("chdir: %d\n", chdir("/"));
    getcwd(buf, 32);
    printf("cwd: %s\n", buf);
    
    printf("test end\n");
    return 0;
}


int pipe_test()
{
    printf("----pipe test----\n");
    int fd[2];
    int ret = pipe(fd);
    if (ret < 0) {
        printf("create pipe failed!\n");
        return -1;
    }
    int pid = fork();
    if (pid == -1)
        return -1; 
    /* 父进程读取子进程传递的数据 */

    if (pid > 0) {
        
        printf("I am parent %d, my child %d.\n", getpid(), pid);
        close(fd[1]);
        /* 读取数据 */
        char buf[32];
        memset(buf, 0, 32);
        read(fd[0], buf, 32);
        printf("parent read: %s\n", buf);

        close(fd[0]);
    } else {
        
        printf("I am child %d, my parent %d.\n", getpid(), getppid());
        close(fd[0]);
        write(fd[1], "hello, pipe!\n", strlen("hello, pipe!\n"));
        printf("child write done!\n");
        close(fd[1]);
    }
    return 0;
}

int sched_test()
{
    while (1)
    {
        printf("hello, ");
        sched_yield();
        printf("world!\n");
    }
}

int can_set_alarm = 0;

int alarm_test()
{
    int i = 1;
    can_set_alarm = 1;
    //signal(SIGALRM, );
    while (i <= 5)
    {
        if (can_set_alarm) {
            alarm(i);
            printf("set %ds.\n", i);
            can_set_alarm = 0;
        }
    }
}

int time_test()
{
    walltime_t wt;
    walltime(&wt);
    printf("time: %d/%d/%d %d:%d:%d\n", 
        wt.year, wt.month, wt.day, wt.hour, wt.minute, wt.second);
    clock_t ticks = getticks();
    printf("ticks: %d\n", ticks);
    
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    printf("gettimeofday: tv sec=%ld, usec=%ld, tz dsttime=%ld, minuteswest=%ld\n",
        tv.tv_sec, tv.tv_usec, tz.tz_dsttime, tz.tz_minuteswest);
    
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf("clock_gettime: CLOCK_MONOTONIC: ts sec=%ld nsec=%ld\n",
        ts.tv_sec, ts.tv_nsec);
    clock_gettime(CLOCK_REALTIME, &ts);
    printf("clock_gettime: CLOCK_REALTIME: ts sec=%ld nsec=%ld\n",
        ts.tv_sec, ts.tv_nsec);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    printf("clock_gettime: CLOCK_PROCESS_CPUTIME_ID: ts sec=%ld nsec=%ld\n",
        ts.tv_sec, ts.tv_nsec);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
    printf("clock_gettime: CLOCK_THREAD_CPUTIME_ID: ts sec=%ld nsec=%ld\n",
        ts.tv_sec, ts.tv_nsec);
    
    printf("usleep: 100 ms\n");
    usleep(100 * 1000);
    printf("usleep: 1000 ms\n");
    usleep(1000 * 1000);
    printf("usleep: 3 s\n");
    usleep(1000 * 1000 * 3);
    
    struct tms tms;
    times(&tms);
    printf("times: cstime=%lx cutime=%lx stime=%lx utime=%lx\n", 
        tms.tms_cstime, tms.tms_cutime, tms.tms_stime, tms.tms_utime);

    char hostname[64];
    gethostname(hostname, 64);
    printf("gethostname: hostname %s\n", hostname);
    
}


int main()
{
    #if 0
    int a = 0;
    char buf[32] = {0};
    int c = a + 10;
    t();
    while (1) {
        
    }
    #endif
    #if 1
    /* 打开tty，用来进行基础地输入输出 */
    int tty0 = open("/dev/con0", O_RDONLY);
    if (tty0 < 0) {
        while (1)
        {
            /* code */
        }
        
        return -1;
    }
    
    int tty1 = open("/dev/con0", O_WRONLY);
    if (tty1 < 0) {
        while (1)
        {
            /* code */
        }
        close(tty0);
        return -1;
    }
    int tty2 = dup(tty1);
    
    //printf("[INIT]: start.\n");
    char *str = "[INIT]: start.\n";
    write(tty1, str, strlen(str));
    

    //file_test1();
    //file_test2();
    //file_test3();
    //file_test4();
    //pipe_test();
    //sched_test();
    //alarm_test();
    time_test();
    while (1)
    {
        /* code */
    }
    


    /* exec测试 */
    char *argv[3] = {"/sbin/test1","arg1", 0};
    char *env[3] = {"env0", "env1",0};
    // execve("/sbin/init", argv, env);    
    #if 0
    pid_t child = create_process(argv, env, 0);
    printf("create %d\n", child);
    waitpid(child, NULL, 0);
    printf("Wait done\n");
        
    while (1)
    {
        /* code */
    }
    
    #endif
    #if 0
    pid_t pid1 = fork();
    if (pid1 > 0) {
        write(tty1, "I am parent\n", 12);
        int state;
        waitpid(pid1, &state, 0);
        write(tty1, "Wait done\n", 9);
        while (1)
        {
            /* code */
        }
    } else {
        write(tty1, "I am child\n", 11);
        execve("/sbin/test1", argv, env);    
        _exit(0);
    }
    
    while (1)
    {
        /* code */
    }
    #endif
    /*====内存测试===*/
    brk(NULL);
    char *mbuf = sbrk(1024);
    memset(mbuf, 0, 1024);

    char *v = (char *) 0x00001000;
    *v = 0x10;

    printf("hello, world!\n");
    int n = fib(10);
    printf("fib:%d\n", n);
    
    char *mbuf0 = malloc(32);
    char *mbuf1 = malloc(512);
    char *mbuf2 = malloc(4096*2);
    printf("malloc:%p %p %p\n", mbuf0, mbuf1, mbuf2);
    free(mbuf0);
    free(mbuf1);
    free(mbuf2);
    printf("test done!\n");

    // printf("hello, world!\n");
    pid_t pid = fork();
    if (pid > 0) {
        printf("I am parent, pid=%d child=%d\n", getpid(), pid);
        int state;
        waitpid(pid, &state, 0);
        write(tty1, "Wait done\n", 9);
    } else {
        printf("I am child, pid=%d parent=%d\n", getpid(), getppid());
        sleep(3);
        printf("sleep done\n");
    }
    pid = fork();
    if (pid > 0) {
        printf("I am parent, pid=%d child=%d\n", getpid(), pid);
        int state;
        waitpid(pid, &state, 0);
    } else {
        printf("I am child, pid=%d parent=%d\n", getpid(), getppid());
    }
    #endif
    _exit(0);
    while (1)
    {
        /* code */
    }
    
    return 0;
}

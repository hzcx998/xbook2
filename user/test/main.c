#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
#include <assert.h>
#include  <errno.h>
#include  <semaphore.h>
#include  <arpa/inet.h>

#include <srv/guisrv.h>

#include <sys/srvcall.h>
#include <sys/proc.h>
#include <sys/res.h>
#include <sgi/sgi.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "cond_sem.h"
#if 0

#include <lua.h>
#include <lstate.h>
#include <lauxlib.h>
#include <lualib.h>

#if 1

/* 测试的Lua代码字符串 */
const char lua_test[] = { 
    "print(\"Hello,I am lua!\\n--this is newline printf\")\n"
    "function foo()\n"
    "  local i = 0\n"
    "  local sum = 1\n"
    "  while i <= 10 do\n"
    "    sum = sum * 2\n"
    "    i = i + 1\n"
    "  end\n"
    "return sum\n"
    "end\n"
    "print(\"sum =\", foo())\n"
    "print(\"and sum = 2^11 =\", 2 ^ 11)\n"
    "print(\"exp(200) =\", math.exp(200))\n"
};
#else
/* 测试的Lua代码字符串 */
const char lua_test[] = { 
    "print(\"Hello,I am lua!\\n--this is newline printf\")\n"
};

#endif
 
/* 运行Lua */
int main2(int argc, char *argv[])
{
    
    lua_State *L;
    
    L = luaL_newstate(); /* 建立Lua运行环境 */
    if (L == NULL) {
        printf("luaL_newstate failed!\n");
    }
    printf("luaL_newstate done.\n");
    luaL_openlibs(L);
    
    printf("luaL_openlibs done.\n");
    luaopen_base(L);
    
    printf("luaopen_base done.\n");
    luaL_dostring(L, lua_test); /* 运行Lua脚本 */
    
    luaL_dofile(L, "/test.lua");

    printf("luaL_dostring done.\n");
    lua_close(L);
    printf("lua_close done.\n");
    while (1)
    {
        /* code */
    }
    
    return 0;
}
#endif

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>

#if 1
int main(int argc, char *argv[])
{
    printf("hello, test.\n");

    char *p = malloc(8);
    if (!p)
        return -1;
    memset(p, 1, 8);
    printf("realloc\n");
    p = realloc(p, 64);
    printf("data: %x %x %x\n", p[0], p[7], p[63]);
    free(p);
    
    p = malloc(128);
    if (!p)
        return -1;
    memset(p, 2, 128);

    p = realloc(p, 64);
    printf("data: %x %x %x\n", p[0], p[63], p[127]);
    free(p);
    
    p = malloc(1024);
    if (!p)
        return -1;
    memset(p, 3, 1024);
    free(p);
    
    p = malloc(1024*10);
    if (!p)
        return -1;
    memset(p, 4, 1024*10);
    free(p);
    
    p = malloc(1024*1024);
    if (!p)
        return -1;
    memset(p, 5, 1024*1024);
    free(p);
    
    p = malloc(1024*1024*10);
    if (!p)
        return -1;
    memset(p, 6, 1024*1024*10);
    p = realloc(p, 1024*1024*20);
    printf("data: %x %x %x\n", p[0], p[1024*1024*10-1], p[1024*1024*20-1]);
    free(p);
    
    p = malloc(1024*1024);
    if (!p)
        return -1;
    memset(p, 5, 1024*1024);
    free(p);
    
    p = malloc(1024*10);
    if (!p)
        return -1;
    memset(p, 4, 1024*10);
    free(p);

    p = malloc(128);
    if (!p)
        return -1;
    memset(p, 2, 128);

    p = realloc(p, 64);
    printf("data: %x %x %x\n", p[0], p[63], p[127]);
    free(p);

    printf("test done.\n");
    return 0;
}
#endif


#if 0

//#define SOCKET_UDP_SERVER_TEST

#ifdef SOCKET_UDP_SERVER_TEST

#define UDPPORT 8080
#define UDPSERV "192.168.0.105"
#define UDPSERV2 "192.168.0.104"

int main()
{
    printf("[test] start.\n");
    sleep(3);
    int ret;
    printf("[test] socket.\n");
    int servfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (servfd < 0) {
        printf("create socket failed!\n");
        return -1;
    }
    printf("[test] socket ok.\n");

    struct sockaddr_in client_addr;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(UDPPORT);
    addr.sin_addr.s_addr = inet_addr(UDPSERV);

    int serv_addr_len = sizeof(struct sockaddr_in);
    int client_addr_len = sizeof(struct sockaddr_in);
    
    ret = bind(servfd, (struct sockaddr *)&addr, serv_addr_len);
    if (ret < 0) {
        printf("bind socket failed!\n");
        sockclose(servfd);
        return -1;
    }

    char buf[512];
    int recvbytes;
    while (1)
    {
        memset(buf, 0, 512);
        recvbytes = recvfrom(servfd, buf, 512, 0, ( struct sockaddr *)&client_addr, &client_addr_len);
        if (recvbytes < 0) {
            printf("[test] recv failed!\n");
            break;
        }

        printf("recv len=%d, %s.\n", recvbytes, buf);
        printf("client len %d, addr %x port %d.\n", client_addr_len, client_addr.sin_addr.s_addr, client_addr.sin_port);
        
        memset(buf, 0, 512);
        strcpy(buf, "get data!\n\r");
        recvbytes = sendto(servfd, buf, strlen(buf), 0,  (struct sockaddr *)&client_addr, client_addr_len);
        if (recvbytes < 0) {
            printf("[test] send failed!\n");
            //break;
        }
        printf("send len=%d.\n", recvbytes);
        usleep(50000);
    }
    if (sockclose(servfd) < 0) {
        printf("close socket failed!\n");
        
    }

    while (1)
    {
        /* code */
    }
    

    return 0;
}
#else

#define UDPPORT 8081
#define UDPSERV2 "192.168.0.104"

int main()
{
    printf("[test] start.\n");
    sleep(3);
    int ret;
    printf("[test] socket.\n");
    int servfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (servfd < 0) {
        printf("create socket failed!\n");
        return -1;
    }
    printf("[test] socket ok.\n");

    struct sockaddr_in client_addr;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(UDPPORT);
    addr.sin_addr.s_addr = inet_addr(UDPSERV2);
    //printf("addr: len=%d family=%d port=%d addr=%x\n", addr.sin_len, addr.sin_family, addr.sin_port, addr.sin_addr.s_addr);

    memset(&client_addr, 0, sizeof(addr));
    client_addr.sin_len = sizeof(addr);
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(8080);
    client_addr.sin_addr.s_addr = inet_addr("192.168.0.105");
    ret = bind(servfd, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_in));
    if (ret < 0) {
        printf("bind socket failed!\n");
        return -1;
    }
    int serv_addr_len = sizeof(struct sockaddr_in);

    char buf[512];
    int recvbytes;
    while (1)
    {
        printf("addr: len=%d family=%d port=%x addr=%x\n", addr.sin_len, addr.sin_family, addr.sin_port, addr.sin_addr.s_addr);
    
        memset(buf, 0, 512);
        strcpy(buf, "get data!\n\r");
        recvbytes = sendto(servfd, buf, strlen(buf), 0,  (struct sockaddr *)&addr, serv_addr_len);
        if (recvbytes < 0) {
            printf("[test] send failed!\n");
            //break;
        }
        printf("send len=%d.\n", recvbytes);

        usleep(100000);

        memset(buf, 0, 512);
        recvbytes = recvfrom(servfd, buf, 512, 0, ( struct sockaddr *)&addr, &serv_addr_len);
        if (recvbytes < 0) {
            printf("[test] recv failed!\n");
            break;
        }

        printf("recv len=%d, %s.\n", recvbytes, buf);
        printf("client len %d, addr %x port %d.\n", serv_addr_len, addr.sin_addr.s_addr, addr.sin_port);
    }
    if (sockclose(servfd) < 0) {
        printf("close socket failed!\n");
    }

    while (1)
    {
        /* code */
    }
    

    return 0;
}
#endif
#endif


#if 0

//#define SOCKET_SERVER_TEST

#ifdef SOCKET_SERVER_TEST
int main()
{
    printf("[test] start.\n");
    sleep(3);
    int ret;
    printf("[test] socket.\n");
    int socket_id = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_id < 0) {
        printf("create socket failed!\n");
        return -1;
    }
    printf("[test] socket ok.\n");


    int opt_val;
    socklen_t opt_len = sizeof(opt_val);
    
    if(getsockopt(socket_id, SOL_SOCKET, SO_TYPE, &opt_val, &opt_len) < 0)
    {
        perror("fail to getsockopt");
    }
    
    printf("optval = %x\n", opt_val);



    struct sockaddr_in other_addr;

    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr_in);

    memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("192.168.0.105");

    ret = bind(socket_id, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if (ret < 0) {
        printf("bind socket failed!\n");
        sockclose(socket_id);
        return -1;
    }

    ret = listen(socket_id, 2);
    if (ret < 0) {
        printf("listen socket failed!\n");
        sockclose(socket_id);
        return -1;
    }
    int client_socket;
    client_socket = accept(socket_id, (struct sockaddr *)&other_addr, &len);
    if (client_socket < 0) {
        printf("accept socket failed!\n");
        sockclose(socket_id);
        return -1;
    }
    printf("accept ok, client ip %x port %d, addr len %d.\n", other_addr.sin_addr, other_addr.sin_port, len);

    char buf[512];
    int recvbytes;

    struct sockaddr_in myaddr;
    socklen_t mylen = sizeof(struct sockaddr_in );

    while (1)
    {
        memset(buf, 0, 512);
        recvbytes = sockread(client_socket, buf, 512);
        if (recvbytes < 0) {
            printf("[test] recv failed!\n");
            break;
        }
        printf("recv %s.\n", buf);
        recvbytes = sockwrite(client_socket, "get data", 8);
        if (recvbytes < 0) {
            printf("[test] send failed!\n");
            break;
        }

        if (getsockname(socket_id, (struct sockaddr *)&myaddr, &mylen) < 0) {
            printf("[test] get sock name failed!\n");
        }
        printf("my ip %x port %d\n", ntohl(myaddr.sin_addr.s_addr), ntohs(myaddr.sin_port));
        
        if (getpeername(client_socket, (struct sockaddr *)&myaddr, &mylen) < 0) {
            printf("[test] get sock name failed!\n");
        }
        printf("client ip %x port %d\n", ntohl(myaddr.sin_addr.s_addr), ntohs(myaddr.sin_port));

        //usleep(50000);
    }

    sleep(3);
    printf("sleep 3 seconds over.\n");
    sockclose(client_socket);
    if (sockclose(socket_id) < 0) {
        printf("close socket failed!\n");
        
    }

    while (1)
    {
        /* code */
    }
    

    return 0;
}
#else
int main()
{
    printf("[test] start.\n");
    sleep(3);

    int fd = open("c:/test.txt", O_RDONLY, 0);
    ioctl(fd, 0, 0);
    fcntl(fd, 0, 0);
    close(fd);

    int ret;
    printf("[test] socket.\n");
    int socket_id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_id < 0) {
        printf("create socket failed!\n");
        return -1;
    }
    printf("[test] socket ok.\n");


    struct sockaddr_in addr;
    size_t len;

    memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("192.168.0.104");

    ret = connect(socket_id, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if (ret < 0) {
        printf("connect socket failed!\n");
        sockclose(socket_id);
        return -1;
    }

    char buf[512];
    int recvbytes;

    struct sockaddr_in myaddr;
    socklen_t mylen = sizeof(struct sockaddr_in );
    // shutdown(socket_id, SHUT_WR);

    while (1)
    {
        memset(buf, 0, 512);
        recvbytes = write(socket_id, "get data", 8);
        if (recvbytes < 0) {
            printf("[test] send failed!\n");
            break;
        }
        
        recvbytes = read(socket_id, buf, 512);
        if (recvbytes < 0) {
            printf("[test] recv failed!\n");
            break;
        }
        res_write(1, 0, buf, 128);

        if (getsockname(socket_id, (struct sockaddr *)&myaddr, &mylen) < 0) {
            printf("[test] get sock name failed!\n");
        }
        printf("ip %x port %d\n", ntohl(myaddr.sin_addr.s_addr), ntohs(myaddr.sin_port));

        //usleep(50000);
    }

    printf("connect ok, sleep 3 seconds.\n");
        
        
    sleep(3);
    printf("sleep 3 seconds over.\n");
    
    if (sockclose(socket_id) < 0) {
        printf("close socket failed!\n");
        
    }

    while (1)
    {
        /* code */
    }
    

    return 0;
}

#endif
#endif

#if 0
pthread_mutex_t mutex;
pthread_cond_t cond;

int ticks = 0;

void *thread1(void *arg)
{
    pthread_cleanup_push(pthread_mutex_unlock, &mutex);
    //提供函数回调保护
    while (1) {
        //printf("thread1 is running\n");
        pthread_mutex_lock(&mutex);
        
        pthread_cond_wait(&cond, &mutex);
        ticks++;
        //usleep(100);
        //printf("thread1 applied the condition\n");
        pthread_mutex_unlock(&mutex);
        //sleep(4);
        
    }
    pthread_cleanup_pop(0);
}

void *thread2(void *arg)
{
    while (1) {
        //printf("thread2 is running\n");
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        /*printf("thread2 applied the condition\n");
        printf("thread %d: ticks %d\n", pthread_self(), ticks);*/
        //usleep(100);
        pthread_mutex_unlock(&mutex);
        //sleep(1);
        
    }
}

int main()
{
    pthread_t thid1, thid2;
    printf("condition variable study!\n");
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&thid1, NULL, (void *) thread1, NULL);
    pthread_create(&thid2, NULL, (void *) thread2, NULL);
    do {
        pthread_cond_signal(&cond);
        usleep(1000);
    } while (1);
    sleep(3);
    pthread_exit(0);
    return 0;
}
#endif
#if 0

pthread_mutex_t mutex;

void *thread_a();
void *thread_b();

int ticks = 0;

int main(int argc, char *argv[])
{
    pthread_mutex_init(&mutex, NULL);

    pthread_t t0, t1, t2, t3, t4; 
    
    pthread_create(&t0, NULL, thread_a, NULL);
    pthread_create(&t1, NULL, thread_a, NULL);
    pthread_create(&t2, NULL, thread_b, NULL);
    pthread_create(&t3, NULL, thread_b, NULL);
    pthread_create(&t4, NULL, thread_b, NULL);

    pthread_join(t0, NULL);

    return 0;
}

void *thread_a()
{
    printf("thread %d: start.\n", pthread_self());
        
    while (1)
    {
        pthread_mutex_lock(&mutex);
        ticks++;
        pthread_mutex_unlock(&mutex);
        usleep(10);   
    }
}

void *thread_b()
{
    printf("thread %d: start.\n", pthread_self());
    
    while (1)
    {
        pthread_mutex_lock(&mutex);
        printf("thread %d: ticks %d.\n", pthread_self(), ticks);
        pthread_mutex_unlock(&mutex);
        usleep(10);
    }
}
#endif

#if 0
sem_t sema;
void *thread_a();
void *thread_b();

int ticks = 0;

int main(int argc, char *argv[])
{
    sem_init(&sema, 0, 1);

    pthread_t t0, t1, t2, t3, t4; 
    
    pthread_create(&t0, NULL, thread_a, NULL);
    pthread_create(&t1, NULL, thread_a, NULL);
    pthread_create(&t2, NULL, thread_b, NULL);
    pthread_create(&t3, NULL, thread_b, NULL);
    pthread_create(&t4, NULL, thread_b, NULL);
    pthread_join(t0, NULL);
    return 0;
}

void *thread_a()
{
    while (1)
    {
        //printf("thread %d: ticks %d.\n", pthread_self(), ticks);
        sem_wait(&sema);
        ticks++;
        sem_post(&sema);
        usleep(10);
    }
}


void *thread_b()
{
    while (1)
    {
        //printf("thread %d: ticks %d.\n", pthread_self(), ticks);
        sem_wait(&sema);
        //printf("thread %d: ticks %d.\n", pthread_self(), ticks);
        sem_post(&sema);
        usleep(100);
    }
}

#endif

#if 0

#define	NBUFF	 8
#define BUFFSIZE 128
 
struct {	/* data shared by producer and consumer */
  struct {
    char	data[BUFFSIZE];			/* a buffer */
    ssize_t	n;						/* count of #bytes in the buffer */
  } buff[NBUFF];					/* NBUFF of these buffers/counts */
  sem_t nempty, nfull;		/* semaphores, not pointers */
  sem_t writer_mutex, reader_mutex;
} shared;
 
int writer_index = 0, reader_index = 0;
 
int		fd;							/* input file to copy to stdout */
void* produce(void *), *consume(void *);
void* produce_tryP(void *arg);
 
 
int main(int argc, char **argv)
{
    pthread_t tid_produce1, tid_produce2, tid_produce3;
    pthread_t tid_consume1, tid_consume2;
 
    if (argc != 2)
    {
        printf("use <pathname> as pramater \n");
        exit(1);
    }
 
    fd = open(argv[1], O_RDONLY);
    if( fd == -1 )
    {
        printf("cann't open the file\n");
        return -1;
    }
 
    printf("init sem.\n");
        
    sem_init(&shared.writer_mutex, 0,1);
    sem_init(&shared.reader_mutex, 0,1);
    sem_init(&shared.nempty, 0,NBUFF);
    sem_init(&shared.nfull, 0,0);
 
    /*
    pthread_init(&tid_produce1);
    pthread_init(&tid_produce2);
    pthread_init(&tid_produce3);
    pthread_init(&tid_consume1);
    pthread_init(&tid_consume2);
 
    pthread_create(&tid_consume1, NULL, consume);
    pthread_create(&tid_consume2, NULL, consume);
    pthread_create(&tid_produce1, NULL, produce);
    pthread_create(&tid_produce2, NULL, produce);
    pthread_create(&tid_produce3, NULL, produce_tryP);
 
    pthread_start(&tid_consume1, NULL);
    pthread_start(&tid_consume2, NULL);
    pthread_start(&tid_produce1, NULL);
    pthread_start(&tid_produce2, NULL);
    pthread_start(&tid_produce3, NULL);
    */
    printf("create thread.\n");
    
    pthread_create(&tid_produce1, NULL, produce, NULL);
    pthread_create(&tid_produce2, NULL, produce, NULL);
    pthread_create(&tid_produce3, NULL, produce_tryP, NULL);
    pthread_create(&tid_consume1, NULL, consume, NULL);
    pthread_create(&tid_consume2, NULL, consume, NULL);
    
    printf("join thread.\n");
    
    pthread_join(tid_consume1, NULL);
    pthread_join(tid_consume2, NULL);
 
    pthread_join(tid_produce1, NULL);
    pthread_join(tid_produce2, NULL);
    pthread_join(tid_produce3, NULL);

    printf("destroy sem.\n");
    
 /*
    pthread_destroy(tid_consume1);
    pthread_destroy(tid_consume2);
    pthread_destroy(tid_produce1);
    pthread_destroy(tid_produce2);
    pthread_destroy(tid_produce3);
 */
 
    sem_destroy(&shared.writer_mutex);
    sem_destroy(&shared.reader_mutex);
    sem_destroy(&shared.nempty);
    sem_destroy(&shared.nfull);
 

    close(fd);
    printf("test end.\n");
    exit(0);
    return 0;
}

void *produce(void *arg)
{
    while( 1 )
    {
        sem_wait(&shared.nempty);	/* wait for at least 1 empty slot */
 
        sem_wait(&shared.writer_mutex);
 
        shared.buff[writer_index].n =
                read(fd, shared.buff[writer_index].data, BUFFSIZE);
        //printf("read.\n");

        if( shared.buff[writer_index].n <= 0 )
        {
            sem_post(&shared.nfull);
            sem_post(&shared.writer_mutex);
            return NULL;
        }
 
        writer_index = (writer_index+1)%NBUFF;
 
        sem_post(&shared.nfull);
        sem_post(&shared.writer_mutex);
    }
 
    return NULL;
}
 
 
void* produce_tryP(void *arg)
{
    int status;
    while( 1 )
    {
        /* wait for at least 1 empty slot */
        while( 1 )
        {
            status = sem_trywait(&shared.nempty);
            if( status == 0 )
                break;
            else if( status == EAGAIN )
            {
                //usleep(10*1000); //sleep 10 毫秒
                clock_t start = clock();
                while ((clock() - start) < 5)
                {
                }
                continue;
            }
            else
                return NULL;
        }
 
        sem_wait(&shared.writer_mutex);
 
        shared.buff[writer_index].n =
                read(fd, shared.buff[writer_index].data, BUFFSIZE);
 
        if( shared.buff[writer_index].n <= 0 )
        {
            sem_post(&shared.nfull);
            sem_post(&shared.writer_mutex);
            return NULL;
        }
 
        writer_index = (writer_index+1)%NBUFF;
 
        sem_post(&shared.nfull);
        sem_post(&shared.writer_mutex);
    }
 
    return NULL;
}

void* consume(void *arg)
{
    while( 1 )
    {
        sem_wait(&shared.nfull);
        sem_wait(&shared.reader_mutex);
 
        if( shared.buff[reader_index].n <= 0)
        {
            sem_post(&shared.nempty);
            sem_post(&shared.reader_mutex);
            return NULL;
        }

        //printf("write.\n");
        /*
        res_write(STDOUT_FILENO, 0, shared.buff[reader_index].data,
                shared.buff[reader_index].n);*/
        //printf("%s\n", shared.buff[reader_index].data);

        reader_index = (reader_index+1)%NBUFF;
 
        sem_post(&shared.nempty);
        sem_post(&shared.reader_mutex);
    }
 
    return NULL;
}
#endif
#if 0

int main(int argc, char *argv[])
{
    printf("hello, test!\n");
    sleep(1);
    
    SGI_Display *display = SGI_OpenDisplay();
    if (display == NULL) {
        printf("[test] open gui failed!\n");
        return -1;
    }
    printf("[test] open display ok!\n");

    SGI_Window win = SGI_CreateSimpleWindow(
        display,
        display->root_window,
        10,
        100,
        320,
        240,
        0XffFAFA55
    );

    if (win < 0) {
        printf("[test] create window failed!\n");
    }
    printf("[test] create window success!\n");

    SGI_SetWMName(display, win, "new title, 123 abc #$");

    static SGI_Argb icon[5*5*4];
    int i;
    for (i = 0; i < 5*5; i++) {
        icon[i] = SGI_RGB(i * 20, i* 15, i* 5);
    }
    SGI_SetWMIcon(display, win, icon, 5, 5);

    if (SGI_MapWindow(display, win)) {
        printf("[test] map window failed!\n");
    } else {
        printf("[test] map window success!\n");
    }
    int x, y;
    for (y = 0; y < 10; y++) {
        for (x = 0; x < 320; x++) {
            SGI_DrawPixel(display, win, x, y, SGIC_RED);
        }
    }
    
    SGI_DrawRect(display, win, 50, 50, 100, 50, SGIC_BLUE);
    SGI_DrawFillRect(display, win, 70, 100, 50, 100, SGIC_GREEN);


    SGI_Argb pixmap[10*10*sizeof(SGI_Argb)];
    for (y = 0; y < 10; y++) {
        for (x = 0; x < 10; x++) {
            pixmap[y * 10 + x] = SGI_ARGB(0xff, x * 10, x * 15, y * 10);
        }
    }
    SGI_DrawPixmap(display, win, 100, 200, 10, 10, pixmap);
    
    SGI_DrawString(display, win, 100, 50, "hello, text!\nabc\n\rdef", 30, SGIC_BLUE);

    if (SGI_SetFont(display, win, SGI_LoadFont(display, "standard-8*16")) < 0) {
        printf("[test] set font failed!\n");
    }

    SGI_DrawString(display, win, 100, 200, "hello, text!\nabc\n\rdef", 30, SGIC_BLUE);
    
    if (SGI_UpdateWindow(display, win, 0, 0, 320, 240))
        printf("[test] update window failed!\n");
    else
        printf("[test] update window success!\n");

    
    printf("[test] window handle %d\n", win);

    SGI_SelectInput(display, win, SGI_ButtonPressMask | SGI_ButtonRleaseMask |
        SGI_KeyPressMask | SGI_KeyRleaseMask | SGI_EnterWindow | SGI_LeaveWindow);

    SGI_Event event;
    SGI_Window event_window;
    while (1) {
        if (SGI_NextEvent(display, &event))
            continue;
        
        event_window = SGI_DISPLAY_EVENT_WINDOW(display);
        // printf("[test] event window %d\n", event_window);
        switch (event.type)
        {
        case SGI_MOUSE_BUTTON:
            if (event.button.state == SGI_PRESSED) {    // 按下
                if (event.button.button == 0) {
                    printf("[test] left button pressed.\n");
                } else if (event.button.button == 1) {
                    printf("[test] middle button pressed.\n");
                } else if (event.button.button == 2) {
                    printf("[test] right button pressed.\n");
                }
            } else {
                if (event.button.button == 0) {
                    printf("[test] left button released.\n");
                } else if (event.button.button == 1) {
                    printf("[test] middle button released.\n");
                } else if (event.button.button == 2) {
                    printf("[test] right button released.\n");
                }
            }
            break;
        case SGI_MOUSE_MOTION:
            if (event.motion.state == SGI_ENTER) {
                printf("[test] mouse enter window motion %d, %d.\n", event.motion.x, event.motion.y);
            } else if (event.motion.state == SGI_LEAVE) {
                printf("[test] mouse leave window motion %d, %d.\n", event.motion.x, event.motion.y);
            } else {
                printf("[test] mouse motion %d, %d.\n", event.motion.x, event.motion.y);
            }
            break;
        case SGI_KEY:
            if (event.key.state == SGI_PRESSED) {
                printf("[test] keyboard key pressed [%x, %c] modify %x.\n", event.key.keycode.code, 
                    event.key.keycode.code, event.key.keycode.modify);
                
                
            } else {
                printf("[test] keyboard key released [%x, %c] modify %x.\n", event.key.keycode.code, 
                    event.key.keycode.code, event.key.keycode.modify);
                if (event.key.keycode.code == SGIK_Q || event.key.keycode.code == SGIK_q) {
                    goto exit_gui;
                }
            }
            break;
        case SGI_QUIT:
            printf("[test] get quit event.\n");
            goto exit_gui;
            break;
        default:
            break;
        }
    }
    sleep(1);
exit_gui:
    
    if (SGI_UnmapWindow(display, win) < 0) {
        printf("[test] unmap window failed!\n");
    } else {
        printf("[test] unmap window success!\n");
    }

    if (SGI_DestroyWindow(display, win) < 0) {
        printf("[test] destroy window failed!\n");
    } else {
        printf("[test] destroy window success!\n");
    }
    
    SGI_CloseDisplay(display);
    printf("[test] close display ok!\n");

    return 0;
}

#endif

#if 0
DIR *sys_list_dir(char* path)
{
    struct dirent *de;
    int i;
    DIR *dir = opendir(path);
    if (dir) {
        while (1) {
            /* 读取目录项 */
            if ((de = readdir(dir)) == NULL)
                break;
            
            if (de->d_attr & DE_DIR) {   /* 是目录，就需要递归扫描 */
                printf("%s/%s\n", path, de->d_name);
                /* 构建新的路径 */
                i = strlen(path);
                sprintf(&path[i], "/%s", de->d_name);
                sys_list_dir(path);

                path[i] = 0;
            } else {    /* 直接列出文件 */
                printf("%s/%s  size=%d\n", path, de->d_name, de->d_size);
            }
        }
        closedir(dir);
        return NULL;
    }
    return dir;
}

int main(int argc, char *argv[])
{
    printf("hello, test!\n");
    printf("this is string: %s %c value:%d %x!\n", "abc", 'A', 123456789, 0x1234abcd);
#if 0
    /* 文件操作测试 */
    int fd = open("c:/gcc", O_CREAT | O_RDWR);
    if (fd < 0) {
        printf("open file failed!\n");
    }

    char *str = "hello, test!";
    int wr = write(fd, str, strlen(str));
    if (wr < 0) {
        printf("write file failed!\n");
    }

    printf("write %d bytes.\n", wr);

    lseek(fd, 0, SEEK_SET);

    char buf[32] = {0};
    read(fd, buf, 12);
    printf("read buf:%s\n", buf);

    if (fsync(fd))
        printf("> fsync failed!\n");

    ftruncate(fd, 5);

    //lseek(fd, 0, SEEK_SET);
    rewind(fd);

    memset(buf, 0, 32);
    printf("read bytes:%d\n", read(fd, buf, 12));

    printf("read buf:%s\n", buf);

    if (fchmod(fd, 0))
        printf("fchmod failed!\n");

    printf("tell file pos:%d fsize:%d\n", tell(fd), _size(fd));

    close(fd);

    char path[MAX_PATH] = {0};
    strcpy(path, "c:");
    sys_list_dir(path);

    /*
    if (mkfs("ram0", "fat16", 0)) {
        printf("make fs failed!\n");
    }*/

    if (mount("ram0", "d:", "fat16", 0)) {
        printf("mount fs failed!\n");
    }

    fd = open("d:/test", O_CREAT | O_RDWR);
    if (fd < 0) {
        printf("open file failed!\n");
    }

    char *str2 = "hello, ram!\n";
    write(fd, str2, strlen(str2));

    rewind(fd);

    memset(buf, 0, 32);

    read(fd, buf, 32);

    printf("buf:%s\n", buf);

    fsync(fd);
    close(fd);

    memset(path, 0, MAX_PATH);
    strcpy(path, "d:");
    sys_list_dir(path);

    if (unmount("d:",  0)) {
        printf("unmount fs failed!\n");
    }

    fd = open("d:/test", O_CREAT | O_RDWR);
    if (fd < 0) {
        printf("open file after unmount failed!\n");
    }
#endif


    char cwd[32];
    getcwd(cwd, 32);
    printf("cwd:%s\n", cwd);

    /* 生成绝对路径测试 */
    char abs_path[MAX_PATH] = {0};
    build_path("c:/abc", abs_path);
    printf("path:%s\n", abs_path);

    build_path("/abc", abs_path);
    printf("path:%s\n", abs_path);
    
    if (mkdir("c:/xbook", 0))
        printf("mk root failed!\n");

    chdir("c:/xbook");

    build_path("def", abs_path);
    printf("path:%s\n", abs_path);
    
    build_path("./abc", abs_path);
    printf("path:%s\n", abs_path);
    
    if (mkdir("c:/xbook/log", 0))
        printf("mk xbook/log failed!\n");

    chdir("c:/xbook/log");
    
    build_path("../abc/def", abs_path);
    printf("path:%s\n", abs_path);
    
    build_path("/abc/", abs_path);
    printf("path:%s\n", abs_path);
    

    build_path("abc/", abs_path);
    printf("path:%s\n", abs_path);


    /* 文件操作测试 */
    int fd = open("nasm", O_CREAT | O_RDWR);
    if (fd < 0) {
        printf("open file failed!\n");
    }

    char *str = "hello, nasm!";
    int wr = write(fd, str, strlen(str));
    if (wr < 0) {
        printf("write file failed!\n");
    }

    printf("write %d bytes.\n", wr);

    lseek(fd, 0, SEEK_SET);

    char buf[32] = {0};
    read(fd, buf, 12);
    printf("read buf:%s\n", buf);

    printf("tell file pos:%d fsize:%d\n", tell(fd), _size(fd));

    close(fd);

    char path[MAX_PATH] = {0};
    strcpy(path, "c:");
    sys_list_dir(path);

#if 0
    if (mkdir("c:/tmp", 0) < 0)
        printf("mkdir failed!\n");

    if (mkdir("c:/tmp/test", 0) < 0)
        printf("mkdir failed!\n");

    if (mkdir("c:/share", 0) < 0)
        printf("mkdir failed!\n");

    memset(path, 0, MAX_PATH);
    strcpy(path, "c:");
    sys_list_dir(path);

    if (rmdir("c:/share"))
        printf("rmdir share failed!\n");
    
    if (unlink("c:/tmp"))  
        printf("unlink tmp failed!\n");

    if (unlink("c:/tmp/test"))  
        printf("unlink tmp/test failed!\n");
    
    if (unlink("c:/tmp"))  
        printf("unlink tmp failed!\n");

    if (unlink("c:/gcc"))  
        printf("unlink gcc failed!\n");


    if (rename("c:/tmp", "c:/tmp2"))  
        printf("rename tmp failed!\n");

    if (rename("c:/tmp2/test", "c:/tmp2/app"))  
        printf("rename tmp2/test failed!\n");

    if (rename("c:/null", "c:/dev"))  
        printf("rename null failed!\n");

    const struct utimbuf utimebuf = {0, 0x12345678};
    if (utime("c:/gcc", &utimebuf))
        printf("utime failed!\n");

    if (chmod("c:/gcc", 0))
        printf("chmod failed!\n");

    struct stat sbuf;
    memset(&sbuf, 0, sizeof(struct stat));
    if (stat("c:/gcc", &sbuf) < 0)
        printf("stat failed!\n");

    printf("state: size=%d, date=%x, time=%x, attr=%x\n", 
        sbuf.st_size, sbuf.st_date, sbuf.st_time, sbuf.st_attr);




    //sys_list_dir(path);

    fd = open("c:/gcc", O_RDWR);
    if (fd < 0) {
        printf("open file failed!\n");
    }
    printf("ready read file:\n");
    char ch;
    while (!_eof(fd)) {
        read(fd, &ch, 1);
        printf("%c", ch);
    }
    close(fd);
#endif






    int i = 0;
    
    char cha;
    while ((cha = getchar()) != '\n')
    {
        putchar(cha);
    }

    return 0;
}
#endif
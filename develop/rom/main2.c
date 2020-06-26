#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
#include <assert.h>

#include <srv/guisrv.h>
#include <sys/srvcall.h>
#include <sys/proc.h>
#include <sys/res.h>
#include <sgi/sgi.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/dir.h>

#include "cond_sem.h"
#include  <sys/types.h>
#include  <errno.h>

 
#define	NBUFF	 8
#define BUFFSIZE 4096
 
 
struct {	/* data shared by producer and consumer */
  struct {
    char	data[BUFFSIZE];			/* a buffer */
    ssize_t	n;						/* count of #bytes in the buffer */
  } buff[NBUFF];					/* NBUFF of these buffers/counts */
  cond_sem_t nempty, nfull;		/* semaphores, not pointers */
  cond_sem_t writer_mutex, reader_mutex;
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
        
    cond_sem_init(&shared.writer_mutex, 1);
    cond_sem_init(&shared.reader_mutex, 1);
    cond_sem_init(&shared.nempty, NBUFF);
    cond_sem_init(&shared.nfull, 0);
 
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
 
    cond_sem_destroy(&shared.writer_mutex);
    cond_sem_destroy(&shared.reader_mutex);
    cond_sem_destroy(&shared.nempty);
    cond_sem_destroy(&shared.nfull);
 
    printf("test end.\n");
    exit(0);
    return 0;
}
 
 
 
void *produce(void *arg)
{
    while( 1 )
    {
        cond_sem_p(&shared.nempty);	/* wait for at least 1 empty slot */
 
        cond_sem_p(&shared.writer_mutex);
 
        shared.buff[writer_index].n =
                read(fd, shared.buff[writer_index].data, BUFFSIZE);
        //printf("read.\n");

        if( shared.buff[writer_index].n == 0 )
        {
            cond_sem_v(&shared.nfull);
            cond_sem_v(&shared.writer_mutex);
            return NULL;
        }
 
        writer_index = (writer_index+1)%NBUFF;
 
        cond_sem_v(&shared.nfull);
        cond_sem_v(&shared.writer_mutex);
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
            status = cond_sem_tryP(&shared.nempty);
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
 
        cond_sem_p(&shared.writer_mutex);
 
        shared.buff[writer_index].n =
                read(fd, shared.buff[writer_index].data, BUFFSIZE);
 
        if( shared.buff[writer_index].n == 0 )
        {
            cond_sem_v(&shared.nfull);
            cond_sem_v(&shared.writer_mutex);
            return NULL;
        }
 
        writer_index = (writer_index+1)%NBUFF;
 
        cond_sem_v(&shared.nfull);
        cond_sem_v(&shared.writer_mutex);
    }
 
    return NULL;
}
 
 
 
void* consume(void *arg)
{
    while( 1 )
    {
        cond_sem_p(&shared.nfull);
        cond_sem_p(&shared.reader_mutex);
 
        if( shared.buff[reader_index].n == 0)
        {
            cond_sem_v(&shared.nempty);
            cond_sem_v(&shared.reader_mutex);
            return NULL;
        }

        //printf("write.\n");
        /*
        write(STDOUT_FILENO, shared.buff[reader_index].data,
                shared.buff[reader_index].n);*/
        printf("(%x %x)", shared.buff[reader_index].data[0], shared.buff[reader_index].data[shared.buff[reader_index].n-1]);

        reader_index = (reader_index+1)%NBUFF;
 
        cond_sem_v(&shared.nempty);
        cond_sem_v(&shared.reader_mutex);
    }
 
    return NULL;
}

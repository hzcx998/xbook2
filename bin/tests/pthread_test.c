#include "test.h"
#include <pthread.h>

int count = 0; 

pthread_mutex_t mutex;

void *thread_entry(void *arg)
{
    while (1) {
        pthread_mutex_lock(&mutex);
        printf("child count: %d\n", count);
        count++;

        pthread_mutex_unlock(&mutex);
        if (count > 100)
            return (void *) count;
        usleep(100);
    }
}

int pthread_test1(int argc, char *argv[])
{
    pthread_mutex_init(&mutex, NULL);
    pthread_t thread;
    if (pthread_create(&thread, NULL, thread_entry, NULL) < 0) {
        printf("create thread failed!\n");
        return -1;
    }

    while (1) {
        pthread_mutex_lock(&mutex);
        printf("parent count: %d\n", count);
        count++;
        
        pthread_mutex_unlock(&mutex);
        if (count > 100)
            break;
        usleep(100);
    }

    void *status;
    pthread_join(thread, &status);
    printf("thread exit with %x!\n", status);
    return 0;
}


#define N 1000
int testnum = 0;
sem_t sp;

void* func1(void * p)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYCHRONOUS, NULL); //取消类型为立即取消
    sem_post(&sp);
    for(;;){
        testnum += 1;
    }
}

int pthread_test(int argc, char *argv[])
{
    pthread_t p1;
    printf("maintid: %ld\n", pthread_self());
    sem_init(&sp, 0, 0);
    pthread_create(&p1, NULL, func1, NULL);
    sem_wait(&sp);
    sleep(1);
    pthread_cancel(p1);
    // for(i = 1; i <= N; ++i){
    //     if(pthread_cancel(p1) == 3)
    //         break;
    // }
    pthread_join(p1, NULL);
    printf("final number:%d\n", testnum);
    return 0;
}

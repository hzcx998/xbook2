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

int pthread_test(int argc, char *argv[])
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
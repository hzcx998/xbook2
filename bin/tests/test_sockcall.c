#include "test.h"

#include <sys/socket.h>

int test_sockcall(int argc, char *argv[])
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("create socket %d\n", sockfd);
    fork();
    return 0;
}

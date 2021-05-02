#include "test.h"

#include <sys/socket.h>

int test_sockcall(int argc, char *argv[])
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("create socket %d\n", sockfd);
    
    char buf[32] = {0};
    write(sockfd, buf, 32);

    ioctl(sockfd, 0, NULL);
    fcntl(sockfd, F_SETFL, NULL);
    
    read(sockfd, buf, 32);
    
    fork();
    return 0;
}

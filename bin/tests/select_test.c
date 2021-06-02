#include "test.h"
#include <sys/select.h>
#include <sys/socket.h>

int select_test(int argc, char *argv[])
{
    int filefd = open("select.txt", O_CREAT | O_RDWR);
    if (filefd < 0) {
        printf("open select file failed\n");
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("open sock failed\n");
        return -1;
    }

    fd_set rdfds;
    fd_set wrfds;
    fd_set expfds;
    
    FD_ZERO(&rdfds);
    FD_SET(sockfd, &rdfds);

    FD_ZERO(&wrfds);
    FD_SET(filefd,&wrfds);
    
    FD_ZERO(&expfds);

    int maxfdp = 0;
    maxfdp = max(filefd, sockfd) + 1;

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    int n = select(maxfdp, &rdfds, &wrfds, &expfds, &tv);   
    switch (n) { 
    case -1:
        printf("error\n");
        break;
    case 0:
        printf("timeout\n");
        break;    
    default:
        if (FD_ISSET(filefd, &rdfds)) {
            printf("file readable\n");
        }
        break;
    }
    close(filefd);
    close(sockfd);
    
    return 0;
}


#include "test.h"

#include <arpa/inet.h>

#define MAXLINE 4096
int socket_test(int argc, char *argv[])
{
    int sockfd, n; 
    char recvline[4096], sendline[4096]; 
    struct sockaddr_in servaddr; 
    
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    { 
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        exit(0); 
    }
    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port =htons(6666); 
    servaddr.sin_addr.s_addr = ipaddr_addr("192.168.1.14");

    if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    { 
        printf("connect error: %s(errno: %d)\n",strerror(errno),errno); 
        exit(0); 
    } 
    printf("send msg to server: \n"); fgets(sendline, 4096, stdin); 
    if( send(sockfd, sendline, strlen(sendline), 0) < 0) 
    { 
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
     exit(0); 
    } 
    close(sockfd); 
    exit(0);
}

int socket_test2(int argc, char *argv[])
{
    printf("socket2 test start!\n");
        
    int err;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("create socket failed!\n");
        return -1;
    }
    printf("create socket %d\n", fd);
    struct sockaddr_in myaddr;
    memset(&myaddr, 0, sizeof(struct sockaddr_in));
    myaddr.sin_addr.s_addr = htonl(0);
    myaddr.sin_port = htons(8080);
    myaddr.sin_family = AF_INET;
    
    err = bind(fd, (struct sockaddr *) &myaddr, sizeof(struct sockaddr));
    if (err < 0) {
        printf("socket bind failed!\n");
        return -1;
    }

    err = listen(fd, 5);
    if (err < 0) {
        printf("socket listen failed!\n");
        return -1;
    }

    struct sockaddr client_addr;
    socklen_t client_len;
    int client_fd;
    while (1) {
        client_fd = accept(fd, NULL, NULL);
        printf("accept %d done!\n", client_fd);
        while (client_fd >= 0) {
            char buf[512];
            memset(buf, 0, 512);
            recv(client_fd, buf, 512, 0);
            printf("recv done %s!\n", buf);
            
            //send(client_fd, buf, strlen(buf), 0);
            //printf("send done!\n");
        }
        
    }
    return 0;
}
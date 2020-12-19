#include <stdio.h>
#include <netserv.h>
#include <sys/portcomm.h>
#include <netsocket.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    printf("netserv start\n");
    network_init();
    while (1) {
        
    }
    return 0;
}

#include <arpa/inet.h>

#define MAXLINE 4096
int socket_test(int argc, char *argv[])
{
    int sockfd; 
    char sendline[512]; 
    struct sockaddr_in servaddr; 
    
    if( (sockfd = net_socket(AF_INET, SOCK_STREAM, 0)) < 0)
    { 
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        exit(0); 
    }

    printf("socket fd: %d\n", sockfd); 

    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port =htons(8080); 
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.104");

    if( net_connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    { 
        printf("connect error: %s(errno: %d)\n",strerror(errno),errno); 
        exit(0); 
    } 
    printf("send msg to server: \n"); 
    strcpy(sendline, "hello!\n");
    if( net_send(sockfd, sendline, strlen(sendline), 0) < 0) 
    { 
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0); 
    } 
    memset(sendline, 0, 512);
    if (net_recv(sockfd, sendline, 512, 0) < 0) {
        printf("recv msg error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0); 
    }
    printf("recv: %s\n", sendline);
    net_close(sockfd); 
    exit(0);
    return 0;
}

void client()
{
    usleep(1000 * 1000);
    socket_test(0, NULL);
}
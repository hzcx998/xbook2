#include "test.h"

// #include <sys/socket.h>
#include <netsocket.h>

#include <arpa/inet.h>

#define MAXLINE 4096
int socket_test(int argc, char *argv[])
{
    int sockfd; 
    char sendline[4096]; 
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
    strcpy(sendline, "helllo!\n");
    if( net_send(sockfd, sendline, strlen(sendline), 0) < 0) 
    { 
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
     exit(0); 
    } 
    memset(sendline, 0, 4096);
    if (net_recv(sockfd, sendline, 4096, 0) < 0) {
        printf("recv msg error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0); 
    }
    printf("recv: %s\n", sendline);
    net_close(sockfd); 
    exit(0);
    return 0;
}

int socket_test2(int argc, char *argv[])
{
    printf("socket2 test start!\n");
        
    int err;
    int fd = net_socket(AF_INET, SOCK_STREAM, 0);
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
    
    err = net_bind(fd, (struct sockaddr *) &myaddr, sizeof(struct sockaddr));
    if (err < 0) {
        printf("socket bind failed!\n");
        return -1;
    }

    err = net_listen(fd, 5);
    if (err < 0) {
        printf("socket listen failed!\n");
        return -1;
    }

    int client_fd;
    while (1) {
        client_fd = net_accept(fd, NULL, NULL);
        printf("accept %d done!\n", client_fd);
        if (client_fd >= 0) {
            char buf[512];
            memset(buf, 0, 512);
            net_recv(client_fd, buf, 512, 0);
            printf("recv done %s!\n", buf);
            
            net_send(client_fd, buf, strlen(buf), 0);
            printf("send done!\n");

            close(client_fd);
        }
    }
    return 0;
}

#define SERVER_IP   "192.168.0.104"
#define BUF_LEN 512
int socket_test3(int argc, char *argv[])
{
    printf("socket3 test start!\n");
        
    /* udp连接 */
    int fd = net_socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        printf("create socket failed!\n");
        return -1;
    }
    printf("create socket %d\n", fd);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_addr.s_addr = htonl(IPADDR_ANY);
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_len = sizeof(struct sockaddr_in);
    net_bind(fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
    
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_len = sizeof(struct sockaddr_in);

    struct sockaddr src;
    socklen_t len;
    while (1) {
        
        char buf[BUF_LEN] = "hello! Test!\n";
        len = sizeof(struct sockaddr_in);
        net_sendto(fd, buf, BUF_LEN, 0, (struct sockaddr *)&serv_addr, len);
        sleep(1);
        printf("client: %s\n", buf);
        memset(buf, 0, BUF_LEN);
        net_recvfrom(fd, buf, BUF_LEN, 0, &src, &len);
        printf("server: %s\n", buf);
        break;
    }
    net_close(fd);
    return 0;
}

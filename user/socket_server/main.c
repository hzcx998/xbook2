#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<string.h>
 
int main()
{
    //创建一个socket
    int listenSocket=socket(AF_INET,SOCK_STREAM,0);
    
    //配置ip port 协议
    struct sockaddr_in addrSrc,addrClient;
    addrSrc.sin_family=AF_INET;
    addrSrc.sin_port=htons(6666);
    addrSrc.sin_addr.s_addr=INADDR_ANY;
 
    //绑定
    bind(listenSocket,(struct sockaddr*)&addrSrc,sizeof(struct sockaddr_in));
 
    //监听
    listen(listenSocket,5);
 
    int connfd=0;
    socklen_t len=sizeof(struct sockaddr_in);
    connfd=accept(listenSocket,(struct sockaddr*)&addrClient,&len);
    char *ipStr=inet_ntoa(addrClient.sin_addr);
    printf("connect is %s\n",ipStr);
 
    char sendBuf[100]="hello client!";
    //发送信息到客户端
    send(connfd,sendBuf,strlen(sendBuf)+1,0);
    
    while(1)
    {
        //接收客户端信息
        char recvBuf[100]={0};
        int ret=recv(connfd,recvBuf,sizeof(recvBuf)-1,0);
        if(0==ret)
        {
            printf("%s is leave!\n",ipStr);
            break;
        }
        char printBuf[1024]={0};
        //字符串拼接
        sprintf(printBuf,"from %s: %s",ipStr,recvBuf);
        puts(printBuf);
    }
    
    //关闭套接字
    close(connfd);
    close(listenSocket);
    
    return 0;
}
/**
 * TODO: 实现函数代码
 */
#ifndef _XLIBC_NETDB_H
#define _XLIBC_NETDB_H

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hostent {
    char *h_name;    //主机名
    char **h_aliases; //主机别名列表，可能有多个
    int h_addrtype;   //地址类型（地址族）
    int h_length;     //地址长度
    char **h_addr_list;//按网络字节序列出的主机IP地址列表
};

struct servent
{
    char* s_name;       //服务名称
    char** s_aliases;   //服务的别名列表，可能有多个
    int s_port;         //端口号
    char* s_proto;     //服务类型，通常是tcp或者udp
};

struct addrinfo
{
    int ai_flags;  //
    int ai_family;//地址族
    int ai_socktype;//服务类型，SOCK_STREAM 或 SOCK_DGRAM
    int ai_protocol;//
    socklen_t ai_addrlen;// socket地址 ai_addr的长度
    char* ai_canonname;//主机的别名
    struct sockaddr* ai_addr; //指向socket地址
    struct addrinfo* ai_next; //指向下一个sockinfo结构的对象
};

struct hostent* gethostbyname(const char* name);
struct hostent* gethostbyaddr(const void* addr, size_t len, int type);

struct servent* getservbyname(const char* name, const char* proto);
struct servent* getsrvbyport(int port, const char* proto);

int getaddrinfo(const char* hostname, const char* service, 
    const struct addrinfo* hints, struct addrinfo** result);

void freeaddrinfo(struct addrinfo*  res);

int getnameinfo(const struct sockaddr* sockaddr, socklen_t addrlen, char* host, 
    socklen_t hostlen, char* serv, socklen_t servlen, int flags);

#ifdef __cplusplus
}
#endif

#endif /* _XLIBC_NETDB_H */

#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

#include <types.h>
#include <stdint.h>

typedef uint32_t socklen_t;

typedef uint32_t u32_t;
typedef uint16_t u16_t;
typedef uint8_t u8_t;


/* These macros should be calculated by the preprocessor and are used
   with compile-time constants only (so that there is no little-endian
   overhead at runtime). */
#define PP_HTONS(x) ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))
#define PP_NTOHS(x) PP_HTONS(x)
#define PP_HTONL(x) ((((x) & 0xff) << 24) | \
                     (((x) & 0xff00) << 8) | \
                     (((x) & 0xff0000UL) >> 8) | \
                     (((x) & 0xff000000UL) >> 24))
#define PP_NTOHL(x) PP_HTONL(x)


/* Socket family */
#define AF_UNSPEC       0
#define AF_INET         2
#define PF_INET         AF_INET
#define PF_UNSPEC       AF_UNSPEC

/* Socket protocol types (TCP/UDP/RAW) */
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3

#define IPPROTO_IP      0
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17
#define IPPROTO_UDPLITE 136

struct in_addr {
  u32_t s_addr;
};

/* This is the aligned version of ip_addr_t,
   used as local variable, on the stack, etc. */
struct ip_addr {
  u32_t addr;
};

typedef struct ip_addr ip_addr_t;

/** 255.255.255.255 */
#define IPADDR_NONE         ((u32_t)0xffffffffUL)

/** IPv4 only: set the IP address given as an u32_t */
#define ip4_addr_set_u32(dest_ipaddr, src_u32) ((dest_ipaddr)->addr = (src_u32))
/** IPv4 only: get the IP address as an u32_t */
#define ip4_addr_get_u32(src_ipaddr) ((src_ipaddr)->addr)

/* members are in network byte order */
struct sockaddr_in {
  u8_t sin_len;
  u8_t sin_family;
  u16_t sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

struct sockaddr {
  u8_t sa_len;
  u8_t sa_family;
  char sa_data[14];
};

int socket(int domain, int type, int protocol);
int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
int connect(int sockfd, struct sockaddr *serv_addr, int addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int send(int sockfd, const void *msg, int len, int flags);
int recv(int sockfd, void *buf, int len, unsigned int flags);
int sendto(int sockfd, const void *msg, int len, unsigned int flags,
    const struct sockaddr *to, int tolen);
int recvfrom(int sockfd, void *buf, int len, unsigned int flags,
    struct sockaddr *from, int *fromlen);
int shutdown(int sockfd, int how);
int sockclose(int sockfd);

int getpeername(int sockfd, struct sockaddr *serv_addr, socklen_t *addrlen);
int getsockname(int sockfd, struct sockaddr *my_addr, socklen_t *addrlen);
int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

#define inet_addr(cp)         _ipaddr_addr(cp)

u32_t
_ipaddr_addr(const char *cp);

#endif  /* _SYS_SOCKET_H */
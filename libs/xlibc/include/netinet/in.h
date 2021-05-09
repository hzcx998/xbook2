
#ifndef _XLIBC_NETINET_INET_H
#define _XLIBC_NETINET_INET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint32_t in_addr_t;

uint32_t htonl(uint32_t);
uint16_t htons(uint16_t);
uint32_t ntohl(uint32_t);
uint16_t ntohs(uint16_t);

#define  INET_ADDRSTRLEN   16
#define  INET6_ADDRSTRLEN  46

#ifdef __cplusplus
}
#endif

#endif  /* _XLIBC_NETINET_INET_H */

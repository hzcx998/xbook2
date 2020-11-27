
#ifndef _XLIBC_ARPA_INET_H
#define _XLIBC_ARPA_INET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint32_t htonl(uint32_t);
uint16_t htons(uint16_t);
uint32_t ntohl(uint32_t);
uint16_t ntohs(uint16_t);

#ifdef __cplusplus
}
#endif

#endif  /* _XLIBC_ARPA_INET_H */

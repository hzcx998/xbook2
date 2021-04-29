#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

#include <types.h>

struct sockaddr {
  u8_t sa_len;
  u8_t sa_family;
  char sa_data[14];
};

int sys_socket(int domain, int type, int protocol);

#endif   /* _SYS_SOCKET_H */


#include <unistd.h>
#include <types.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/srvcall.h>
#include <srv/netsrv.h>
#include <arpa/inet.h>

int socket(int domain, int type, int protocol)
{
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, NETSRV_SOCKET, 0);
    SETSRV_ARG(&srvarg, 1, domain, 0);
    SETSRV_ARG(&srvarg, 2, type, 0);
    SETSRV_ARG(&srvarg, 3, protocol, 0);
    SETSRV_RETVAL(&srvarg, -1);

    if (!srvcall(SRV_NET, &srvarg)) {
        int sockid = GETSRV_RETVAL(&srvarg, int);
        if (sockid == -1) {
            return -1;
        }
        return sockid;
    }
    return -1;
}

int bind(int sockfd, struct sockaddr *my_addr, int addrlen)
{
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, NETSRV_BIND, 0);
    SETSRV_ARG(&srvarg, 1, sockfd, 0);
    SETSRV_ARG(&srvarg, 2, my_addr, addrlen);
    SETSRV_RETVAL(&srvarg, -1);

    if (!srvcall(SRV_NET, &srvarg)) {
        int ret = GETSRV_RETVAL(&srvarg, int);
        if (ret == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int connect(int sockfd, struct sockaddr *serv_addr, int addrlen)
{
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, NETSRV_CONNECT, 0);
    SETSRV_ARG(&srvarg, 1, sockfd, 0);
    SETSRV_ARG(&srvarg, 2, serv_addr, addrlen);
    SETSRV_RETVAL(&srvarg, -1);

    if (!srvcall(SRV_NET, &srvarg)) {
        int ret = GETSRV_RETVAL(&srvarg, int);
        if (ret == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int listen(int sockfd, int backlog)
{
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, NETSRV_LISTEN, 0);
    SETSRV_ARG(&srvarg, 1, sockfd, 0);
    SETSRV_ARG(&srvarg, 2, backlog, 0);
    SETSRV_RETVAL(&srvarg, -1);

    if (!srvcall(SRV_NET, &srvarg)) {
        int ret = GETSRV_RETVAL(&srvarg, int);
        if (ret == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, NETSRV_ACCEPT, 0);
    SETSRV_ARG(&srvarg, 1, sockfd, 0);
    SETSRV_ARG(&srvarg, 2, addr, *addrlen);
    SETSRV_ARG(&srvarg, 3, addrlen, sizeof(socklen_t));
    SETSRV_ARG(&srvarg, 4, *addrlen, 0); // 传入地址长度
    SETSRV_IO(&srvarg, (SRVIO_USER << 2) | (SRVIO_USER << 3));
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_NET, &srvarg)) {
        int ret = GETSRV_RETVAL(&srvarg, int);
        if (ret == -1) {
            return -1;
        }
        return ret;
    }
    return -1;
}

int send(int sockfd, const void *buf, int len, int flags)
{
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, NETSRV_SEND, 0);
    SETSRV_ARG(&srvarg, 1, sockfd, 0);
    SETSRV_ARG(&srvarg, 2, buf, len);
    SETSRV_ARG(&srvarg, 3, flags, 0);
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_NET, &srvarg)) {
        int ret = GETSRV_RETVAL(&srvarg, int);
        if (ret == -1) {
            return -1;
        }
        return ret;
    }
    return -1;
}

int recv(int sockfd, void *buf, int len, unsigned int flags)
{
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, NETSRV_RECV, 0);
    SETSRV_ARG(&srvarg, 1, sockfd, 0);
    SETSRV_ARG(&srvarg, 2, buf, len);
    SETSRV_ARG(&srvarg, 3, flags, 0);
    SETSRV_IO(&srvarg, (SRVIO_USER << 2));
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_NET, &srvarg)) {
        int ret = GETSRV_RETVAL(&srvarg, int);
        if (ret == -1) {
            return -1;
        }
        return ret;
    }
    return -1;
}

int sendto(int sockfd, const void *buf, int len, unsigned int flags,
    const struct sockaddr *to, int tolen)
{
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, NETSRV_SENDTO, 0);
    SETSRV_ARG(&srvarg, 1, sockfd, 0);
    SETSRV_ARG(&srvarg, 2, buf, len);
    SETSRV_ARG(&srvarg, 3, flags, 0);
    SETSRV_ARG(&srvarg, 4, to, tolen);
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_NET, &srvarg)) {
        int ret = GETSRV_RETVAL(&srvarg, int);
        if (ret == -1) {
            return -1;
        }
        return ret;
    }
    return -1;
}

int recvfrom(int sockfd, void *buf, int len, unsigned int flags,
    struct sockaddr *from, int *fromlen)
{
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, NETSRV_RECVFROM, 0);
    SETSRV_ARG(&srvarg, 1, sockfd, 0);
    SETSRV_ARG(&srvarg, 2, buf, len);
    SETSRV_ARG(&srvarg, 3, flags, 0);
    SETSRV_ARG(&srvarg, 4, from, *fromlen);
    SETSRV_ARG(&srvarg, 5, fromlen, sizeof(int));
    SETSRV_ARG(&srvarg, 6, *fromlen, 0);
    
    SETSRV_IO(&srvarg, (SRVIO_USER << 2) | (SRVIO_USER << 4) | (SRVIO_USER << 5));
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_NET, &srvarg)) {
        int ret = GETSRV_RETVAL(&srvarg, int);
        if (ret == -1) {
            return -1;
        }
        return ret;
    }
    return -1;
}


int sockclose(int sockfd)
{
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, NETSRV_CLOSE, 0);
    SETSRV_ARG(&srvarg, 1, sockfd, 0);
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_NET, &srvarg)) {
        int ret = GETSRV_RETVAL(&srvarg, int);
        if (ret == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int
__ipaddr_aton(const char *cp, ip_addr_t *addr);

/**
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 *
 * @param cp IP address in ascii represenation (e.g. "127.0.0.1")
 * @return ip address in network order
 */
u32_t
_ipaddr_addr(const char *cp)
{
  ip_addr_t val;

  if (__ipaddr_aton(cp, &val)) {
    return ip4_addr_get_u32(&val);
  }
  return (IPADDR_NONE);
}

/**
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 *
 * @param cp IP address in ascii represenation (e.g. "127.0.0.1")
 * @param addr pointer to which to save the ip address in network order
 * @return 1 if cp could be converted to addr, 0 on failure
 */
int
__ipaddr_aton(const char *cp, ip_addr_t *addr)
{
  u32_t val;
  u8_t base;
  char c;
  u32_t parts[4];
  u32_t *pp = parts;

  c = *cp;
  for (;;) {
    /*
     * Collect number up to ``.''.
     * Values are specified as for C:
     * 0x=hex, 0=octal, 1-9=decimal.
     */
    if (!isdigit(c))
      return (0);
    val = 0;
    base = 10;
    if (c == '0') {
      c = *++cp;
      if (c == 'x' || c == 'X') {
        base = 16;
        c = *++cp;
      } else
        base = 8;
    }
    for (;;) {
      if (isdigit(c)) {
        val = (val * base) + (int)(c - '0');
        c = *++cp;
      } else if (base == 16 && isxdigit(c)) {
        val = (val << 4) | (int)(c + 10 - (islower(c) ? 'a' : 'A'));
        c = *++cp;
      } else
        break;
    }
    if (c == '.') {
      /*
       * Internet format:
       *  a.b.c.d
       *  a.b.c   (with c treated as 16 bits)
       *  a.b (with b treated as 24 bits)
       */
      if (pp >= parts + 3) {
        return (0);
      }
      *pp++ = val;
      c = *++cp;
    } else
      break;
  }
  /*
   * Check for trailing characters.
   */
  if (c != '\0' && !isspace(c)) {
    return (0);
  }
  /*
   * Concoct the address according to
   * the number of parts specified.
   */
  switch (pp - parts + 1) {

  case 0:
    return (0);       /* initial nondigit */

  case 1:             /* a -- 32 bits */
    break;

  case 2:             /* a.b -- 8.24 bits */
    if (val > 0xffffffUL) {
      return (0);
    }
    val |= parts[0] << 24;
    break;

  case 3:             /* a.b.c -- 8.8.16 bits */
    if (val > 0xffff) {
      return (0);
    }
    val |= (parts[0] << 24) | (parts[1] << 16);
    break;

  case 4:             /* a.b.c.d -- 8.8.8.8 bits */
    if (val > 0xff) {
      return (0);
    }
    val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
    break;
  default:
    printf("unhandled");
    break;
  }
  if (addr) {
    ip4_addr_set_u32(addr, htonl(val));
  }
  return (1);
}
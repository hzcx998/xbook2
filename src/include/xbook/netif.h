#ifndef _XBOOK_NETIF_H
#define _XBOOK_NETIF_H

#ifdef CONFIG_NET

#define DEBUG_NETIF
int netif_close(int sock);
int netif_incref(int sock);
int netif_decref(int sock);
int netif_read(int sock, void *buffer, size_t nbytes);
int netif_write(int sock, void *buffer, size_t nbytes);
int netif_ioctl(int sock, int request, void *arg);
int netif_fcntl(int sock, int cmd, long val);
int do_socket_close(int sock);
#endif

#endif  /* _XBOOK_NETIF_H */
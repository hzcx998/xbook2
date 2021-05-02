#ifndef _XBOOK_SOCKET_CACHE_H
#define _XBOOK_SOCKET_CACHE_H

#include <xbook/spinlock.h>
#include <xbook/list.h>
#include <xbook/spinlock.h>

/* 套接字缓存，用来记录套接字信息 */
typedef struct {
    list_t list;            /* 链表 */
    int socket;             /* 套接字值 */
    atomic_t reference;     /* 套接字引用 */
} socket_cache_t;

socket_cache_t *socket_cache_create(int socket);
int socket_cache_destroy(socket_cache_t *socache);
int socket_cache_inc(socket_cache_t *socache);
int socket_cache_dec(socket_cache_t *socache);
socket_cache_t *socket_cache_find(int socket);

#endif   /* _XBOOK_SOCKET_CACHE_H */

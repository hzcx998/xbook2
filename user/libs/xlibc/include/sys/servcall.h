#ifndef _SYS_SERVCALL_H
#define _SYS_SERVCALL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define SERVMSG_SIZE (4096 - sizeof(size_t))

typedef struct {
    uint32_t reserved0;
    uint32_t reserved1;
    size_t size;
    uint8_t data[SERVMSG_SIZE];
} servmsg_t;

int bind_port(int port);
int unbind_port(int port);
int reply_port(int port, servmsg_t *msg);
int receive_port(int port, servmsg_t *msg);
int request_port(int port, servmsg_t *msg);
void servmsg_reset(servmsg_t *msg);

#ifdef __cplusplus
}
#endif

#endif   /* _SYS_SERVCALL_H */
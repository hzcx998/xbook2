#ifndef _SYS_SERVCALL_H
#define _SYS_SERVCALL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define FIRST_CALL_CODE  (0x00000001)
#define LAST_CALL_CODE   (0x000FFFFF)

#define SERVMSG_HEADER_SIZE (4 * sizeof(uint32_t))
#define SERVMSG_SIZE (4096 - SERVMSG_HEADER_SIZE)

/* 知名服务端口号 */
enum {
    SERVPORT_TEST = 0,
    SERVPORT_NET,
    SERVPORT_GRAPH,
    SERVPORT_LAST = 32
};

typedef struct {
    uint32_t reserved0;
    uint32_t reserved1;
    uint32_t code;
    uint32_t size;
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
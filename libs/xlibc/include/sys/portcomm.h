#ifndef _SYS_PORT_COMM_H
#define _SYS_PORT_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>

enum {
    PORT_COMM_TEST = 0,
    PORT_COMM_NET,
    PORT_COMM_GRAPH,
    PORT_COMM_LAST = 8
};

/* 端口绑定标志 */
enum {
    PORT_BIND_GROUP = 0x01,     /* 端口绑定时为组端口 */
    PORT_BIND_ONCE  = 0x02,     /* 端口绑定时只绑定一次，多次绑定还是返回成功 */
};


typedef struct {
    uint32_t reserved0;
    uint32_t reserved1;
    uint32_t size;
} port_msg_header_t;

#define PORT_MSG_HEADER_SIZE (sizeof(port_msg_header_t))
#define PORT_MSG_SIZE (4096 - PORT_MSG_HEADER_SIZE)

typedef struct {
    port_msg_header_t header;
    uint8_t data[PORT_MSG_SIZE];
} port_msg_t;

int bind_port(int port, int flags);
int unbind_port(int port);
int reply_port(int port, port_msg_t *msg);
int receive_port(int port, port_msg_t *msg);
int request_port(int port, port_msg_t *msg);
void port_msg_reset(port_msg_t *msg);
void port_msg_copy_header(port_msg_t *src, port_msg_t *dest);


#ifdef __cplusplus
}
#endif

#endif   /* _SYS_PORT_COMM_H */
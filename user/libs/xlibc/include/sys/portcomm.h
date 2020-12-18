#ifndef _SYS_PORT_COMM_H
#define _SYS_PORT_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

enum {
    PORT_COMM_TEST = 0,
    PORT_COMM_NET,
    PORT_COMM_GRAPH,
    PORT_COMM_LAST = 8
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

int bind_port(int port);
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
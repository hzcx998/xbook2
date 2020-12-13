#ifndef _SYS_LPC_H
#define _SYS_LPC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define LPC_MESSAGE_DATA_LEN   4096

typedef struct {
    uint32_t id;
    uint32_t size;
    uint8_t data[LPC_MESSAGE_DATA_LEN]; 
} lpc_message_t;


int create_port(char *name, uint32_t max_connects, uint32_t max_msgsz);
int close_port(int phandle);
int accept_port(int phandle, int isaccept);
int connect_port(char *name, uint32_t *max_msgsz);
int reply_port(int phandle, lpc_message_t *lpc_msg);
int receive_port(int phandle, lpc_message_t *lpc_msg);
int request_port(int phandle, lpc_message_t *lpc_msg);
void lpc_reset_message(lpc_message_t *msg);

#ifdef __cplusplus
}
#endif

#endif   /* _SYS_LPC_H */
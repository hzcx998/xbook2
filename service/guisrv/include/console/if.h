#ifndef _GUISRV_CONSOLE_IF_H
#define _GUISRV_CONSOLE_IF_H

#include <xcons.h>

void *guisrv_echo_thread(void *arg);
int guisrv_if_send_msg(xcons_msg_t *msg);
int guisrv_if_recv_data(void *buf, int buflen);

#endif  /* _GUISRV_CONSOLE_IF_H */
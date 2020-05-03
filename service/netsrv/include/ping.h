#ifndef __PING_H__
#define __PING_H__

/**
 * PING_USE_SOCKETS: Set to 1 to use sockets, otherwise the raw api is used
 */
void ping_init(void);
void ping_send_now();
#endif /* __PING_H__ */

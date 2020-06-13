#ifndef __SGI_MESSAGE_H__ /* message */
#define __SGI_MESSAGE_H__

/* SGI: Simple graphical interface */
#include <stddef.h>
#include "sgi.h"

typedef struct _SGI_Msg {
    long type;              /* 固定格式：消息类型 */
    unsigned long id;       /* 消息id */
    unsigned long arg0;     /* 参数 */
    unsigned long arg1;     /* 参数 */
    unsigned long arg2;     /* 参数 */
    unsigned long arg3;     /* 参数 */
    unsigned long arg4;     /* 参数 */
    unsigned long arg5;     /* 参数 */
    unsigned long arg6;     /* 参数 */
    unsigned long arg7;     /* 参数 */
} __attribute__ ((packed)) SGI_Msg;

#define SGI_MSG_LEN (sizeof(struct _SGI_Msg) - sizeof(long))

#endif  /* __SGI_MESSAGE_H__ */
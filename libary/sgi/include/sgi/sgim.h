#ifndef __SGI_MESSAGE_H__ /* message */
#define __SGI_MESSAGE_H__

/* SGI: Simple graphical interface */
#include <stddef.h>
#include "sgi.h"

typedef struct _SGI_Msg {
    long type;              /* 固定格式：消息类型 */
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

enum SGI_MsgType {
    SGI_MSG_UNKNOWN = 0,
    SGI_MSG_UPDATE_WINDOW,  /* 更新窗口 */
};


#endif  /* __SGI_MESSAGE_H__ */
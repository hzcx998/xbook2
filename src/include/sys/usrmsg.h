#ifndef _SYS_USRMSG_H
#define _SYS_USRMSG_H

enum {
    UMSG_NONE = 0,          /* none message */
    UMSG_OPEN,              /* device open */
    UMSG_CLOSE,             /* device close */
    UMSG_READ,              /* device close */
    UMSG_WRITE,             /* device read */
    UMSG_IOCTL,             /* device ioctl */
    UMSG_GETC,              /* device getc */
    UMSG_PUTC,              /* device putc */
    UMSG_FORK,              /* process fork */
    UMSG_EXECFILE,          /* process execute file*/
    UMSG_EXIT,              /* process exit */
    UMSG_WAIT,              /* process wait */
    UMSG_EXECRAW,           /* process execute raw block */
    UMSG_HEAP,              /* memory heap */
};

/* user message 用户消息 */
typedef struct {
    unsigned long type;         /* 消息类型 */
    unsigned long retval;       /* 返回值 */
    unsigned long arg0;         /* 参数0 */
    unsigned long arg1;         /* 参数1 */
    unsigned long arg2;         /* 参数2 */
    unsigned long arg3;         /* 参数3 */
    unsigned long arg4;         /* 参数4 */
} umsg_t;

#define umsg_set_type(msg, tp)      (msg).type = (tp) 
#define umsg_set_arg0(msg, val)     (msg).arg0 = (unsigned long )(val) 
#define umsg_set_arg1(msg, val)     (msg).arg1 = (unsigned long )(val) 
#define umsg_set_arg2(msg, val)     (msg).arg2 = (unsigned long )(val) 
#define umsg_set_arg3(msg, val)     (msg).arg3 = (unsigned long )(val) 
#define umsg_set_arg4(msg, val)     (msg).arg4 = (unsigned long )(val) 
#define umsg_get_arg0(msg, type)    (type)(msg).arg0 
#define umsg_get_arg1(msg, type)    (type)(msg).arg1 
#define umsg_get_arg2(msg, type)    (type)(msg).arg2 
#define umsg_get_arg3(msg, type)    (type)(msg).arg3 
#define umsg_get_arg4(msg, type)    (type)(msg).arg4 
#define umsg_get_retval(msg, type)  (type)(msg).retval 
#define umsg_get_type(msg)          (msg).type 

#define __umsg_init(msgname) \
    { .type = 0 \
    , .retval = 0 \
    , .arg0 = 0 \
    , .arg1 = 0 \
    , .arg2 = 0 \
    , .arg3 = 0 \
    , .arg4 = 0 \
    }

#define define_umsg(msgname) \
    umsg_t msgname = __umsg_init(msgname);

int usrmsg(umsg_t *msg);

static inline void umsg_init(umsg_t *msg, unsigned long type)
{
    msg->type = type;
    msg->retval = 0;
    msg->arg0 = 0;
    msg->arg1 = 0;
    msg->arg2 = 0;
    msg->arg3 = 0;
    msg->arg4 = 0;
}

#define umsg(msg) usrmsg(&(msg)) 

#endif  /* _SYS_USRMSG_H */

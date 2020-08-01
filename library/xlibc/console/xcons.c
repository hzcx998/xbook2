
#include <sys/srvcall.h>
#include <sys/res.h>
#include <sys/ipc.h>
#include <string.h>
#include <stdio.h>
#include <srv/guisrv.h>

#include <xcons.h>
#include <unistd.h>

/* 控制台连接状态 */
int __xcons_connect_status = -1;

int __xcons_msgid = -1;
int __xcons_pipeid = -1;


int xcons_connect()
{
    if (__xcons_connect_status >= 0)    /* 不能重复连接 */
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_CONNECT, 0);
    SETSRV_RETVAL(&srvarg, -1);

    if (srvcall(SRV_GUI, &srvarg)) {
        return -1;
    }
    if (GETSRV_RETVAL(&srvarg, int) == -1) {
        return -1;
    }
    /* 链接服务成功 */
    /* 打开消息队列，接收服务的消息 */
    int msgid = res_open("guisrv-msg", RES_IPC | IPC_MSG | IPC_CREAT, 0);
    if (msgid < 0) {
        goto rollback_connect;
    }

    int pipeid = open("guisrv-pipe", O_PIPE | O_WRONLY, 0);
    if (pipeid < 0) {
        goto rollback_msg;
    }
    
    __xcons_msgid = msgid;
    __xcons_pipeid = pipeid;
    __xcons_connect_status = 0;
    return 0;

rollback_msg:
    res_close(msgid);
rollback_connect:
    __xcons_connect_status = -1;
    
    SETSRV_ARG(&srvarg, 0, GUISRV_CLOSE, 0);
    SETSRV_RETVAL(&srvarg, -1);

    if (srvcall(SRV_GUI, &srvarg)) {
        return -1;
    }
    if (GETSRV_RETVAL(&srvarg, int) == -1) {
        return -1;
    }
    return -1;
}

int xcons_close()
{
    if (__xcons_connect_status < 0)    /* 只有连接后才能关闭 */
        return -1;

    /* 发送关闭显示服务请求 */
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_CLOSE, 0);
    SETSRV_RETVAL(&srvarg, -1);
    if (srvcall(SRV_GUI, &srvarg)) {
        return -1;
    }
    if (GETSRV_RETVAL(&srvarg, int) == -1) {
        return -1;
    }
    if (__xcons_msgid >= 0) {
        res_ioctl(__xcons_msgid, IPC_DEL, 0);
        res_close(__xcons_msgid);
    }
    if (__xcons_pipeid >= 0)
        close(__xcons_pipeid);

    __xcons_connect_status = -1;
    return 0;
}

int xcons_next_msg(xcons_msg_t *m)
{
    if (__xcons_connect_status < 0)
        return -1;
    
    xcons_msg_t msg;
    msg.type = 0;
    /* 接收一个消息 */
    if (res_read(__xcons_msgid, 0, &msg, sizeof(xcons_msg_t) - sizeof(long)) < 0) {
        return -1;
    }
    
    /* 复制消息 */
    if (m)
        *m = msg;
    return 0;
}

int xcons_poll_msg(xcons_msg_t *m)
{
    if (__xcons_connect_status < 0)
        return -1;
    
    xcons_msg_t msg;
    msg.type = 0;
    /* 接收一个消息 */
    if (res_read(__xcons_msgid, IPC_NOWAIT, &msg, sizeof(xcons_msg_t) - sizeof(long)) < 0) {
        return -1;
    }
    
    /* 复制消息 */
    if (m)
        *m = msg;
    return 0;
}

int xcons_xmit_data(void *buf, size_t buflen)
{
    if (__xcons_connect_status < 0)
        return -1;
    return write(__xcons_pipeid, buf, buflen);
}

int xcons_clear()
{
    if (__xcons_connect_status < 0)    /* 只有连接后才能关闭 */
        return -1;

    /* 发送关闭显示服务请求 */
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_CLEAR, 0);
    SETSRV_RETVAL(&srvarg, -1);
    if (srvcall(SRV_GUI, &srvarg)) {
        return -1;
    }
    if (GETSRV_RETVAL(&srvarg, int) == -1) {
        return -1;
    }
    return 0;
}

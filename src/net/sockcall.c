#include <xbook/sockcall.h>
#include <errno.h>

/**
 * 根据参数选择不同的套接字调用
 */
int sys_sockcall(int sockop, sock_param_t *param)
{
    if (sockop < SOCKOP_socket || sockop > SOCKOP_recvmsg || !param) 
        return -EINVAL;
    switch (sockop) {
    case SOCKOP_socket:
        return sys_socket(param->socket.domain, param->socket.type, param->socket.protocol);
    default:
        return -ENOSYS;
    }
    /* should never be here! */
    return -1;
}
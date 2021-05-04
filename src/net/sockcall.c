#include <sys/socket.h>
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
    case SOCKOP_bind:
        return sys_socket_bind(param->bind.sock, param->bind.my_addr, param->bind.addrlen);    
    case SOCKOP_connect:
        return sys_socket_connect(param->connect.sock, param->connect.serv_addr, param->connect.addrlen);    
    case SOCKOP_accept:
        return sys_socket_accept(param->accept.sock, param->accept.addr, param->accept.addrlen);    
    case SOCKOP_listen:
        return sys_socket_listen(param->listen.sock, param->listen.backlog);    
    case SOCKOP_recv:
        return sys_socket_recv(param->recv.sock, param->recv.buf, param->recv.len, param->recv.flags);    
    case SOCKOP_recvfrom:
        return sys_socket_recvfrom(param->recvfrom.sock, param->recvfrom.buf, param->recvfrom.len, 
            param->recvfrom.flags, param->recvfrom.from, param->recvfrom.fromlen);    
    case SOCKOP_send:
        return sys_socket_send(param->send.sock, param->send.buf, param->send.len, param->send.flags);    
    case SOCKOP_sendto:
        return sys_socket_sendto(param->sendto.sock, param->sendto.buf, param->sendto.len, param->sendto.flags,
        param->sendto.to, param->sendto.tolen);    
    case SOCKOP_shutdown:
        return sys_socket_shutdown(param->shutdown.sock, param->shutdown.how);    
    case SOCKOP_getpeername:
        return sys_socket_getpeername(param->getpeername.sock, param->getpeername.serv_addr, param->getpeername.addrlen);    
    case SOCKOP_getsockname:
        return sys_socket_getsockname(param->getsockname.sock, param->getsockname.my_addr, param->getsockname.addrlen);    
    case SOCKOP_getsockopt:
        return sys_socket_getsockopt(param->getsockopt.sock, param->getsockopt.level, param->getsockopt.optname,
        param->getsockopt.optval, param->getsockopt.optlen);    
    case SOCKOP_setsockopt:
        return sys_socket_setsockopt(param->setsockopt.sock, param->setsockopt.level, param->setsockopt.optname,
        param->setsockopt.optval, param->setsockopt.optlen);
    case SOCKOP_socketpair:
    case SOCKOP_sendmsg:
    case SOCKOP_recvmsg:
    default:
        return -ENOSYS;
    }
    /* should never be here! */
    return -EPERM;
}
#include <sys/syscall.h>
#include <sys/sockcall.h>
#include <sys/socket.h>
#include <errno.h>

/**
 * sockcall is a syscall for socket call
 * @sockop: socket operate
 * @param:  socket param for different operate
 * 
 * @return:failed return -1, success return retval
 */
int sockcall(int sockop, sock_param_t *param)
{
    int retval = syscall2(int, SYS_SOCKCALL, sockop, param);
    if (retval < 0) {
        _set_errno(-retval);
        return -1;
    }
    return retval;
}

int socket(int domain, int type, int protocol)
{
    sock_param_t param;
    param.socket.domain = domain;
    param.socket.type = type;
    param.socket.protocol = protocol;
    return sockcall(SOCKOP_socket, &param);
}

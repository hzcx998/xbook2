# 用户态套接字接口
sys_socket  -> 请求服务-> 安装套接字fd  -> done
sys_close   -> 请求服务-> 删除套接字fd  -> done
sys_read    -> 请求服务-> 数据读取      -> done
sys_write   -> 请求服务-> done

陷入套接字：如果网络服务不存在则返回NOSYS
存在则发送请求，等待应答请求。
等待过程中可以被终止等待，然后返回执行失败

服务绑定流程：
注册一个内核扩展服务，记录当前服务进程的信息。
从内核服务队列获取任务一个任务，根据任务信息完成任务。
完成任务后，需要对任务进行回复。并返回响应的参数信息。

参数传递：单位数据就直接传送，缓冲区则根据缓冲区大小判断建立共享内核和对缓冲区进行切片。


#define SOCKOP_socket  1
#define SOCKOP_bind  2
#define SOCKOP_connect  3
#define SOCKOP_listen  4
#define SOCKOP_accept  5
#define SOCKOP_getsockname 6
#define SOCKOP_getpeername 7
#define SOCKOP_socketpair 8
#define SOCKOP_send  9
#define SOCKOP_recv  10
#define SOCKOP_sendto  11
#define SOCKOP_recvfrom  12
#define SOCKOP_shutdown  13
#define SOCKOP_setsockopt 14
#define SOCKOP_getsockopt 15
#define SOCKOP_sendmsg  16
#define SOCKOP_recvmsg  17
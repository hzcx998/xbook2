
#ifndef __NETSRV_IF_H   /* 接口 */
#define __NETSRV_IF_H

/* 接口模型 */

/*
netsrv进程：扫描网卡数据。
lwip核心线程：处理网络协议数据。
netsrv接口线程：处理用户接口信息，根据套接字的创建和关闭来创建一个
    lwip线程，来处理数据。
lwip代理线程：帮助用户进程处理数据，并把处理结果返回给用户进程。

用户进程：通过网络套接字接口，netsrv接口线程操作，进行数据传输。
*/

int init_netsrv_if();

#endif  /* __NETSRV_IF_H */

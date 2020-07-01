#ifndef _SRV_FILE_SRV_H
#define _SRV_FILE_SRV_H

/* file server call */
enum filesrv_call_num {
    NETSRV_SOCKET = 0,
    NETSRV_BIND,
    NETSRV_CONNECT,
    NETSRV_LISTEN,
    NETSRV_ACCEPT,
    NETSRV_SEND,
    NETSRV_RECV,
    NETSRV_CLOSE,
    NETSRV_CALL_NR,    /* 最大数量 */
};

#endif   /* _SRV_FILE_SRV_H */
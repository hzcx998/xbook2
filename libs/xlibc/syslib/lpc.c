#include <sys/lpc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

lpc_parcel_t lpc_parcel_get()
{
    lpc_parcel_t parcel = malloc(PORT_MSG_SIZE);
    if (parcel) {
        memset(parcel, 0, PORT_MSG_SIZE);
    }
    return parcel;
}

int lpc_parcel_put(lpc_parcel_t parcel)
{
    if (parcel) {
        free(parcel);
        return 0;
    }
    return -1;
}

static uint16_t lpc_parcel_get_arg_type(lpc_parcel_t parcel, uint16_t index)
{
    return parcel->header.argtype[index];
}

static int lpc_parcel_alloc_arg_solt(lpc_parcel_t parcel)
{
    if (!parcel)
        return -1;
    int i; for (i = 0; i < LPC_PARCEL_ARG_NR; i++) {
        if (!(parcel->header.argused & (1 << i))) 
            break;
    }
    if (i >= LPC_PARCEL_ARG_NR) {
        return -1;
    }
    return i;
}

static int lpc_parcel_find_arg_solt(lpc_parcel_t parcel, uint16_t type)
{
    if (!parcel)
        return -1;
    int i; for (i = 0; i < LPC_PARCEL_ARG_NR; i++) {
        if ((parcel->header.argused & (1 << i))) {
            if (lpc_parcel_get_arg_type(parcel, i) == type && 
                parcel->header.arglen[i] > 0) {
                break;
            }
        }
    }
    if (i >= LPC_PARCEL_ARG_NR) {
        return -1;
    }
    return i;
}

static void lpc_parcel_set_arg(lpc_parcel_t parcel, uint16_t index,
        uint32_t data, uint16_t len, uint16_t type)
{
    parcel->header.args[index] = data;
    parcel->header.arglen[index] = len;
    parcel->header.argtype[index] = type;
    parcel->header.argused |= (1 << index); // set as used
}

static void lpc_parcel_clear_arg(lpc_parcel_t parcel, uint16_t index)
{
    parcel->header.args[index] = 0;
    parcel->header.arglen[index] = 0;
    parcel->header.argtype[index] = LPC_PARCEL_ARG_NONE;
    parcel->header.argused &= ~(1 << index); // set as unused
}

void lpc_parcel_dump_args(lpc_parcel_t parcel)
{
    printf("parcel args:\n");
    int i; for (i = 0; i < LPC_PARCEL_ARG_NR; i++) {
        if (parcel->header.argused & (1 << i)) {
            printf("arg%d: ", i);
            switch (lpc_parcel_get_arg_type(parcel, i))
            {
            case LPC_PARCEL_ARG_SEQUENCE:
                {
                    printf("seq[%d] ", parcel->header.arglen[i]);
                    int j; for (j = 0; j < parcel->header.arglen[i]; j++) {
                        printf("%d", parcel->data[parcel->header.args[i] + j]);                    
                    }        
                    printf("\n");
                }
                break;
            case LPC_PARCEL_ARG_STRING:
                printf("str[%d] %s\n", parcel->header.arglen[i], &parcel->data[parcel->header.args[i]]);            
                break;
            case LPC_PARCEL_ARG_INT:
                printf("int[%d] %d\n", parcel->header.arglen[i], parcel->header.args[i]);
                break;
            default:
                printf("unknown[%d] %d\n", parcel->header.arglen[i], parcel->header.args[i]);
                break;
            }
        }
    }
}

int lpc_parcel_write_string(lpc_parcel_t parcel, char *str)
{
    int i = lpc_parcel_alloc_arg_solt(parcel);
    if (i < 0)
        return -1;
    int len = strlen(str);
    int size = parcel->header.size + len + 1;
    if (size >= LPC_PARCEL_BUF_SIZE)
        return -1; // no free space
    lpc_parcel_set_arg(parcel, i, parcel->header.size, len + 1, LPC_PARCEL_ARG_STRING);
    char *buf = (char *) &parcel->data[parcel->header.size];
    memcpy(buf, str, len);
    buf[len] = '\0'; // 末尾追加0
    parcel->header.size = size;
    return 0;
}

int lpc_parcel_write_sequence(lpc_parcel_t parcel, void *buf, size_t len)
{
    int i = lpc_parcel_alloc_arg_solt(parcel);
    if (i < 0)
        return -1;
    int size = parcel->header.size + len + 1;
    if (size >= LPC_PARCEL_BUF_SIZE)
        return -1; // no free space
    lpc_parcel_set_arg(parcel, i, parcel->header.size, len, LPC_PARCEL_ARG_SEQUENCE);
    uint8_t *pbuf = (uint8_t *) &parcel->data[parcel->header.size];
    if (buf)
        memcpy(pbuf, buf, len);
    pbuf[len] = '\0'; // 末尾追加0
    parcel->header.size = size;
    return 0;
}


int lpc_parcel_write_int(lpc_parcel_t parcel, uint32_t num)
{
    int i = lpc_parcel_alloc_arg_solt(parcel);
    if (i < 0)
        return -1;
    lpc_parcel_set_arg(parcel, i, num, sizeof(num), LPC_PARCEL_ARG_INT);
    return 0;
}


int lpc_parcel_write_long(lpc_parcel_t parcel, uint64_t num)
{
    int i = lpc_parcel_alloc_arg_solt(parcel);
    if (i < 0)
        return -1;
    lpc_parcel_set_arg(parcel, i, num, sizeof(num), LPC_PARCEL_ARG_LONG);
    return 0;
}

int lpc_parcel_write_short(lpc_parcel_t parcel, uint16_t num)
{
    int i = lpc_parcel_alloc_arg_solt(parcel);
    if (i < 0)
        return -1;
    lpc_parcel_set_arg(parcel, i, num, sizeof(num), LPC_PARCEL_ARG_SHORT);
    return 0;
}

int lpc_parcel_write_char(lpc_parcel_t parcel, uint8_t num)
{
    int i = lpc_parcel_alloc_arg_solt(parcel);
    if (i < 0)
        return -1;
    lpc_parcel_set_arg(parcel, i, num, sizeof(num), LPC_PARCEL_ARG_CHAR);
    return 0;
}

int lpc_parcel_read_int(lpc_parcel_t parcel, uint32_t *num)
{
    int i = lpc_parcel_find_arg_solt(parcel, LPC_PARCEL_ARG_INT);
    if (i < 0) 
        return -1;
    if (num)
        *num = parcel->header.args[i]; // get num
    lpc_parcel_clear_arg(parcel, i);
    return 0;
}

int lpc_parcel_read_long(lpc_parcel_t parcel, uint64_t *num)
{
    int i = lpc_parcel_find_arg_solt(parcel, LPC_PARCEL_ARG_LONG);
    if (i < 0) 
        return -1;
    if (num)
        *num = parcel->header.args[i]; // get num
    lpc_parcel_clear_arg(parcel, i);
    return 0;
}

int lpc_parcel_read_short(lpc_parcel_t parcel, uint16_t *num)
{
    int i = lpc_parcel_find_arg_solt(parcel, LPC_PARCEL_ARG_SHORT);
    if (i < 0) 
        return -1;
    if (num)
        *num = parcel->header.args[i]; // get num
    lpc_parcel_clear_arg(parcel, i);
    return 0;
}

int lpc_parcel_read_char(lpc_parcel_t parcel, uint8_t *num)
{
    int i = lpc_parcel_find_arg_solt(parcel, LPC_PARCEL_ARG_CHAR);
    if (i < 0) 
        return -1;
    if (num)
        *num = parcel->header.args[i]; // get num
    lpc_parcel_clear_arg(parcel, i);
    return 0;
}

int lpc_parcel_read_string(lpc_parcel_t parcel, char **str)
{
    int i = lpc_parcel_find_arg_solt(parcel, LPC_PARCEL_ARG_STRING);
    if (i < 0) 
        return -1;
    if (str) {
        *str = (char *) &parcel->data[parcel->header.args[i]];
    }
    lpc_parcel_clear_arg(parcel, i);
    return 0;
}

int lpc_parcel_read_sequence(lpc_parcel_t parcel, void *buf, size_t *len)
{
    int i = lpc_parcel_find_arg_solt(parcel, LPC_PARCEL_ARG_SEQUENCE);
    if (i < 0) 
        return -1;
    if (len) {
        *len = parcel->header.arglen[i];
    }
    if (buf) {  
        memcpy(buf, (void *) &parcel->data[parcel->header.args[i]], parcel->header.arglen[i]);
    }
    lpc_parcel_clear_arg(parcel, i);
    return 0;
}

int lpc_parcel_read_sequence_buf(lpc_parcel_t parcel, void **buf, size_t *len)
{
    int i = lpc_parcel_find_arg_solt(parcel, LPC_PARCEL_ARG_SEQUENCE);
    if (i < 0) 
        return -1;
    if (buf) {
        *buf = (void *) &parcel->data[parcel->header.args[i]];
    }
    if (len) {
        *len = parcel->header.arglen[i];
    }
    lpc_parcel_clear_arg(parcel, i);
    return 0;
}

/**
 * 应答端口上面的请求
 * 首先会绑定一个端口，如果失败则返回错误
 * 然后就是分配端口的消息，分配失败则返回错误
 * 接下来就是一个循环，首先会从端口上接收消息，收到消息后需要根据消息内容调用对应的函数，
 * 执行完后，封装应答消息并进行消息应答。一直循环往复。
 */
int lpc_do_echo(uint32_t port, lpc_handler_t func)
{
#if LPC_MSG_USE_MALLOC == 1
    /* TODO: 需要在使用完成之后释放掉msg消息 */
    port_msg_t *msg_recv = NULL;
    port_msg_t *msg_reply = NULL;
    
    msg_recv = malloc(sizeof(port_msg_t));
    if (msg_recv == NULL) {
        fprintf(stderr, "lpc: malloc for recv msg failed!\n");
        return -1;
    }
    msg_reply = malloc(sizeof(port_msg_t));
    if (msg_reply == NULL) {
        fprintf(stderr, "lpc: malloc for reply msg failed!\n");
        free(msg_recv);
        return -1;
    }
#else
    port_msg_t msg_recv_buf;
    port_msg_t msg_reply_buf;
    memset(&msg_recv_buf, 0, sizeof(port_msg_t));
    memset(&msg_reply_buf, 0, sizeof(port_msg_t));
    port_msg_t *msg_recv = &msg_recv_buf;
    port_msg_t *msg_reply = &msg_reply_buf;
#endif  /* LPC_MSG_USE_MALLOC == 1 */
    while (1) {
        port_msg_reset(msg_recv);
        port_msg_reset(msg_reply);
        if (receive_port(port, msg_recv) < 0)
            continue;
        /* process msg */
        bool result = false;
        lpc_parcel_t recv_parcel = (lpc_parcel_t) msg_recv->data;
        lpc_parcel_t reply_parcel = (lpc_parcel_t) msg_reply->data;
        if (func)
            result = func(
                        recv_parcel->code,
                        recv_parcel,
                        reply_parcel);
        if (!result) {
            fprintf(stderr, "lpc: port %d do serv func failed!\n", port);
        }
        port_msg_copy_header(msg_recv, msg_reply);
        // 计算应答头大小
        msg_reply->header.size = sizeof(_lpc_parcel_t) + reply_parcel->header.size;
        msg_reply->header.size += sizeof(port_msg_header_t); 
        if (reply_port(port, msg_reply) < 0) {
            fprintf(stderr, "lpc: reply port %d failed!\n", port);
        }
    }
}

/**
 * 应答端口上面的请求
 * 首先会绑定一个端口，如果失败则返回错误
 * 然后就是分配端口的消息，分配失败则返回错误
 * 接下来就是一个循环，首先会从端口上接收消息，收到消息后需要根据消息内容调用对应的函数，
 * 执行完后，封装应答消息并进行消息应答。一直循环往复。
 */
int lpc_echo(uint32_t port, lpc_handler_t func)
{
    if (bind_port(port, 0) < 0) {
        fprintf(stderr, "lpc: bind port %d failed!\n", port);
        return -1;
    }
    return lpc_do_echo(port, func);
}

/**
 * 应答端口组上面的请求
 * 首先会绑定一个端口，如果失败则返回错误
 * 然后就是分配端口的消息，分配失败则返回错误
 * 接下来就是一个循环，首先会从端口上接收消息，收到消息后需要根据消息内容调用对应的函数，
 * 执行完后，封装应答消息并进行消息应答。一直循环往复。
 */
int lpc_echo_group(uint32_t port, lpc_handler_t func)
{
    if (bind_port(port, PORT_BIND_GROUP) < 0) {
        fprintf(stderr, "lpc: bind port group %d failed!\n", port);
        return -1;
    }
    return lpc_do_echo(port, func);
}

/**
 * 往port发起一个调用
 * 最开始先进行初始化，分配消息缓冲区，绑定一个自由端口。
 * 填写消息后往端口发起一个请求，然后复制应答数据到结构体中。
 */
int lpc_call(uint32_t port, uint32_t code, lpc_parcel_t data, lpc_parcel_t reply)
{
#if LPC_MSG_USE_MALLOC == 1
    port_msg_t *msg = NULL;
    /* 分配请求消息 */
    if (!msg) {
        msg = malloc(sizeof(port_msg_t));
        if (!msg)
            return -1;
    }
#else
    port_msg_t msg_buf;
    memset(&msg_buf, 0, sizeof(port_msg_t));
    port_msg_t *msg = &msg_buf;
#endif
    /* 绑定一个随机端口 */
    if (bind_port(-1, PORT_BIND_ONCE) < 0) {
        #if LPC_MSG_USE_MALLOC == 1
        free(msg);
        #endif
        return -1;
    }
    port_msg_reset(msg);
    data->code = code;
    int msglen = sizeof(_lpc_parcel_t) + data->header.size;
    memcpy(msg->data, data, msglen);
    // 计算请求头大小
    msg->header.size = sizeof(port_msg_header_t);
    msg->header.size += msglen;
    if (request_port(port, msg) < 0) {
        #if LPC_MSG_USE_MALLOC == 1
        free(msg);
        #endif
        return -1;
    }
    if (reply) {
        lpc_parcel_t reply_ = (lpc_parcel_t) msg->data;
        memcpy(reply, msg->data, sizeof(_lpc_parcel_t) + reply_->header.size);
    }
    #if LPC_MSG_USE_MALLOC == 1
    free(msg);
    #endif
    return 0;
}
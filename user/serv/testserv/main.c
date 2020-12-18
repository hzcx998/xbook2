#include <stdio.h>
#include <sys/portcomm.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <test.serv.h>

/* the args we support now */
#define LPC_PARCEL_ARG_NR 8

// TODO: add FLOAT & DOUBLE arg support

enum lpc_parcel_arg_type {
    LPC_PARCEL_ARG_NONE = 0,
    LPC_PARCEL_ARG_CHAR,
    LPC_PARCEL_ARG_SHORT,
    LPC_PARCEL_ARG_INT,
    LPC_PARCEL_ARG_LONG,
    LPC_PARCEL_ARG_FLOAT,   // not support
    LPC_PARCEL_ARG_DOUBLE,  // not support
    LPC_PARCEL_ARG_STRING,
    LPC_PARCEL_ARG_SEQUENCE
};

typedef struct {
    uint32_t args[LPC_PARCEL_ARG_NR];   // 参数值
    uint16_t arglen[LPC_PARCEL_ARG_NR]; // 表明参数的长度
    uint16_t argtype[LPC_PARCEL_ARG_NR];// 表明参数的类型
    uint32_t argused;       // 如果某位是1，则表示这个参数已经被使用
    uint32_t nextpos;       // 当参数是字符串或者缓冲区时，记录了下一个缓冲区的位置
} lpc_parcel_header_t;

#define LPC_PARCEL_HEADER_SIZE sizeof(lpc_parcel_header_t)
#define LPC_PARCEL_BUF_SIZE (PORT_MSG_SIZE - LPC_PARCEL_HEADER_SIZE)

typedef struct {
    lpc_parcel_header_t header;
    uint8_t data[1];         // 柔性数组数据
} *lpc_parcel_t;

typedef bool (*lpc_handler_t)(uint32_t, lpc_parcel_t, lpc_parcel_t);

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

uint16_t lpc_parcel_get_arg_type(lpc_parcel_t parcel, uint16_t index)
{
    return parcel->header.argtype[index];
}

int lpc_parcel_alloc_arg_solt(lpc_parcel_t parcel)
{
    if (!parcel)
        return -1;
    int i; for (i = 0; i < LPC_PARCEL_ARG_NR; i++) {
        if (!(parcel->header.argused & (1 << i))) 
            break;
    }
    if (i >= LPC_PARCEL_ARG_NR) {
        SERVPRINT("servparcel: no free arg left!\n");
        return -1;
    }
    return i;
}

int lpc_parcel_find_arg_solt(lpc_parcel_t parcel, uint16_t type)
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
        printf("servparcel: no free arg left!\n");
        return -1;
    }
    return i;
}

void lpc_parcel_set_arg(lpc_parcel_t parcel, uint16_t index,
        uint32_t data, uint16_t len, uint16_t type)
{
    parcel->header.args[index] = data;
    parcel->header.arglen[index] = len;
    parcel->header.argtype[index] = type;
    parcel->header.argused |= (1 << index); // set as used
}

void lpc_parcel_clear_arg(lpc_parcel_t parcel, uint16_t index)
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
    int nextpos = parcel->header.nextpos + len + 1;
    if (nextpos >= LPC_PARCEL_BUF_SIZE)
        return -1; // no free space
    lpc_parcel_set_arg(parcel, i, parcel->header.nextpos, len + 1, LPC_PARCEL_ARG_STRING);
    char *buf = (char *) &parcel->data[parcel->header.nextpos];
    memcpy(buf, str, len);
    buf[len] = '\0'; // 末尾追加0
    parcel->header.nextpos = nextpos;
    return 0;
}

int lpc_parcel_write_sequence(lpc_parcel_t parcel, uint8_t *buf, size_t len)
{
    int i = lpc_parcel_alloc_arg_solt(parcel);
    if (i < 0)
        return -1;
    int nextpos = parcel->header.nextpos + len + 1;
    if (nextpos >= LPC_PARCEL_BUF_SIZE)
        return -1; // no free space
    lpc_parcel_set_arg(parcel, i, parcel->header.nextpos, len, LPC_PARCEL_ARG_SEQUENCE);
    uint8_t *pbuf = (uint8_t *) &parcel->data[parcel->header.nextpos];
    if (buf)
        memcpy(pbuf, buf, len);
    pbuf[len] = '\0'; // 末尾追加0
    parcel->header.nextpos = nextpos;
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

int lpc_parcel_read_sequence(lpc_parcel_t parcel, uint8_t **buf, size_t *len)
{
    int i = lpc_parcel_find_arg_solt(parcel, LPC_PARCEL_ARG_SEQUENCE);
    if (i < 0) 
        return -1;
    if (buf) {
        *buf = (uint8_t *) &parcel->data[parcel->header.args[i]];
    }
    if (len) {
        *len = parcel->header.arglen[i];
    }
    lpc_parcel_clear_arg(parcel, i);
    return 0;
}

int lpc_echo(uint32_t port, lpc_handler_t func)
{
    static char this_init_done = 0;
    static port_msg_t *msg_recv = NULL;
    static port_msg_t *msg_reply = NULL;
    if (!this_init_done) {
        if (bind_port(port) < 0) {
            SERVPRINT("bind port failed!\n");
            return -1;
        }
        msg_recv = NULL;
        msg_reply = NULL;
        msg_recv = malloc(sizeof(port_msg_t));
        if (msg_recv == NULL) {
            SERVPRINT("malloc for recv msg failed!\n");
            return -1;
        }
        msg_reply = malloc(sizeof(port_msg_t));
        if (msg_reply == NULL) {
            SERVPRINT("malloc for reply msg failed!\n");
            free(msg_recv);
            return -1;
        }
        this_init_done = 1;
    }
    while (1) {
        port_msg_reset(msg_recv);
        port_msg_reset(msg_reply);
        if (receive_port(port, msg_recv) < 0)
            continue;
        /* process msg */
        bool result = false;
        if (func)
            result = func(
                        msg_recv->code,
                        (lpc_parcel_t) msg_recv->data,
                        (lpc_parcel_t) msg_reply->data);
        if (!result) {
            SERVPRINT("do serv func failed!\n");
        }
        port_msg_copy_header(msg_recv, msg_reply);
        if (reply_port(port, msg_reply) < 0) {
            SERVPRINT("reply port failed!\n");
        }
    }
}

int hello(char *str)
{
    SERVPRINT("say: %s\n", str);
    return strlen(str);
}

bool lpc_echo_main(uint32_t code, lpc_parcel_t data, lpc_parcel_t reply)
{
    switch (code)
    {
    case TESTSERV_hello:
        {
            char *sbuf;
            printf("server hello\n");
            lpc_parcel_read_string(data, &sbuf);
            int len = hello(sbuf);
            lpc_parcel_write_int(reply, len);
        }
        break;
    default:
        return false;
        break;
    }
    return true;
}

int lpc_call(uint32_t port, uint32_t code, lpc_parcel_t data, lpc_parcel_t reply)
{
    static char this_init_done = 0;
    static port_msg_t *msg = NULL;
    if (!this_init_done) {
        /* 分配请求消息 */
        if (!msg) {
            msg = malloc(sizeof(port_msg_t));
            if (!msg)
                return -1;
        }
        /* 绑定一个随机端口 */
        if (bind_port(-1) < 0) {
            free(msg);
            return -1;
        }
        this_init_done = 1;
    }
    port_msg_reset(msg);
    msg->code = code;
    if (data)
        memcpy(msg->data, data, PORT_MSG_SIZE);
    if (request_port(port, msg) < 0) {
        return -1;
    }
    if (reply)
        memcpy(reply, msg->data, PORT_MSG_SIZE);
    return 0;
}

int serv_hello(char *str) 
{
    #if 0
    lpc_parcel_t parcel = lpc_parcel_get();
    lpc_parcel_write_string(parcel, str);
    lpc_parcel_write_string(parcel, str);
    char seqbuf[32];
    int i; for (i = 0; i < 32; i++) {
        seqbuf[i] = 'a' + i;
    }
    lpc_parcel_write_sequence(parcel, seqbuf, 32);
    lpc_parcel_write_int(parcel, 10);
    lpc_parcel_write_int(parcel, 20);
    lpc_parcel_dump_args(parcel);
    uint32_t num;
    lpc_parcel_read_int(parcel, &num);
    printf("read num:%d\n", num);
    lpc_parcel_read_int(parcel, &num);
    printf("read num:%d\n", num);
    char *sbuf;
    lpc_parcel_read_string(parcel, &sbuf);
    printf("read string:%s\n", sbuf);
    lpc_parcel_read_string(parcel, &sbuf);
    printf("read string:%s\n", sbuf);
    size_t len;
    lpc_parcel_read_sequence(parcel, &sbuf, &len);
    printf("read seq:%s len: %d\n", sbuf + 1, len);
    lpc_parcel_dump_args(parcel);
    #endif
    #if 1
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_string(parcel, str);
    if (lpc_call(PORT_COMM_TEST, TESTSERV_hello, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int num; lpc_parcel_read_int(parcel, &num);
    lpc_parcel_put(parcel);
    return num;
    #endif
    return 0;
}

int main(int argc, char *argv[])
{
    printf("testserv start\n");
    
    pid_t pid = fork();
    if (pid > 0) {
        exit(lpc_echo(PORT_COMM_TEST, lpc_echo_main));
    } else {
        usleep(1000 * 10);
        printf("len:%d\n", serv_hello("hello, world!"));
    }
    return 0;
}
#include <stdio.h>
#include <sys/servcall.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <test.serv.h>

static servmsg_t *servmsg_recv_reply = NULL;
static servmsg_t *servmsg_request = NULL;

static char serv_init_done = 0;
#define SERVPARCEL_ARGS 8

#define SERVPARCEL_BUF_HEADER_SIZE (2*SERVPARCEL_ARGS*sizeof(uint32_t) + 3*sizeof(uint32_t)) 
#define SERVPARCEL_BUF_SIZE (SERVMSG_SIZE - SERVPARCEL_BUF_HEADER_SIZE)

typedef struct {
    uint32_t args[SERVPARCEL_ARGS];
    uint32_t arglen[SERVPARCEL_ARGS];
    uint32_t argtype;
    uint32_t argused;
    uint32_t next_pos;  // 下一个缓冲区的位置
    uint8_t buf[1];
} *servparcel_t;

typedef bool (*servcall_func_t)(uint32_t, servparcel_t, servparcel_t);

/*
int servparcel_read_int(servparcel_t data)
{
    return data->
}
*/

servparcel_t servparcel_get()
{
    servparcel_t parcel = malloc(SERVMSG_SIZE);
    if (parcel) {
        memset(parcel, 0, SERVMSG_SIZE);
    }
    return parcel;
}

int servparcel_put(servparcel_t parcel)
{
    if (parcel) {
        free(parcel);
        return 0;
    }
    return -1;
}

void servparcel_dump_args(servparcel_t parcel)
{
    printf("parcel args:\n");
    int i; for (i = 0; i < SERVPARCEL_ARGS; i++) {
        if (parcel->argused & (1 << i)) {
            printf("arg%d: ", i);
            if (parcel->argtype & (1 << i)) {
                printf("buf[%d] %s\n", parcel->arglen[i], &parcel->buf[parcel->args[i]]);
            } else {
                printf("int[%d] %d\n", parcel->arglen[i], parcel->args[i]);
            }
        }
    }
}

/*
basic arg type:
byte, char, short, int, long, float, double, boolen, string, charsequence
*/

int servparcel_write_string(servparcel_t parcel, char *str)
{
    int i; for (i = 0; i < SERVPARCEL_ARGS; i++) {
        if (!(parcel->argused & (1 << i))) 
            break;
    }
    if (i >= SERVPARCEL_ARGS) {
        printf("servparcel: no free arg left!\n");
        return -1;
    }
    int len = strlen(str);
    // 计算剩余的空间
    int next_pos = parcel->next_pos + len + 1;
    if (next_pos >= SERVPARCEL_BUF_SIZE)
        return -1; // no free space
    parcel->argused |= (1 << i); // set as used
    parcel->argtype |= (1 << i); // set as string or buf
    parcel->args[i] = parcel->next_pos; // record pos
    parcel->arglen[i] = len + 1; // record len
    char *buf = (char *) &parcel->buf[parcel->next_pos];
    memcpy(buf, str, len);
    buf[len] = '\0'; // 末尾追加0
    parcel->next_pos = len + 1;
    return 0;
}

int servparcel_write_charseq(servparcel_t parcel, uint8_t *buf, size_t len)
{
    int i; for (i = 0; i < SERVPARCEL_ARGS; i++) {
        if (!(parcel->argused & (1 << i))) 
            break;
    }
    if (i >= SERVPARCEL_ARGS) {
        printf("servparcel: no free arg left!\n");
        return -1;
    }
    // 计算剩余的空间
    int next_pos = parcel->next_pos + len + 1;
    if (next_pos >= SERVPARCEL_BUF_SIZE)
        return -1; // no free space
    parcel->argused |= (1 << i); // set as used
    parcel->argtype |= (1 << i); // set as string or buf
    parcel->args[i] = parcel->next_pos; // record pos
    parcel->arglen[i] = len; // record len
    uint8_t *pbuf = (uint8_t *) &parcel->buf[parcel->next_pos];
    memcpy(pbuf, buf, len);
    pbuf[len] = '\0'; // 末尾追加0
    parcel->next_pos = next_pos;
    return 0;
}

int servparcel_write_int(servparcel_t parcel, uint32_t num)
{
    int i; for (i = 0; i < SERVPARCEL_ARGS; i++) {
        if (!(parcel->argused & (1 << i))) 
            break;
    }
    if (i >= SERVPARCEL_ARGS) {
        printf("servparcel: no free arg left!\n");
        return -1;
    }
    parcel->argused |= (1 << i); // set as used
    parcel->args[i] = num; // record num
    parcel->arglen[i] = sizeof(num); // record num
    return 0;
}

int servparcel_read_int(servparcel_t parcel, uint32_t *num)
{
    int i; for (i = 0; i < SERVPARCEL_ARGS; i++) {
        if ((parcel->argused & (1 << i))) {
            if (!(parcel->argtype & (1 << i)) && parcel->arglen[i] == sizeof(uint32_t)) { // 是数值型
                break;
            }
        }
    }
    if (i >= SERVPARCEL_ARGS) {
        printf("servparcel: no free arg left!\n");
        return -1;
    }
    parcel->argused &= ~(1 << i); // set as unsed
    if (num)
        *num = parcel->args[i]; // get num
    parcel->arglen[i] = 0; // clean len
    parcel->args[i] = 0;    // clean num
    return 0;
}

int servparcel_read_string(servparcel_t parcel, char *str, size_t maxlen)
{
    int i; for (i = 0; i < SERVPARCEL_ARGS; i++) {
        if ((parcel->argused & (1 << i))) {
            if ((parcel->argtype & (1 << i)) && parcel->arglen[i] > 0) { // 是数值型
                break;
            }
        }
    }
    if (i >= SERVPARCEL_ARGS) {
        printf("servparcel: no free arg left!\n");
        return -1;
    }
    parcel->argused &= ~(1 << i); // set as unsed
    parcel->argtype &= ~(1 << i); // clean type
    if (str) {
        memcpy(str, &parcel->buf[parcel->args[i]], min(parcel->arglen[i], maxlen));
    }
    parcel->arglen[i] = 0;
    parcel->args[i] = 0; // clean len
    return 0;
}

int servparcel_read_charseq(servparcel_t parcel, uint8_t *buf, size_t *len, size_t maxlen)
{
    int i; for (i = 0; i < SERVPARCEL_ARGS; i++) {
        if ((parcel->argused & (1 << i))) {
            if ((parcel->argtype & (1 << i)) && parcel->arglen[i] > 0) { // 是数值型
                break;
            }
        }
    }
    if (i >= SERVPARCEL_ARGS) {
        printf("servparcel: no free arg left!\n");
        return -1;
    }
    parcel->argused &= ~(1 << i); // set as unsed
    parcel->argtype &= ~(1 << i); // clean type
    if (buf) {
        memcpy(buf, &parcel->buf[parcel->args[i]], min(parcel->arglen[i], maxlen));
    }
    if (len) {
        *len = parcel->arglen[i];
    }
    parcel->arglen[i] = 0; // clean len
    parcel->args[i] = 0; // clean len
    return 0;
}

int servcall_init(uint32_t port)
{
    if (serv_init_done)
        return -1;
    if (bind_port(port) < 0) {
        SERVPRINT("bind port failed!\n");
        return -1;
    }
    servmsg_recv_reply = NULL;
    servmsg_recv_reply = malloc(sizeof(servmsg_t));
    if (servmsg_recv_reply == NULL) {
        SERVPRINT("malloc for serv msg failed!\n");
        return -1;
    }
    serv_init_done = 1;
    return 0;
}

int servcall_loop(uint32_t port, servcall_func_t func)
{
    if (!serv_init_done)
        return -1;
    while (1) {
        servmsg_reset(servmsg_recv_reply);
        if (receive_port(port, servmsg_recv_reply) < 0)
            continue;
        /* process msg */
        bool result = false;
        if (func)
            result = func(
                        servmsg_recv_reply->code,
                        (servparcel_t) servmsg_recv_reply->data,
                        (servparcel_t) servmsg_recv_reply->data);
        if (!result) {
            SERVPRINT("do serv func failed!\n");
        }
        if (reply_port(port, servmsg_recv_reply) < 0) {
            SERVPRINT("reply port failed!\n");
        }
    }
}

int hello(char *str)
{
    SERVPRINT("say: %s\n", str);
    return strlen(str);
}

bool on_serv(uint32_t code, servparcel_t data, servparcel_t reply)
{
    char sbuf[32] = {0};
    int num;
    switch (code)
    {
    case TESTSERV_hello:
        printf("server hello\n");
        servparcel_read_string(data, sbuf, 32);
        int len = hello(sbuf);
        servparcel_write_int(reply, len);
        break;
    default:
        return false;
        break;
    }
    return true;
}

int servcall(uint32_t port, uint32_t code, servparcel_t data, servparcel_t reply)
{
    if (!servmsg_request) {
        servmsg_request = malloc(sizeof(servmsg_t));
        if (!servmsg_request)
            return -1;
    }
    servmsg_reset(servmsg_request);
    servmsg_request->code = code;
    if (data)
        memcpy(servmsg_request->data, data, SERVMSG_SIZE);
    request_port(port, servmsg_request);
    if (reply)
        memcpy(reply, servmsg_request->data, SERVMSG_SIZE);
    return 0;
}

int servcall_safe(servmsg_t *msg, uint32_t port, uint32_t code, servparcel_t data, servparcel_t reply)
{
    return 0;
}

int serv_hello(char *str) 
{
    #if 0
    servparcel_t parcel = servparcel_get();
    servparcel_write_string(parcel, str);
    servparcel_write_string(parcel, str);
    servparcel_write_int(parcel, 10);
    servparcel_write_int(parcel, 20);
    servparcel_dump_args(parcel);
    uint32_t num;
    servparcel_read_int(parcel, &num);
    printf("read num:%d\n", num);
    servparcel_read_int(parcel, &num);
    printf("read num:%d\n", num);
    char sbuf[32] = {0};
    servparcel_read_string(parcel, sbuf, 32);
    printf("read string:%s\n", sbuf);
    memset(sbuf, 0, 32);
    servparcel_read_string(parcel, sbuf, 32);
    printf("read string:%s\n", sbuf);
    servparcel_dump_args(parcel);
    #endif
    servparcel_t parcel = servparcel_get();
    servparcel_write_string(parcel, str);
    printf("client write arg\n");

    servcall(SERVPORT_TEST, TESTSERV_hello, parcel, parcel);
    printf("client call done\n");
    int num;
    servparcel_read_int(parcel, &num);
    printf("client get retval\n");
    
    servparcel_put(parcel);
    return num;
}

int main(int argc, char *argv[])
{
    printf("testserv start\n");
    servcall_init(SERVPORT_TEST);
    pid_t pid = fork();
    if (pid > 0) {
        servcall_loop(SERVPORT_TEST, on_serv);
    } else {
        bind_port(-1);
        printf("len:%d\n", serv_hello("hello, world!"));
    }
    return 0;
}


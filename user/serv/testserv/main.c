#include <stdio.h>
#include <sys/portcomm.h>
#include <sys/lpc.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <test.serv.h>

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
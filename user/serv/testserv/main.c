#include <stdio.h>
#include <sys/portcomm.h>
#include <sys/lpc.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <test.service.h>
#include <test.h>
#include <test.client.h>

/* 将服务分词服务端和客户端。服务端接口在服务进程中实现。客户端接口以库的形式呈现，让用户直接使用。
test.client.h
test.service.h
用户库封装test.client.h的接口，变成xxx.h接口
服务器封装test.service.h的接口，直接在服务器中实现
*/

int remote_hello(char *str)
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
            lpc_parcel_read_string(data, &sbuf);
            int len = remote_hello(sbuf);
            lpc_parcel_write_int(reply, len);
        }
        break;
    default:
        return false;
        break;
    }
    return true;
}

int main(int argc, char *argv[])
{
    printf("testserv start\n");
    pid_t pid = fork();
    if (pid > 0) {
        exit(lpc_echo(LPC_ID_TEST, lpc_echo_main));
    } else {
        usleep(1000 * 10); // wait for server init done
        printf("len:%d\n", hello("hello, world!"));
    }
    return 0;
}
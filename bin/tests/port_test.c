#include "test.h"
#include <sys/portcomm.h>
#include <signal.h>

pid_t child_pid;

#define SERV_PORT   5
#define CLIENT_PORT   6

int port_comm_test(int argc, char *argv[])
{
    bind_port(SERV_PORT, 0);
    
    pid_t pid = fork();
    if (pid < 0) {
        printf("fork failed!\n");
        return -1;
    }
    port_msg_t msg;
    int count = 10;
    if (pid > 0) {
        child_pid = pid;
        while (count > 0)
        {
            port_msg_reset(&msg);
            if (receive_port(SERV_PORT, &msg) < 0)
                continue;
            
            printf("server: recv %d: %s\n", msg.header.size, (char *) msg.data);

            char *str = "hello, client!\n";
            strcpy((char *)msg.data, str);
            msg.header.size = strlen(str);
            sleep(1);
            if (!reply_port(SERV_PORT, &msg)) {
                printf("server: reply ok!\n");
            }
            count--;
        }
        printf("server: exit!\n");
    } else {

        bind_port(CLIENT_PORT, 0);
        while (count > 0)
        {
            port_msg_reset(&msg);
            char *str = "hello, i am client!\n";
            strcpy((char *)msg.data, str);
            msg.header.size = strlen(str);
            if (!request_port(SERV_PORT, &msg))
                printf("client: recv %d: %s\n", msg.header.size, (char *) msg.data);
            count--;
        }
        printf("client: exit!\n");        
        sleep(1);
    }
    return 0;
}

static void bind_server() 
{
    bind_port(SERV_PORT, PORT_BIND_GROUP);
    pid_t pid = fork();
    if (pid < 0) {
        printf("fork failed!\n");
        return;
    }
    port_msg_t msg;
    int count = 3;
    if (pid > 0) {  /* 父进程 */
        child_pid = pid;
        while (count > 0)
        {
            port_msg_reset(&msg);
            if (receive_port(SERV_PORT, &msg) < 0)
                continue;
            
            printf("pid=%d server: recv %d: %s\n",  getpid(),msg.header.size, (char *) msg.data);

            char *str = "hello, client!\n";
            strcpy((char *)msg.data, str);
            msg.header.size = strlen(str);
            sleep(1);
            if (!reply_port(SERV_PORT, &msg)) {
                printf("server: reply ok!\n");
            }
            count--;
        }
        printf("server: exit!\n");
        exit(0);
    }
}

static void bind_client() 
{
    int count = 6;
    bind_port(CLIENT_PORT, 0);
    port_msg_t msg;
    while (count > 0) {
        port_msg_reset(&msg);
        char *str = "hello, i am client!\n";
        strcpy((char *)msg.data, str);
        msg.header.size = strlen(str);
        if (!request_port(SERV_PORT, &msg))
            printf("client: recv %d: %s\n", msg.header.size, (char *) msg.data);
        count--;
    }
    printf("client: exit!\n");        
    sleep(1);
    exit(0);
}

int port_comm_test2(int argc, char *argv[])
{
    /* 创建2个服务 */
    bind_server();
    bind_server();
    bind_client();
    return 0;
}


static void bind_server2() 
{
    bind_port(SERV_PORT, PORT_BIND_GROUP);
    pid_t pid = fork();
    if (pid < 0) {
        printf("fork failed!\n");
        return;
    }
    port_msg_t msg;
    int count = 2000;
    if (pid > 0) {  /* 父进程 */
        child_pid = pid;
        while (count > 0)
        {
            port_msg_reset(&msg);
            if (receive_port(SERV_PORT, &msg) < 0)
                continue;
            
            printf("pid=%d server: recv %d: %s\n",  getpid(),msg.header.size, (char *) msg.data);
            usleep(1000 * 50);
            char *str = "hello, client!\n";
            strcpy((char *)msg.data, str);
            msg.header.size = strlen(str);
            if (!reply_port(SERV_PORT, &msg)) {
                printf("server: reply ok!\n");
            }
            count--;
        }
        printf("server: exit!\n");
        exit(0);
    }
}

static void bind_client2() 
{
    int count = 1000;
    bind_port(CLIENT_PORT, 0);
    port_msg_t msg;
    while (count > 0) {
        port_msg_reset(&msg);
        char *str = "hello, i am client!\n";
        strcpy((char *)msg.data, str);
        msg.header.size = strlen(str);
        if (!request_port(SERV_PORT, &msg))
            printf("client: recv %d: %s\n", msg.header.size, (char *) msg.data);
        count--;
    }
    printf("client: exit!\n");        
    sleep(1);
    exit(0);
}

int port_comm_test3(int argc, char *argv[])
{
    /* 创建2个服务 */
    bind_server2();
    bind_server2();
    bind_server2();
    bind_server2();
    bind_server2();
    bind_server2();
    bind_server2();
    bind_server2();
    bind_client2();
    return 0;
}
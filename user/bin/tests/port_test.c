#include "test.h"
#include <sys/portcomm.h>
#include <signal.h>

pid_t child_pid;

int port_comm_test(int argc, char *argv[])
{
    bind_port(0);
    
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
            if (receive_port(0, &msg) < 0)
                continue;
            
            printf("server: recv %d: %s\n", msg.size, (char *) msg.data);

            char *str = "hello, client!\n";
            strcpy(msg.data, str);
            msg.size = strlen(str);
            sleep(1);
            if (!reply_port(0, &msg)) {
                printf("server: reply ok!\n");
            }
            count--;
        }
        printf("server: exit!\n");
    } else {

        bind_port(1);
        while (count > 0)
        {
            port_msg_reset(&msg);
            char *str = "hello, i am client!\n";
            strcpy(msg.data, str);
            msg.size = strlen(str);
            if (!request_port(0, &msg))
                printf("client: recv %d: %s\n", msg.size, (char *) msg.data);
            count--;
        }
        printf("client: exit!\n");        
        sleep(1);
    }
}
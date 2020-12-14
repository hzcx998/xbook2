#include "test.h"
#include <sys/lpc.h>

int lpc_test(int argc, char *argv[])
{
    int phandle = create_port("lpc_test", 3, 64);
    if (phandle < 0) {
        printf("create port failed!\n");
        return -1;
    }
    printf("create port %d ok!\n", phandle);

    pid_t pid = fork();
    if (pid < 0) {
        printf("fork failed!\n");
        return -1;
    }
    lpc_message_t msg;
    int count = 10;
    if (pid > 0) {
        int client_port = accept_port(phandle, 1);
        printf("accept port %d ok!\n", client_port);
        while (count > 0)
        {
            lpc_reset_message(&msg);
            if (receive_port(client_port, &msg) < 0)
                continue;

            printf("server: recv %d: %s\n", msg.id, (char *) msg.data);

            char *str = "hello, client!\n";
            strcpy(msg.data, str);
            msg.size = strlen(str);

            if (!reply_port(client_port, &msg)) {
                printf("server: reply ok!\n");
            }
            count--;
        }
        printf("server: exit!\n");
    } else {
        uint32_t max_msgsz;
        int myport = connect_port("lpc_test", &max_msgsz);
        printf("connect port %d ok!\n", myport);
        while (count > 0)
        {
            lpc_reset_message(&msg);
            char *str = "hello, i am client!\n";
            strcpy(msg.data, str);
            msg.size = strlen(str);
            request_port(myport, &msg);
            printf("client: recv %d: %s\n", msg.id, (char *) msg.data);
            
            count--;
        }
        printf("client: exit!\n");        
    }
    
    
}
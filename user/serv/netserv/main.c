#include <stdio.h>
#include <netserv.h>
#include <sys/portcomm.h>
#include <netsocket.h>

int main(int argc, char *argv[])
{
    printf("netserv start\n");
    network_init();
    while (1) {
        
    }
    return 0;
}

void client()
{
    usleep(1000 * 1000);
    printf("call net_socket\n");
    int sock = net_socket(PF_INET, SOCK_DGRAM, 0);
    printf("sock %d\n", sock);
    exit(0);
}
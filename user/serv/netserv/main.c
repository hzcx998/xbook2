#include <stdio.h>
#include <netserv.h>
#include <sys/servcall.h>

int main(int argc, char *argv[])
{
    printf("netserv start\n");
    network_init();
    while (1) {
        
    }
    return 0;
}

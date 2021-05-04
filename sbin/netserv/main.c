#include <stdio.h>
#include <netserv.h>
#include <sys/portcomm.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    printf("netserv start\n");
    network_init();
    return 0;
}
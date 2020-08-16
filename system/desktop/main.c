#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <graph.h>
#include "desktop.h"

int main(int argc, char *argv[])
{
    printf("[desktop]: begin.\n");

    if (init_desktop() < 0) {
        return -1;
    }
    if (init_taskbar() < 0) {
        return -1;
    }
    int win = g_layer_new(200, 0, 400, 200);
    if (win < 0)
        printf("layer new failed!\n");
    g_layer_z(win, 1);
    g_layer_rect(win, 0, 0, 400, 200, 0xff00ff00);
    g_layer_refresh(win, 0, 0, 400, 200);

    while (1)
    {

    }
    
    return 0;    
}

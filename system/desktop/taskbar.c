#include "desktop.h"
#include <graph.h>
#include <stdio.h>
#include <malloc.h>

int taskbar_layer;
int init_taskbar()
{
    int layer = g_layer_new(0, 0, desktop_width, TASKBAR_HEIGHT);
    if (layer < 0) {
        printf("[desktop]: new taskbar layer failed!\n");
        return -1;
    }
        
    taskbar_layer = layer;
    g_layer_z(layer, 1);    /* layer z = 1 */
    g_layer_rect_fill(layer, 0, 0, desktop_width, TASKBAR_HEIGHT, GC_GREEN);
    g_layer_refresh(layer, 0, 0, desktop_width, TASKBAR_HEIGHT);
    
    /* set task bar as win top, thus win should be lower than task bar layer */
    g_layer_set_wintop(layer);
    
    return 0;
}
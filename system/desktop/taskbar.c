#include "desktop.h"
#include <gapi.h>
#include <stdio.h>
#include <malloc.h>

int taskbar_layer;
int init_taskbar()
{
    int layer = g_layer_new(0, 0, g_screen.width, TASKBAR_HEIGHT);
    if (layer < 0) {
        printf("[desktop]: new taskbar layer failed!\n");
        return -1;
    }
    
    taskbar_layer = layer;
    g_layer_z(layer, 1);    /* layer z = 1 */
    g_layer_rect_fill(layer, 0, 0, g_screen.width, TASKBAR_HEIGHT, GC_GRAY);
    g_layer_refresh(layer, 0, 0, g_screen.width, TASKBAR_HEIGHT);
    
    /* set task bar as win top, thus win should be lower than task bar layer */
    g_layer_set_wintop(layer);
    g_region_t rg;
    rg.left = 0;
    rg.top = TASKBAR_HEIGHT;
    rg.right = g_screen.width;
    rg.bottom = g_screen.height - TASKBAR_HEIGHT;
    
    /* gui 系统必须做的事情 */
    g_screen_set_window_region(&rg);
    g_screen_get(&g_screen);    /* 更新屏幕信息 */
    return 0;
}

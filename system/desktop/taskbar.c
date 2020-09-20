#include "desktop.h"
#include <taskbar.h>
#include <winctl.h>
#include <gapi.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/list.h>

taskbar_t taskbar;

int init_taskbar()
{
    taskbar.width = g_screen.width;
    taskbar.height = TASKBAR_HEIGHT;
    taskbar.color = TASKBAR_COLOR;
    
    init_list(&taskbar.touch_list);

    printf("[desktop]: alloc taskbar layer start.\n");

    int layer = g_layer_new(0, g_screen.height - taskbar.height, taskbar.width, taskbar.height);
    if (layer < 0) {
        printf("[desktop]: new taskbar layer failed!\n");
        return -1;
    }
    taskbar.layer = layer;
    g_layer_z(layer, 1);    /* layer z = 1 */
    g_layer_rect_fill(layer, 0, 0, taskbar.width, taskbar.height, taskbar.color);
    g_layer_refresh(layer, 0, 0, taskbar.width, taskbar.height);
    
    /* set task bar as win top, thus win should be lower than task bar layer */
    g_layer_set_wintop(layer);
    g_region_t rg;
    rg.left = 0;
    rg.top = taskbar.height;
    rg.right = g_screen.width;
    rg.bottom = g_screen.height;
    
    /* gui 系统必须做的事情 */
    g_screen_set_window_region(&rg);
    g_screen_get(&g_screen);    /* 更新屏幕信息 */

    if (init_winctl_manager(layer) < 0)
        return -1;

    return 0;
}

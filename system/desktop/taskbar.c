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

    int layer = g_new_layer(0, g_screen.height - taskbar.height, taskbar.width, taskbar.height);
    if (layer < 0) {
        printf("[desktop]: new taskbar layer failed!\n");
        return -1;
    }
    taskbar.layer = layer;

    taskbar.render = g_new_bitmap(taskbar.width, taskbar.height);
    if (taskbar.render == NULL) {
        printf("[desktop]: new taskbar bitmap failed!\n");
        g_del_layer(layer);
        return -1;
    }

    g_set_layer_z(layer, 1);    /* layer z = 1 */
    
    g_rectfill(taskbar.render, 0, 0, taskbar.render->width, taskbar.render->height, GC_GRAY);
    g_bitmap_sync(taskbar.render, layer, 0, 0);

    /* set task bar as win top, thus win should be lower than task bar layer */
    g_set_layer_wintop(layer);
    g_region_t rg;
    rg.left = 0;
    rg.top = 0;
    rg.right = g_screen.width;
    rg.bottom = g_screen.height - taskbar.height;
    
    /* gui 系统必须做的事情 */
    g_set_screen_window_region(&rg);
    g_get_screen(&g_screen);    /* 更新屏幕信息 */

    if (init_winctl_manager(layer) < 0)
        return -1;

    return 0;
}

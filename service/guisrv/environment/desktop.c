#include <string.h>
#include <stdio.h>
#include <drivers/screen.h>
#include <window/window.h>
#include <window/draw.h>

#include <environment/desktop.h>

env_desktop_t env_desktop;

int init_env_desktop()
{
    /* 桌面窗口 */
    env_desktop.window = gui_create_window(NULL, 0, 0, 
        drv_screen.width, drv_screen.height, GUIW_NO_TITLE, NULL);

    if (env_desktop.window == NULL) {
        printf("create desktop window failed!\n");
        return -1;
    }
    gui_window_draw_rect_fill(env_desktop.window, 0, 0, env_desktop.window->width, 
        env_desktop.window->height, COLOR_RGB(0, 128, 192));
    gui_window_update(env_desktop.window, 0, 0, env_desktop.window->width, env_desktop.window->height);

    return 0;
}

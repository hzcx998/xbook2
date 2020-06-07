#include <string.h>
#include <stdio.h>
#include <drivers/screen.h>
#include <window/window.h>
#include <window/draw.h>

#include <environment/desktop.h>
#include <widget/label.h>
#include <widget/button.h>

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

    /* 设置当前窗口 */
    current_window = env_desktop.window;

    gui_label_t *label = gui_create_label(GUI_LABEL_TEXT, 0, 100, 0, 0);
    label->set_text(label, "hello, world!\n");
    label->add(label, env_desktop.window->layer);
    label->show(label);

    gui_button_t *button = gui_create_button(GUI_LABEL_TEXT, 0,400, 40, 20);
    button->add(button, env_desktop.window->layer);
    button->show(button);
    
    return 0;
}

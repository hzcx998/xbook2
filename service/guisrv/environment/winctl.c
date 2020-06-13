#include <environment/winctl.h>
#include <environment/desktop.h>
#include <drivers/screen.h>
#include <window/draw.h>
#include <guisrv.h>
#include <stdio.h>

/* 默认的图标数据 */
static GUI_COLOR winctl_icon_data[12 * 12] = {
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

gui_winctl_manager_t winctl_manager;

int winctl_btn_down_handler(gui_button_t *button, int btn, int local_mx, int local_my)
{
    printf("[button] down handler %d, %d, %d\n", btn, local_mx, local_my);
    int retval = 0;
    gui_window_t *win = (gui_window_t *) button->data;
    
    printf("[button] win id %d\n", win->id);
    
    return retval;
}

int winctl_btn_up_handler(gui_button_t *button, int btn, int local_mx, int local_my)
{
    printf("[button] up handler %d, %d, %d\n", btn, local_mx, local_my);

    int retval = 0;
    gui_window_t *win = (gui_window_t *) button->data;
    
    printf("[button] win id %d\n", win->id);
    
    return retval;
}

gui_winctl_t *gui_create_winctl(gui_window_t *window)
{
    if (!window)
        return NULL;

    gui_winctl_t *winctl = gui_malloc(sizeof(gui_winctl_t));
    if (winctl == NULL)
        return NULL;

    winctl->button = gui_create_button(GUI_LABEL_PIXMAP, 0, 0, 
        GUI_WINCTL_ICON_SIZE + 4, GUI_WINCTL_ICON_SIZE + 4);

    if (winctl->button == NULL) {
        gui_free(winctl);
        return NULL;
    }
    
    winctl->button->set_data(winctl->button, window);
    winctl->button->set_handler(winctl->button, winctl_btn_down_handler,
        winctl_btn_up_handler);

    winctl->button->set_color(winctl->button, winctl_manager.backcolor, 0);
    winctl->button->set_color3(winctl->button, winctl_manager.backcolor,
        winctl_manager.backcolor + 0x202020, winctl_manager.backcolor + 0x101010);
    winctl->button->set_align(winctl->button, GUI_WIDGET_ALIGN_CENTER);
    winctl->button->set_pixmap(winctl->button, 12,
        12, winctl_icon_data);
    
    winctl->button->add(winctl->button, winctl_manager.window->layer);

    winctl->window = window;
    
    return winctl;
}

int gui_winctl_add(gui_winctl_t *winctl)
{
    if (!winctl)
        return -1;
    if (list_find(&winctl->list, &winctl_manager.winctl_list_head))
        return -1;
    list_add_tail(&winctl->list, &winctl_manager.winctl_list_head);
    return 0;
}

int gui_winctl_del(gui_winctl_t *winctl)
{
    if (!winctl)
        return -1;

    if (!list_find(&winctl->list, &winctl_manager.winctl_list_head))
        return -1;

    list_del(&winctl->list);
    return 0;
}


int gui_destroy_winctl(gui_winctl_t *winctl)
{
    if (!winctl)
        return -1;
    
    gui_button_destroy(winctl->button);
    return 0;
}

void gui_winctl_show()
{
    /* 先刷新背景，再显示 */
    gui_window_draw_rect_fill(winctl_manager.window, 0, 0, 
        winctl_manager.window->width, winctl_manager.window->height, winctl_manager.backcolor);
    gui_window_update(winctl_manager.window, 0, 0, 
        winctl_manager.window->width, winctl_manager.window->height);
    
    int x = 4, y = 4;
    gui_winctl_t *winctl;
    list_for_each_owner (winctl, &winctl_manager.winctl_list_head, list) {
        winctl->button->set_location(winctl->button, x, y);
        winctl->button->show(winctl->button);
        y += GUI_WINCTL_ICON_SIZE + 4 + 2;
        /* 显示到最低端就不显示 */
        if (y >= env_desktop.window->height - GUI_WINCTL_ICON_SIZE - 4)
            break;
    }
}

int init_winctl_manager()
{
    init_list(&winctl_manager.winctl_list_head);
    winctl_manager.backcolor = GUI_WINCTL_BACKCOLOR;
    winctl_manager.window = gui_create_window(
        NULL, 0, 0, GUI_WINCTL_WIDTH, env_desktop.window->height,
        winctl_manager.backcolor, GUIW_NO_TITLE | GUIW_FIXED, env_desktop.window);

    if (winctl_manager.window == NULL)
        return -1;

    gui_window_show(winctl_manager.window);

    gui_winctl_t *winctl0 = gui_create_winctl(env_desktop.window);
    if (winctl0 == NULL)
        return -1;

    gui_winctl_add(winctl0);

    gui_winctl_t *winctl1 = gui_create_winctl(env_desktop.window);
    if (winctl0 == NULL)
        return -1;
    gui_winctl_add(winctl1);

    gui_winctl_t *winctl2 = gui_create_winctl(env_desktop.window);
    if (winctl0 == NULL)
        return -1;
    gui_winctl_add(winctl2);
    
    gui_winctl_show();

    return 0;    
}
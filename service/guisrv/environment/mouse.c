#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <drivers/screen.h>
#include <layer/draw.h>
#include <environment/mouse.h>

#include <layer/layer.h>
#include <window/window.h>

env_mouse_t env_mouse = {0};

extern list_t layer_show_list_head;


static void __left_button_down()
{
    printf("[mouse ] left button down.\n");
    printf("[mouse ] x:%d, y:%d\n", env_mouse.x, env_mouse.y);

    /* 如果抓住窗口后，就不能判断点击 */
    if (env_mouse.hold_window)
        return;

    gui_window_t *window;
    layer_t *layer;
    int local_mx, local_my;
    /* 查看点击的位置，看是否是一个 */
    list_for_each_owner_reverse (layer, &layer_show_list_head, list) {
        if (layer == layer_topest)
            continue;
        if (layer->extension == NULL)
            continue;
        
        window = (gui_window_t *) layer->extension;
        printf("[mouse ] fond a window %x.\n", window);
     
        local_mx = env_mouse.x - window->x;
        local_my = env_mouse.y - window->y;
        if (local_mx >= 0 && local_mx < window->width && 
            local_my >= 0 && local_my < window->height) {
            printf("touch window: pos=%d-%d, size=%d-%d\n", 
                window->x, window->y, window->width, window->height);
                    
            if (!(window->attr & GUIW_NO_TITLE)) {
                
                /* 有标题才判断标题栏 */
                if (local_my < GUIW_TITLE_HEIGHT) { /* in title */
                    /* 判断是否为控制按钮，如果是，就处理，不是就可能要移动窗口，切换窗口 */
                    printf("in window title\n");

                    env_mouse.local_x = local_mx;
                    env_mouse.local_y = local_my;
                    
                    env_mouse.hold_window = window;

                    /* 如果不是活动窗口，就切换成为活动窗口 */

                    break;
                } else {    /* in body */
                    /* 发送消息给窗口，如果不是激活窗口，就先激活 */
                    printf("in window body\n");
                    
                }
            } else {    /* 没有标题，就发送消息，切换窗口并激活 */
                printf("no window title\n");
                    
            }
            break;
        }
    }
}

static void __left_button_up()
{
    printf("[mouse ] left button up.\n");
    printf("[mouse ] x:%d, y:%d\n", env_mouse.x, env_mouse.y);
    /* 处于抓住窗口，所以就释放窗口 */
    if (env_mouse.hold_window) {
        /* 更新窗口的位置 */
        env_mouse.hold_window->x = env_mouse.hold_window->layer->x;
        env_mouse.hold_window->y = env_mouse.hold_window->layer->y;

        env_mouse.hold_window = NULL;
    } else {    /* 没有抓住窗口，简单传输按键弹起消息 */
        

    }
}

static void __right_button_down()
{
    printf("[mouse ] right button down.\n");
    printf("[mouse ] x:%d, y:%d\n", env_mouse.x, env_mouse.y);
}

static void __right_button_up()
{
    printf("[mouse ] right button up.\n");
    printf("[mouse ] x:%d, y:%d\n", env_mouse.x, env_mouse.y);
}

static void __middle_button_down()
{
    printf("[mouse ] middle button down.\n");
    printf("[mouse ] x:%d, y:%d\n", env_mouse.x, env_mouse.y);
}

static void __middle_button_up()
{
    printf("[mouse ] middle button up.\n");
    printf("[mouse ] x:%d, y:%d\n", env_mouse.x, env_mouse.y);
}

void env_mouse_move()
{
    /* 设定鼠标移动消息 */
    //printf("[mouse ] x:%d, y:%d\n", env_mouse.x, env_mouse.y);
    /* 尝试移动鼠标 */
    if (layer_topest) {
        layer_set_xy(layer_topest, env_mouse.x, env_mouse.y);
        if (env_mouse.hold_window) {    /* 让抓住的窗口跟住鼠标移动 */
            layer_set_xy(env_mouse.hold_window->layer, env_mouse.x - 
                env_mouse.local_x, env_mouse.y - env_mouse.local_y);
        }
    }
}

int init_env_mouse()
{
    env_mouse.x = drv_screen.width / 2;
    env_mouse.y = drv_screen.height / 2;
    env_mouse.local_x = 0;
    env_mouse.local_y = 0;
    env_mouse.hold_window = NULL;
    env_mouse.left_btn_down     = __left_button_down;
    env_mouse.left_btn_up       = __left_button_up;
    env_mouse.right_btn_down    = __right_button_down;
    env_mouse.right_btn_up      = __right_button_up;
    env_mouse.middle_btn_down   = __middle_button_down;
    env_mouse.middle_btn_up     = __middle_button_up;

    /* 顶层图层是鼠标图层 */
    layer_topest = create_layer(32, 32);
    if (layer_topest == NULL) {
        printf("create layer failed!\n");
        return -1;
    }
    printf("create layer: addr %x, buffer %x, width %d, height %d\n",
        layer_topest, layer_topest->buffer, layer_topest->width, layer_topest->height);
    memset(layer_topest->buffer, 0xdd, 32 * 32 * sizeof(GUI_COLOR));
    
    /* 绘制鼠标图层 */

    /* 设置鼠标图层高度 */
    layer_set_z(layer_topest, 0);
    
    /* 设置鼠标图层位置 */
    layer_set_xy(layer_topest, env_mouse.x, env_mouse.y);

    return 0;
}

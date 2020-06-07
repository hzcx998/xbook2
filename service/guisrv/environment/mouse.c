#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <drivers/screen.h>
#include <layer/draw.h>
#include <environment/mouse.h>

#include <layer/layer.h>
#include <window/window.h>
#include <widget/widget.h>

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
            
            gui_widget_mouse_button_down(&layer->widget_list_head, 0, env_mouse.x, env_mouse.y);

            printf("touch window: pos=%d-%d, size=%d-%d\n", 
                window->x, window->y, window->width, window->height);
                    
            if (!(window->attr & GUIW_NO_TITLE)) {
                 
                /* 有标题才判断标题栏 */
                if (local_my < GUIW_TITLE_HEIGHT) { /* in title */
                    /* 判断是否为控制按钮，如果是，就处理，不是就可能要移动窗口，切换窗口 */
                    printf("in window title\n");

                    /* 处理按钮 */

                    env_mouse.local_x = local_mx;
                    env_mouse.local_y = local_my;
                    
                    env_mouse.hold_window = window;

                    /* 如果不是活动窗口，就切换成为活动窗口 */
                    gui_window_switch(window);

                    mouse_enter_state(env_mouse);

                    break;
                } else {    /* in body */
                    /* 发送消息给窗口，如果不是激活窗口，就先激活 */
                    printf("in window body\n");
                    gui_window_switch(window);

                }
            } else {    /* 没有标题，就发送消息，聚焦窗口 */
                printf("no window title\n");

                /* 聚焦窗口 */
                gui_window_focus(window);
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

        mouse_enter_state(env_mouse);
        env_mouse_set_state(MOUSE_CURSOR_NORMAL);
    } else {    /* 没有抓住窗口，简单传输按键弹起消息 */
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
            local_mx = env_mouse.x - window->x;
            local_my = env_mouse.y - window->y;
            gui_widget_mouse_button_up(&layer->widget_list_head, 0, env_mouse.x, env_mouse.y);
            
            if (local_mx >= 0 && local_mx < window->width && 
                local_my >= 0 && local_my < window->height) {
                
                break;
            }
        }
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
            
            env_mouse_set_state(MOUSE_CURSOR_HOLD);

            layer_set_xy(env_mouse.hold_window->layer, env_mouse.x - 
                env_mouse.local_x, env_mouse.y - env_mouse.local_y);
        } else {
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
                local_mx = env_mouse.x - window->x;
                local_my = env_mouse.y - window->y;
                gui_widget_mouse_motion(&layer->widget_list_head, env_mouse.x, env_mouse.y);
                    
                if (local_mx >= 0 && local_mx < window->width && 
                    local_my >= 0 && local_my < window->height) {
                    if (current_window != window) { /* 鼠标切换到其它窗口 */
                        /* 改变窗口消息 */
                    }

                    /* 移动消息 */

                    break;
                }
            }
        }

        
    }
}

/* 需要对齐，不然数据就会出现混乱 */
char __attribute__((aligned(32)))__mouse_cursor_normal[MOUSE_CURSOR_WIDTH * MOUSE_CURSOR_HEIGHT] = {
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,2,2,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,1,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,1,0,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,0,0,0,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,0,0,0,0,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};
/* 需要对齐，不然数据就会出现混乱 */
char __attribute__((aligned(32)))__mouse_cursor_hold[MOUSE_CURSOR_WIDTH * MOUSE_CURSOR_HEIGHT] = {
    2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,1,1,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,2,2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,2,0,2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,2,0,0,0,2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,0,0,0,0,2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

/**
 * env_mouse_draw_buffer - 往缓冲区里面绘制鼠标
 */
static void env_mouse_draw_buffer(GUI_COLOR *buffer, char *data)
{
    int x, y;
	for (y = 0; y < MOUSE_CURSOR_HEIGHT; y++) {
		for (x = 0; x < MOUSE_CURSOR_WIDTH; x++) {
			if (data[y * MOUSE_CURSOR_WIDTH + x] == 0) {
                /* 透明色 */
                buffer[y * MOUSE_CURSOR_WIDTH + x] = COLOR_ARGB(0, 0, 0, 0);
            } else if (data[y * MOUSE_CURSOR_WIDTH + x] == 1) {
				/* 边框色 */
                buffer[y * MOUSE_CURSOR_WIDTH + x] = COLOR_WHITE;
            } else if (data[y * MOUSE_CURSOR_WIDTH + x] == 2) {
                /* 填充色 */
                buffer[y * MOUSE_CURSOR_WIDTH + x] = COLOR_BLACK;
            }
		}
	}
}

/**
 * 根据鼠标的状态绘制鼠标
 * 
 */
void env_mouse_draw(mouse_cursor_state_t state)
{
    switch (state)
    {
    case MOUSE_CURSOR_NORMAL:
        env_mouse_draw_buffer(env_mouse.layer->buffer, __mouse_cursor_normal);
        break;
    case MOUSE_CURSOR_HOLD:
        env_mouse_draw_buffer(env_mouse.layer->buffer, __mouse_cursor_hold);
        
        break;
    
    default:
        break;
    }
    /* 刷新鼠标图层 */
    layer_refresh(env_mouse.layer, 0, 0, env_mouse.layer->width, env_mouse.layer->height);
}

void env_mouse_set_state(mouse_cursor_state_t state)
{
    if (env_mouse.state_changed == false) {
        printf("[mouse] change state %d to %d\n", env_mouse.state, state);
        env_mouse_draw(state);
        env_mouse.state_changed = true;
    }
}

int init_env_mouse()
{
    env_mouse.state = MOUSE_CURSOR_NORMAL;
    env_mouse.state_changed = false;
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
    printf("[mouse] layer create.\n");
        
    /* 顶层图层是鼠标图层 */
    layer_topest = create_layer(MOUSE_CURSOR_WIDTH, MOUSE_CURSOR_HEIGHT);
    if (layer_topest == NULL) {
        printf("create layer failed!\n");
        return -1;
    }
    env_mouse.layer = layer_topest;

    printf("create layer: addr %x, buffer %x, width %d, height %d\n",
        layer_topest, layer_topest->buffer, layer_topest->width, layer_topest->height);
    //memset(layer_topest->buffer, 0xdd, 32 * 32 * sizeof(GUI_COLOR));
    //env_mouse_draw(env_mouse.state);
    env_mouse_draw_buffer(env_mouse.layer->buffer, __mouse_cursor_normal);
    /* 绘制鼠标图层 */

    /* 设置鼠标图层高度 */
    layer_set_z(layer_topest, 0);
    
    /* 设置鼠标图层位置 */
    layer_set_xy(layer_topest, env_mouse.x, env_mouse.y);

    return 0;
}

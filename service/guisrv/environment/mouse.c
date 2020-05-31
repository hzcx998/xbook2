#include <string.h>
#include <stdio.h>

#include <drivers/screen.h>
#include <layer/draw.h>
#include <environment/mouse.h>

#include <layer/layer.h>

env_mouse_t env_mouse = {0};


static void __left_button_down()
{
    printf("[mouse ] left button down.\n");
    printf("[mouse ] x:%d, y:%d\n", env_mouse.x, env_mouse.y);
}

static void __left_button_up()
{
    printf("[mouse ] left button up.\n");
    printf("[mouse ] x:%d, y:%d\n", env_mouse.x, env_mouse.y);
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
    }
}

int init_env_mouse()
{
    env_mouse.x = drv_screen.width / 2;
    env_mouse.y = drv_screen.height / 2;
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

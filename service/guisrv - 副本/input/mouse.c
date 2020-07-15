#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/input.h>

#include <drivers/screen.h>
#include <input/mouse.h>
#include <event/event.h>
#include <console/console.h>
#include <console/clipboard.h>
#include <graph/rect.h>
#include <graph/text.h>

#define DEBUG_LOCAL 0

input_mouse_t input_mouse = {0};

static void __button_down(int btn)
{
#if DEBUG_LOCAL == 1    
    printf("[mouse ] %d button down.\n", btn);
    printf("[mouse ] x:%d, y:%d\n", input_mouse.x, input_mouse.y);
#endif    
    input_mouse.show(input_mouse.x, input_mouse.y);

    gui_event e;
    e.type = GUI_EVENT_MOUSE_BUTTON;
    e.button.button = btn;
    e.button.state = GUI_PRESSED;
    e.button.x = input_mouse.x;
    e.button.y = input_mouse.y;
    gui_event_add(&e);
}

static void __button_up(int btn)
{
#if DEBUG_LOCAL == 1    
    printf("[mouse ] %d button up.\n", btn);
    printf("[mouse ] x:%d, y:%d\n", input_mouse.x, input_mouse.y);
#endif   
    input_mouse.show(input_mouse.x, input_mouse.y);

    gui_event e;
    e.type = GUI_EVENT_MOUSE_BUTTON;
    e.button.button = btn;
    e.button.state = GUI_RELEASED;
    e.button.x = input_mouse.x;
    e.button.y = input_mouse.y;
    gui_event_add(&e);
}

void __motion()
{
     /* 对鼠标进行修复 */
    if (input_mouse.x < 0)
        input_mouse.x = 0;
    if (input_mouse.y < 0)
        input_mouse.y = 0;
    /*
    if (input_mouse.x >= drv_screen.width)
        input_mouse.x = drv_screen.width - 1;
    if (input_mouse.y >= drv_screen.height)
        input_mouse.y = drv_screen.height - 1;
    */
    if (input_mouse.x >= screen.columns_width)
        input_mouse.x = screen.columns_width - 1;
    if (input_mouse.y >= screen.rows_height)
        input_mouse.y = screen.rows_height - 1;
    
    /* 移动鼠标 */
    input_mouse.show(input_mouse.x, input_mouse.y);

    gui_event e;
    e.type = GUI_EVENT_MOUSE_MOTION;
    e.button.state = GUI_NOSTATE;
    e.button.x = input_mouse.x;
    e.button.y = input_mouse.y;
    gui_event_add(&e);
}

void __show(int x, int y)
{
    /* 转换成格子对齐坐标 */
    
    int cx = x / screen.char_width;
    int cy = y / screen.char_height;
    int last_cx = input_mouse.last_x / screen.char_width;
    int last_cy = input_mouse.last_y / screen.char_height;
    /*if (cx == last_cx && cy == last_cy)
        return;*/
    
    int dx, dy;

    dx = input_mouse.last_x / screen.char_width * screen.char_width;
    dy = input_mouse.last_y / screen.char_height * screen.char_height;

    /* 绘制原来的位置 */
    char ch;
    con_get_char(&ch, last_cx, last_cy);

    GUI_COLOR bgcolor;
    GUI_COLOR fontcolor;

    /* 根据状态选择背景颜色 */
    
    /* 如果原来的是背景色 */
    if (input_mouse.old_color == drv_screen.gui_to_screen_color(screen.background_color)) {
        bgcolor = screen.background_color;
        fontcolor = screen.font_color;

    } else { /* 是选中颜色 */
        bgcolor = (0xffffff - (screen.background_color & 0xffffff)) | (0xff << 24);
        fontcolor = (0xffffff - (screen.font_color & 0xffffff)) | (0xff << 24);
    }
    
    /* 绘制背景 */
    draw_rect_fill(dx, dy, screen.char_width, screen.char_height, bgcolor);
    if (0x20 <= ch && ch <= 0x7e) {
        /* 绘制字符 */
        draw_word(dx, dy, ch, fontcolor);
    }
    
    dx = x / screen.char_width * screen.char_width;
    dy = y / screen.char_height * screen.char_height;
    
    /* 读取屏幕上的颜色 */
    SCREEN_COLOR scolor;
    drv_screen.input_pixel(dx, dy, &scolor);
    /* 纪录旧颜色 */
    input_mouse.old_color = scolor;

    /* 读取字符 */
    con_get_char(&ch, cx, cy);
    bgcolor = screen.mouse_color;
    /* 绘制背景 */
    draw_rect_fill(dx, dy, screen.char_width, screen.char_height, bgcolor);
    if (0x20 <= ch && ch <= 0x7e) {
        /* 绘制字符 */
        draw_word(dx, dy, ch, fontcolor);
    }

    /* 更新最新值 */
    input_mouse.last_x = x;
    input_mouse.last_y = y;
}

int init_mouse_input()
{
    input_mouse.x = drv_screen.width / 2;
    input_mouse.y = drv_screen.height / 2;
    input_mouse.last_x = input_mouse.x;
    input_mouse.last_y = input_mouse.y;
    input_mouse.button_down   = __button_down;
    input_mouse.button_up     = __button_up;
    input_mouse.motion     = __motion;
    input_mouse.show     = __show;
    return 0;
}

#include <string.h>
#include <stdio.h>
#include <xbook/driver.h>
#include <sys/ioctl.h>
#include <xbook/kmalloc.h>
#include <xbook/vmarea.h>

/// 程序本地头文件
#include <gui/mouse.h>
#include <gui/screen.h>
#include <gui/event.h>
#include <sys/input.h>

#include <gui/console/console.h>
#include <gui/text.h>
#include <gui/rect.h>
#include <gui/layer.h>
#include <gui/draw.h>

#define MOUSE_LAYER_W   32
#define MOUSE_LAYER_H   32

/* 鼠标图层 */
layer_t *mouse_layer = NULL;

void gui_mouse_show(int x, int y)
{
    /* 转换成格子对齐坐标 */
    
    int cx = x / gui_con_screen.char_width;
    int cy = y / gui_con_screen.char_height;
    int last_cx = gui_mouse.last_x / gui_con_screen.char_width;
    int last_cy = gui_mouse.last_y / gui_con_screen.char_height;
    /*if (cx == last_cx && cy == last_cy)
        return;*/
    
    int dx, dy;

    dx = gui_mouse.last_x / gui_con_screen.char_width * gui_con_screen.char_width;
    dy = gui_mouse.last_y / gui_con_screen.char_height * gui_con_screen.char_height;

    /* 绘制原来的位置 */
    char ch;
    con_get_char(&ch, last_cx, last_cy);

    GUI_COLOR bgcolor;
    GUI_COLOR fontcolor;

    /* 根据状态选择背景颜色 */
    
    /* 如果原来的是背景色 */
    if (gui_mouse.old_color == gui_screen.gui_to_screen_color(gui_con_screen.background_color)) {
        bgcolor = gui_con_screen.background_color;
        fontcolor = gui_con_screen.font_color;

    } else { /* 是选中颜色 */
        bgcolor = (0xffffff - (gui_con_screen.background_color & 0xffffff)) | (0xff << 24);
        fontcolor = (0xffffff - (gui_con_screen.font_color & 0xffffff)) | (0xff << 24);
    }
    
    /* 绘制背景 */
    gui_draw_rect_fill(dx, dy, gui_con_screen.char_width, gui_con_screen.char_height, bgcolor);
    if (0x20 <= ch && ch <= 0x7e) {
        /* 绘制字符 */
        gui_draw_word(dx, dy, ch, fontcolor);
    }
    
    dx = x / gui_con_screen.char_width * gui_con_screen.char_width;
    dy = y / gui_con_screen.char_height * gui_con_screen.char_height;
    
    /* 读取屏幕上的颜色 */
    SCREEN_COLOR scolor;
    gui_screen.input_pixel(dx, dy, &scolor);
    /* 纪录旧颜色 */
    gui_mouse.old_color = scolor;

    /* 读取字符 */
    con_get_char(&ch, cx, cy);
    bgcolor = gui_con_screen.mouse_color;
    /* 绘制背景 */
    gui_draw_rect_fill(dx, dy, gui_con_screen.char_width, gui_con_screen.char_height, bgcolor);
    if (0x20 <= ch && ch <= 0x7e) {
        /* 绘制字符 */
        gui_draw_word(dx, dy, ch, fontcolor);
    }

    /* 更新最新值 */
    gui_mouse.last_x = x;
    gui_mouse.last_y = y;

}

void gui_mouse_button_down(int btn)
{
#if DEBUG_LOCAL == 1    
    printf("[mouse ] %d button down.\n", btn);
    printf("[mouse ] x:%d, y:%d\n", gui_mouse.x, gui_mouse.y);
#endif    
    //gui_mouse.show(gui_mouse.x, gui_mouse.y);

    gui_event e;
    e.type = GUI_EVENT_MOUSE_BUTTON;
    e.button.button = btn;
    e.button.state = GUI_PRESSED;
    e.button.x = gui_mouse.x;
    e.button.y = gui_mouse.y;
    gui_event_add(&e);
  
}

void gui_mouse_button_up(int btn)
{
#if DEBUG_LOCAL == 1    
    printf("[mouse ] %d button up.\n", btn);
    printf("[mouse ] x:%d, y:%d\n", gui_mouse.x, gui_mouse.y);
#endif   
    //gui_mouse.show(gui_mouse.x, gui_mouse.y);

    gui_event e;
    e.type = GUI_EVENT_MOUSE_BUTTON;
    e.button.button = btn;
    e.button.state = GUI_RELEASED;
    e.button.x = gui_mouse.x;
    e.button.y = gui_mouse.y;
    gui_event_add(&e);
}

void gui_mouse_motion()
{
     /* 对鼠标进行修复 */
    if (gui_mouse.x < 0)
        gui_mouse.x = 0;
    if (gui_mouse.y < 0)
        gui_mouse.y = 0;
    if (gui_mouse.x >= gui_screen.width)
        gui_mouse.x = gui_screen.width - 1;
    if (gui_mouse.y >= gui_screen.height)
        gui_mouse.y = gui_screen.height - 1;
    
    layer_set_xy(mouse_layer, gui_mouse.x, gui_mouse.y);
    /*
    if (gui_mouse.x >= gui_con_screen.columns_width)
        gui_mouse.x = gui_con_screen.columns_width - 1;
    if (gui_mouse.y >= gui_con_screen.rows_height)
        gui_mouse.y = gui_con_screen.rows_height - 1;
    */
    /* 移动鼠标 */
    //gui_mouse.show(gui_mouse.x, gui_mouse.y);


    gui_event e;
    e.type = GUI_EVENT_MOUSE_MOTION;
    e.button.state = GUI_NOSTATE;
    e.button.x = gui_mouse.x;
    e.button.y = gui_mouse.y;
    gui_event_add(&e);
}

int init_mouse_layer()
{
    mouse_layer = create_layer(MOUSE_LAYER_W, MOUSE_LAYER_H);
    if (mouse_layer == NULL)
        return -1;
    layer_draw_rect(mouse_layer, 0, 0, mouse_layer->width, mouse_layer->height, COLOR_BLUE);
    layer_set_z(mouse_layer, 0);

    return 0;
}

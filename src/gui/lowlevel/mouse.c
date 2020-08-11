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

#ifndef   GUI_MOUSE_DEVICE_NAME
#define   GUI_MOUSE_DEVICE_NAME        "mouse"
#endif

gui_mouse_t gui_mouse = {0};

static  int   mouse_handle = 0;


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

static void gui_mouse_button_down(int btn)
{
#if DEBUG_LOCAL == 1    
    printf("[mouse ] %d button down.\n", btn);
    printf("[mouse ] x:%d, y:%d\n", gui_mouse.x, gui_mouse.y);
#endif    
    gui_mouse.show(gui_mouse.x, gui_mouse.y);

    gui_event e;
    e.type = GUI_EVENT_MOUSE_BUTTON;
    e.button.button = btn;
    e.button.state = GUI_PRESSED;
    e.button.x = gui_mouse.x;
    e.button.y = gui_mouse.y;
    gui_event_add(&e);
  
}

static void gui_mouse_button_up(int btn)
{
#if DEBUG_LOCAL == 1    
    printf("[mouse ] %d button up.\n", btn);
    printf("[mouse ] x:%d, y:%d\n", gui_mouse.x, gui_mouse.y);
#endif   
    gui_mouse.show(gui_mouse.x, gui_mouse.y);

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
    /*
    if (gui_mouse.x >= gui_screen.width)
        gui_mouse.x = gui_screen.width - 1;
    if (gui_mouse.y >= gui_screen.height)
        gui_mouse.y = gui_screen.height - 1;
    */

    if (gui_mouse.x >= gui_con_screen.columns_width)
        gui_mouse.x = gui_con_screen.columns_width - 1;
    if (gui_mouse.y >= gui_con_screen.rows_height)
        gui_mouse.y = gui_con_screen.rows_height - 1;

    /* 移动鼠标 */
    gui_mouse.show(gui_mouse.x, gui_mouse.y);

    gui_event e;
    e.type = GUI_EVENT_MOUSE_MOTION;
    e.button.state = GUI_NOSTATE;
    e.button.x = gui_mouse.x;
    e.button.y = gui_mouse.y;
    gui_event_add(&e);
}

static  int  mouse_open(void)
{
    mouse_handle = device_open( GUI_MOUSE_DEVICE_NAME, 0);
    if ( mouse_handle < 0 )
        return  -1;

    return  0;
}

static  int  mouse_close(void)
{
    return  device_close(mouse_handle);
}

static  int  mouse_read(void)
{
    static int  x_rel                     = 0;
    static int  y_rel                     = 0;
    static int  flag_rel                  = 0;

    struct      input_event  event;
    int         ret = 0;

read_mouse_continue:
    memset( &event, 0, sizeof(event));
    ret = device_read( mouse_handle, &event, sizeof(event), 0);
    if ( ret < 1 )
        return  -1;
    switch (event.type)
    {        
        case EV_REL:
            if ( (event.code) == REL_X )
            {
                x_rel    += event.value; 
                flag_rel  = 1;

                goto  read_mouse_continue;

            } else if ( (event.code) == REL_Y ) {
                y_rel    += event.value; 
                flag_rel  = 1;

                goto  read_mouse_continue;

            } else if ( (event.code) == REL_WHEEL ) {
                /* 一个滚轮事件 */
                return  0;           
            } else {
                /* 鼠标其它偏移事件 */
                return  0;
            }
            break;

        case EV_KEY:
            if ( (event.code) == BTN_LEFT )
            {
                /* 左键按下事件，需要传递鼠标位置 */
                
                if (event.value > 0) {
                    gui_mouse.button_down(0);
                } else {
                    gui_mouse.button_up(0);
                }
                return  0;
            } else if ( (event.code) == BTN_MIDDLE ) {
                /* 中键按下事件，需要传递鼠标位置 */
                if (event.value > 0) {
                    gui_mouse.button_down(1);
                } else {
                    gui_mouse.button_up(1);
                }

                return  0;
            } else if ( (event.code) == BTN_RIGHT ) {
                /* 右键按下事件，需要传递鼠标位置 */
                if (event.value > 0) {
                    gui_mouse.button_down(2);
                } else {
                    gui_mouse.button_up(2);
                }
                return  0;
            } else {
                /* 其它键按下事件，需要传递鼠标位置 */
                
                return  0;
            }
            break;

        case EV_MSC:
            /* 其它事件 */
            return  0;

        case EV_SYN:
            /* 同步事件，设置鼠标相对位置 */
            gui_mouse.x += x_rel;
            gui_mouse.y += y_rel;
            
            /* 相对位置置0 */
            x_rel = 0;
            y_rel = 0;

            if ( flag_rel == 1 )
            {
                gui_mouse.motion();
                flag_rel = 0;
                return  0;
            }
            flag_rel = 0;
            break;

        default:
            break;
    }

    return  0;
}

int gui_init_mouse()
{
    memset(&gui_mouse, 0, sizeof(gui_mouse));
    
    gui_mouse.open = mouse_open;
    gui_mouse.close = mouse_close;

    gui_mouse.read = mouse_read;
    gui_mouse.x = gui_screen.width / 2;
    gui_mouse.y = gui_screen.height / 2;
    gui_mouse.last_x = gui_mouse.x;
    gui_mouse.last_y = gui_mouse.y;
    gui_mouse.button_down   = gui_mouse_button_down;
    gui_mouse.button_up     = gui_mouse_button_up;
    gui_mouse.motion     = gui_mouse_motion;
    gui_mouse.show     = gui_mouse_show;
    return 0;
}

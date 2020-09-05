#include <string.h>
#include <stdio.h>
#include <xbook/driver.h>
#include <sys/ioctl.h>
#include <xbook/kmalloc.h>
#include <xbook/vmarea.h>

/// 程序本地头文件
#include <gui/mouse.h>
#include <gui/screen.h>
#include <sys/input.h>

#ifndef   GUI_MOUSE_DEVICE_NAME
#define   GUI_MOUSE_DEVICE_NAME        "mouse"
#endif

gui_mouse_t gui_mouse = {0};

static  int   mouse_handle = 0;


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
    return 0;
}

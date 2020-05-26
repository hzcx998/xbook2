#include <drivers/mouse.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>
#include <sys/input.h>
#include <string.h>
#include <stdio.h>
#include <guisrv.h>

#ifndef   GUI_MOUSE_DEVICE_NAME
#define   GUI_MOUSE_DEVICE_NAME        "mouse"
#endif

static  int   mouse_res = 0;


static  int  mouse_open(void)
{
    mouse_res = res_open( GUI_MOUSE_DEVICE_NAME, RES_DEV, 0);
    if ( mouse_res < 0 )
        return  -1;

    return  0;
}

static  int  mouse_close(void)
{
    return  res_close(mouse_res);
}

int mouse_x = 100;
int mouse_y = 100;


static  int  mouse_read(void)
{
    static int  x_rel                     = 0;
    static int  y_rel                     = 0;
    static int  flag_rel                  = 0;

    struct      input_event  event;
    int         ret = 0;

read_mouse_continue:
    memset( &event, 0, sizeof(event));
    ret = res_read( mouse_res, 0, &event, sizeof(event) );
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
                    printf("[mouse ] left button down.\n");
                } else {
                    printf("[mouse ] left button up.\n");
                }
                printf("[mouse ] x:%d, y:%d\n", mouse_x, mouse_y);

                return  0;
            } else if ( (event.code) == BTN_MIDDLE ) {
                /* 中键按下事件，需要传递鼠标位置 */
                if (event.value > 0) {
                    printf("[mouse ] middle button down.\n");
                } else {
                    printf("[mouse ] middle button up.\n");
                }
                printf("[mouse ] x:%d, y:%d\n", mouse_x, mouse_y);
                
                return  0;
            } else if ( (event.code) == BTN_RIGHT ) {
                /* 右键按下事件，需要传递鼠标位置 */
                if (event.value > 0) {
                    printf("[mouse ] right button down.\n");
                } else {
                    printf("[mouse ] right button up.\n");
                }
                printf("[mouse ] x:%d, y:%d\n", mouse_x, mouse_y);
                
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
            mouse_x += x_rel;
            mouse_y += y_rel;
            
            /* 相对位置置0 */
            x_rel = 0;
            y_rel = 0;

            if ( flag_rel == 1 )
            {
                /* 设定鼠标移动消息 */
                printf("[mouse ] x:%d, y:%d\n", mouse_x, mouse_y);

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

gui_mouse_t mouse = {0};

int init_mouse_driver()
{
    memset(&mouse, 0, sizeof(mouse));
    
    mouse.open = mouse_open;
    mouse.close = mouse_close;

    mouse.read = mouse_read;
    return 0;
}

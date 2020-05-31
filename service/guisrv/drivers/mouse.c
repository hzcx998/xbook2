#include <drivers/mouse.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>
#include <sys/input.h>
#include <string.h>
#include <stdio.h>
#include <guisrv.h>

#include <environment/mouse.h>

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
                    env_mouse.left_btn_down();
                } else {
                    env_mouse.left_btn_up();
                }
                return  0;
            } else if ( (event.code) == BTN_MIDDLE ) {
                /* 中键按下事件，需要传递鼠标位置 */
                if (event.value > 0) {
                    env_mouse.middle_btn_down();
                } else {
                    env_mouse.middle_btn_up();
                }

                return  0;
            } else if ( (event.code) == BTN_RIGHT ) {
                /* 右键按下事件，需要传递鼠标位置 */
                if (event.value > 0) {
                    env_mouse.right_btn_down();
                    
                } else {
                    env_mouse.right_btn_up();
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
            env_mouse.x += x_rel;
            env_mouse.y += y_rel;
            
            /* 相对位置置0 */
            x_rel = 0;
            y_rel = 0;

            if ( flag_rel == 1 )
            {
                env_mouse_move();
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

drv_mouse_t drv_mouse = {0};

int init_mouse_driver()
{
    memset(&drv_mouse, 0, sizeof(drv_mouse));
    
    drv_mouse.open = mouse_open;
    drv_mouse.close = mouse_close;

    drv_mouse.read = mouse_read;
    return 0;
}

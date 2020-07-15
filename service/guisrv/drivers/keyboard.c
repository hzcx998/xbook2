#include <string.h>
#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>
#include <sys/input.h>

/// 程序本地头文件
#include <guisrv.h>
#include <drivers/keyboard.h>
#include <input/keyboard.h>

#ifndef  GUI_KEYBOARD_DEVICE_NAME 
#define  GUI_KEYBOARD_DEVICE_NAME         "kbd"
#endif

static  int  kbd_res  = 0;

/*
static  unsigned char  caps_lock_value = 0;    
static  unsigned char  num_lock_value  = 0;
*/
drv_keyboard_t drv_keyboard = {0};

static  int  keyboard_open(void)
{
    kbd_res = res_open( GUI_KEYBOARD_DEVICE_NAME, RES_DEV, 0 );
    if ( kbd_res < 0 )
        return  -1;

    int ledstate;
    res_ioctl( kbd_res, EVENIO_GETLED,(unsigned long) &ledstate);

    if ( ledstate&0x01 )
        drv_keyboard.ledstate |= GUI_KMOD_NUM;

    if ( ledstate & 0x02 )
        drv_keyboard.ledstate |= GUI_KMOD_CAPS;

    return 0;
}

static  int  keyboard_close(void)
{
    return  res_close(kbd_res);
}

static  int  keyboard_read()
{
    struct  input_event  event;
    int     ret       = 0;

    memset(&event, 0, sizeof(event));
    ret = res_read(kbd_res, 0, &event, sizeof(event));
    if ( ret < 0 )
        return  -1;

    switch (event.type)
    {                
        case EV_KEY:         
            /* 图形服务先处理按键，然后再根据按键值传输给当前活动的窗口 */
            if ( (event.value) > 0 ) {  /* key presssed  */
                //printf("input key. %d\n", event.value);
                //res_write(1, 0, "key\n", 4);
                return input_keyboard.key_pressed(event.code);   
            } else {    /* key released  */
                return input_keyboard.key_released(event.code);
            }
        default:
            break;
    }

    return  -1;
}

int init_keyboard_driver()
{
    memset(&drv_keyboard, 0, sizeof(drv_keyboard));
    
    drv_keyboard.ledstate = 0;

    drv_keyboard.open = keyboard_open;
    drv_keyboard.close = keyboard_close;

    drv_keyboard.read = keyboard_read;

    return 0;
}

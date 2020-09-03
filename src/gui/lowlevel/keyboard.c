#include <string.h>
#include <stdio.h>
#include <xbook/driver.h>
#include <sys/ioctl.h>
#include <xbook/kmalloc.h>
#include <xbook/vmarea.h>

/// 程序本地头文件
#include <gui/keyboard.h>
#include <sys/input.h>

#ifndef  GUI_KEYBOARD_DEVICE_NAME 
#define  GUI_KEYBOARD_DEVICE_NAME         "kbd"
#endif

static  int  kbd_handle  = 0;

/*
static  unsigned char  caps_lock_value = 0;    
static  unsigned char  num_lock_value  = 0;
*/
gui_keyboard_t gui_keyboard = {0};


static  int  keyboard_open(void)
{
    kbd_handle = device_open( GUI_KEYBOARD_DEVICE_NAME, 0);
    if ( kbd_handle < 0 )
        return  -1;

    int ledstate;
    device_devctl( kbd_handle, EVENIO_GETLED,(unsigned long) &ledstate);

    if ( ledstate&0x01 )
        gui_keyboard.ledstate |= GUI_KMOD_NUM;

    if ( ledstate & 0x02 )
        gui_keyboard.ledstate |= GUI_KMOD_CAPS;

    return 0;
}

static  int  keyboard_close(void)
{
    return  device_close(kbd_handle);
}

static  int  keyboard_read()
{
    struct  input_event  event;
    int     ret       = 0;

    memset(&event, 0, sizeof(event));
    ret = device_read(kbd_handle, &event, sizeof(event), 0);
    if ( ret < 0 )
        return  -1;

    switch (event.type)
    {                
        case EV_KEY:         
            /* 图形服务先处理按键，然后再根据按键值传输给当前活动的窗口 */
            if ( (event.value) > 0 ) {  /* key presssed  */
                //printf("input key. %d\n", event.value);
                //res_write(1, 0, "key\n", 4);
                return gui_key_pressed(event.code);   
            } else {    /* key released  */
                return gui_key_released(event.code);
            }
        default:
            break;
    }

    return  -1;
}

int gui_init_keyboard()
{
    memset(&gui_keyboard, 0, sizeof(gui_keyboard));
    
    gui_keyboard.ledstate = 0;

    gui_keyboard.open = keyboard_open;
    gui_keyboard.close = keyboard_close;

    gui_keyboard.read = keyboard_read;

    return 0;
}

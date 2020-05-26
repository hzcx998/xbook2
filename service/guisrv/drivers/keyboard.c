#include <string.h>
#include <stdio.h>
#include <stdio.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>
#include <sys/input.h>

#include <drivers/keyboard.h>

#ifndef  GUI_KEYBOARD_DEVICE_NAME 
#define  GUI_KEYBOARD_DEVICE_NAME         "kbd"
#endif

static  int  kbd_res  = 0;
static  unsigned char  caps_lock_value = 0;    
static  unsigned char  num_lock_value  = 0;

static  int  keyboard_open(void)
{
    kbd_res = res_open( GUI_KEYBOARD_DEVICE_NAME, RES_DEV, 0 );
    if ( kbd_res < 0 )
        return  -1;

    int ledstate;
    res_ioctl( kbd_res, EVENIO_GETLED,(unsigned long) &ledstate);

    if ( ledstate&0x01 )
        num_lock_value = 1;
    else
        num_lock_value = 0;

    if ( ledstate&0x02 )
        caps_lock_value = 1;
    else
        caps_lock_value = 0;

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

            if ( (event.value) > 0 )
                printf("[keyboard ] key %x->%c down.\n", event.code, event.code);
            else
                printf("[keyboard ] key %x->%c up.\n", event.code, event.code);

            return  0;

        default:
            break;
    }

    return  -1;
}
gui_keyboard_t keyboard = {0};

int init_keyboard_driver()
{
    memset(&keyboard, 0, sizeof(keyboard));
    
    keyboard.open = keyboard_open;
    keyboard.close = keyboard_close;

    keyboard.read = keyboard_read;

    return 0;
}

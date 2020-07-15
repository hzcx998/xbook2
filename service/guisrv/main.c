#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>

#include <guisrv.h>
#include <console/console.h>
#include <drivers/screen.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <input/keyboard.h>
#include <input/mouse.h>
#include <font/font.h>
#include <event/event.h>

int init_guisrv()
{
    /* drivers */
    if (init_screen_driver()) {
        printf("[failed ] %s: init screen driver failed!\n", SRV_NAME);
        return -1;
    }
    
    if (init_keyboard_driver()) {
        printf("[failed ] %s: init keyboard driver failed!\n", SRV_NAME);
        return -1;
    }
    
    if (init_mouse_driver()) {
        printf("[failed ] %s: init mouse driver failed!\n", SRV_NAME);
        return -1;
    }
    
    if (init_keyboard_input()) {
        printf("[failed ] %s: init keyboard input failed!\n", SRV_NAME);
        return -1;
    }
    
    if (init_mouse_input()) {
        printf("[failed ] %s: init mouse input failed!\n", SRV_NAME);
        return -1;
    }

    if (init_event() < 0)
        return -1;

    gui_init_font();

    return 0;
}

int open_guisrv()
{
    /* drivers */
    if (drv_screen.open()) {
        printf("[failed ] %s: open screen driver failed!\n", SRV_NAME);
        return -1;
    }
    if (drv_keyboard.open()) {
        printf("[failed ] %s: open keyboard driver failed!\n", SRV_NAME);
        return -1;
    }
    if (drv_mouse.open()) {
        printf("[failed ] %s: open mouse driver failed!\n", SRV_NAME);
        return -1;
    }
    return 0;
}

int close_guisrv()
{
    /* drivers */
    if (drv_screen.close()) {
        printf("[failed ] %s: close screen driver failed!\n", SRV_NAME);
        return -1;
    }
    if (drv_keyboard.close()) {
        printf("[failed ] %s: close keyboard driver failed!\n", SRV_NAME);
        return -1;
    }
    if (drv_mouse.close()) {
        printf("[failed ] %s: close mouse driver failed!\n", SRV_NAME);
        return -1;
    }
    return 0;
}

void *gui_malloc(size_t size)
{
    return malloc(size);
}

void gui_free(void *ptr)
{
    free(ptr);
}

/*
GUISRV struct:
+-----------------------+
| interface             |
\                       /
+-----------------------+
|console | cmd | cursor |
\                       /
+-----------------------+
| graph | input | font  |
\                       /
+-----------------------+
| drivers               |
+-----------------------+
*/

int main(int argc, char *argv[])
{
    if (init_guisrv())
        return -1;

    if (open_guisrv())
        return -1;
    
    if (init_con_screen() < 0)
        return -1;
    
    con_loop();

    srvprint("exit service.\n");
    if (close_guisrv()) {
        return -1;
    }
    return 0;
}

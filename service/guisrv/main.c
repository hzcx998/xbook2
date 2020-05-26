#include <string.h>
#include <stdio.h>
#include <stdio.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>

#include <drivers/screen.h>
#include <drivers/mouse.h>
#include <drivers/keyboard.h>

#include <guisrv.h>

int init_guisrv()
{
    /* drivers */
    if (init_screen_driver()) {
        printf("[failed ] %s: init screen driver failed!\n", SRV_NAME);
        return -1;
    }
    if (init_mouse_driver()) {
        printf("[failed ] %s: init mouse driver failed!\n", SRV_NAME);
        return -1;
    }
    if (init_keyboard_driver()) {
        printf("[failed ] %s: init keyboard driver failed!\n", SRV_NAME);
        return -1;
    }

    return 0;
}

int open_guisrv()
{
    /* drivers */
    if (screen.open()) {
        printf("[failed ] %s: open screen driver failed!\n", SRV_NAME);
        return -1;
    }
    if (mouse.open()) {
        printf("[failed ] %s: open mouse driver failed!\n", SRV_NAME);
        return -1;
    }
    if (keyboard.open()) {
        printf("[failed ] %s: open keyboard driver failed!\n", SRV_NAME);
        return -1;
    }
    return 0;
}

int close_guisrv()
{
    /* drivers */
    if (screen.close()) {
        printf("[failed ] %s: close screen driver failed!\n", SRV_NAME);
        return -1;
    }
    if (mouse.close()) {
        printf("[failed ] %s: close mouse driver failed!\n", SRV_NAME);
        return -1;
    }
    if (keyboard.close()) {
        printf("[failed ] %s: close keyboard driver failed!\n", SRV_NAME);
        return -1;
    }
    return 0;
}

int loop_guisrv()
{
    while (1)
    {
        mouse.read();
        keyboard.read(); 
    }
}


/*
GUI struct:
+-----------------------+
| graph environment     |
| (system, user)        |
\                       /
+-----------------------+
| window manager        |
\                       /
+-----------------------+
| graph | input | event |
\                       /
+-----------------------+
| drivers               |
+-----------------------+
*/

int main(int argc, char *argv[])
{
    printf("[ok ] graph service start.\n");

    if (init_guisrv()) {
        return -1;
    }
    if (open_guisrv()) {
        return -1;
    }

    screen.output_rect_fill(0, 0, 100, 200, 0xffff0a);

    loop_guisrv();

    if (close_guisrv()) {
        return -1;
    }
    return 0;
}

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>

#include <drivers/screen.h>
#include <drivers/mouse.h>
#include <drivers/keyboard.h>

#include <layer/draw.h>
#include <layer/layer.h>

#include <window/window.h>
#include <environment/mouse.h>
#include <environment/interface.h>
#include <environment/statusbar.h>

#include <input/keyboard.h>

#include <event/msgque.h>

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
    
    if (init_msgque_event()) {
        return -1;
    }

    return 0;
}

int open_guisrv()
{
    /* drivers */
    if (drv_screen.open()) {
        printf("[failed ] %s: open screen driver failed!\n", SRV_NAME);
        return -1;
    }
    if (drv_mouse.open()) {
        printf("[failed ] %s: open mouse driver failed!\n", SRV_NAME);
        return -1;
    }
    if (drv_keyboard.open()) {
        printf("[failed ] %s: open keyboard driver failed!\n", SRV_NAME);
        return -1;
    }
    if (event_msgque.open()) {
        printf("[failed ] %s: open mesque event failed!\n", SRV_NAME);
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
    if (drv_mouse.close()) {
        printf("[failed ] %s: close mouse driver failed!\n", SRV_NAME);
        return -1;
    }
    if (drv_keyboard.close()) {
        printf("[failed ] %s: close keyboard driver failed!\n", SRV_NAME);
        return -1;
    }
    if (event_msgque.close()) {
        printf("[failed ] %s: close msgque event failed!\n", SRV_NAME);
        return -1;
    }

    return 0;
}

int start_guisrv()
{
    /* init layer */
    if (guisrv_init_layer()) {
        printf("[failed ] %s: init window layer failed!\n", SRV_NAME);
        return -1;
    }
    
    if (init_gui_window()) {
        printf("[failed ] %s: init window management failed!\n", SRV_NAME);
        return -1;
    }

    
    return 0;
}

int loop_guisrv()
{
    while (1)
    {
        drv_mouse.read();
        drv_keyboard.read(); 
        // 接收消息队列，根据消息队列内容处理消息
        event_msgque.read();
        statusbar_manager.read();
        
    }
}

void *gui_malloc(size_t size)
{
    //return malloc(size);

    void *p = (void *)sbrk(0);
    if (p == (void *) -1)
        return NULL;

    size = (size + 3) & (~3);
    if (sbrk(size) == (void *) -1)
        return NULL;
    return p;
}

void gui_free(void *ptr)
{
    /* invalid */
    free(ptr);
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
| layer | input | event |
\                       /
+-----------------------+
| drivers               |
+-----------------------+
*/

int main(int argc, char *argv[])
{
    printf("[ok ] graph service start.\n");

    if (init_guisrv())
        return -1;
    
    if (open_guisrv())
        return -1;
    
    if (start_guisrv() < 0)
        return -1;
    
    if (init_guisrv_interface() < 0) 
        return -1;

    init_env_display();

    loop_guisrv();

    if (close_guisrv()) {
        return -1;
    }
    return 0;
}

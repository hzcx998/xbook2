#include <string.h>
#include <stdio.h>
#include <stdio.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>

#include <drivers/screen.h>
#include <drivers/mouse.h>
#include <drivers/keyboard.h>

#include <graph/draw.h>

#include <window/layer.h>


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

int start_guisrv()
{
    
    /* init layer */
    if (guisrv_init_layer()) {
        printf("[failed ] %s: init window layer failed!\n", SRV_NAME);
        return -1;
    }
    
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

    graph_put_point(0,0,0xffff00);
    GUI_COLOR c;
    graph_get_point(0, 0, &c);
    printf("color: %x\n", c);

    graph_put_point(100,0,0x00ff00);
    graph_get_point(100, 0, &c);
    printf("color: %x\n", c);
    
    graph_put_point(1,50,0xff0000);
    graph_get_point(1,50, &c);
    printf("color: %x\n", c);
    
    graph_draw_line(100, 100, 103, 100, 0xff00);
    graph_draw_line(100, 100, 100, 103, 0xff00);
    graph_draw_line(100, 100, 105, 105, 0xff00ff);
    
    graph_draw_rect(200, 200, 3, 5, 0xff00);
    graph_fill_rect(200, 300, 5, 3, 0xffff);

    /* 10 * 5 大小的位图 */
    static GUI_COLOR bmbuf[10*15*4] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 
        0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000,
        0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 
        0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff,          
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 
        0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000,
        0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 
        0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff,          
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 
        0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000,
        0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 0xffff00, 
        0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff,  
    };
    /* 位图测试成功 */
    graph_draw_bitmap(200, 200, 10, 15, bmbuf);    

    start_guisrv();

    loop_guisrv();

    if (close_guisrv()) {
        return -1;
    }
    return 0;
}

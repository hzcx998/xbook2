#include <dwin/dwin.h>
#include <dwin/hal.h>

void dwin_thread(void *arg)
{
    dwin_log(DWIN_TAG"thread running\n");

    while (1)
    {
        dwin_hal->keyboard->read(&dwin_hal->keyboard->parent);
        dwin_hal->mouse->read(&dwin_hal->mouse->parent);
    }
}

void dwin_init(void)
{
    dwin_log(DWIN_TAG"init\n");

    /* init hal for device */
    if (dwin_hal_init() < 0)
    {    
        dwin_log(DWIN_TAG"init failed\n");
        return;
    }
    
    dwin_lcd_draw_rect(&dwin_hal->lcd->parent, 0, 0, 
        dwin_hal->lcd->parent.width,
        dwin_hal->lcd->parent.height,
        0xFFFF0000);
    
    dwin_lcd_draw_rect(&dwin_hal->lcd->parent, 100, 100, 
        dwin_hal->lcd->parent.width / 2,
        dwin_hal->lcd->parent.height / 2,
        0xFF00FF00);

    dwin_lcd_draw_rect(&dwin_hal->lcd->parent, 200, 200, 
        dwin_hal->lcd->parent.width / 4,
        dwin_hal->lcd->parent.height / 4,
        0xFF0000FF);

    /* init messagequeue */

    /* finally start a thread */
    if (dwin_hal->thread->start(&dwin_hal->thread->parent, dwin_thread, NULL) < 0)
    {
        dwin_hal_exit();
        dwin_log(DWIN_TAG"init failed\n");
        return;
    }
    dwin_log(DWIN_TAG"init success\n");
}
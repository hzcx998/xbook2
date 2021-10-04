#include <dwin/dwin.h>
#include <dwin/hal.h>
#include <dwin/workstation.h>
#include <dwin/layer.h>

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
    dwin_lcd_demo(&dwin_hal->lcd->parent);

    dwin_workstation_init(dwin_hal->lcd->parent.width, dwin_hal->lcd->parent.height);
    dwin_layer_init();


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
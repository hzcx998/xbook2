#include <dwin/dwin.h>
#include <dwin/hal.h>
#include <dwin/workstation.h>
#include <dwin/layer.h>
#include <dwin/objects.h>

static struct dwin_thread *main_thread;
static struct dwin_thread *desktop_thread;
static struct dwin_thread *misc_thread;

void dwin_thread(void *arg)
{
    dwin_log(DWIN_TAG"thread running\n");

    while (1)
    {
        dwin_hal->keyboard->read(&dwin_hal->keyboard->parent);
        dwin_hal->mouse->read(&dwin_hal->mouse->parent);
    }
    dwin_hal->thread->stop(main_thread, 0);
}

void dwin_desktop(void *arg)
{
    dwin_log("desktop thread running\n");
    dwin_layer_t *layer = dwin_workstation_get_lowest_layer(dwin_current_workstation);
    dwin_message_t msg;
    dwin_message_zero(&msg);
    while (1)
    {
        if (!dwin_layer_recv_message(layer, &msg, 0))
        {
            dwin_log("desktop recv msg:%d\n", msg.mid);
        }
    }
    dwin_hal->thread->stop(desktop_thread, 0);
}

void dwin_misc(void *arg)
{
    dwin_log("misc thread running\n");
    dwin_message_t msg;
    dwin_message_zero(&msg);
    while (1)
    {
        int lv;
        for (lv = 1; lv < DWIN_LAYER_PRIO_NR; lv++)
        {
            dwin_layer_t *layer = dwin_workstation_get_lowest_layer_priority(dwin_current_workstation, lv);
            if (layer == NULL)
            {
                continue;
            }
            if (!dwin_layer_recv_message(layer, &msg, DWIN_NOBLOCK))
            {
                dwin_log("misc layer#%d_%d recv msg:%d\n", layer->id, msg.lid, msg.mid);
            }
        }
    }
    dwin_hal->thread->stop(misc_thread, 0);
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
    if ((main_thread = dwin_hal->thread->start(dwin_thread, NULL)) == NULL)
    {
        dwin_hal_exit();
        dwin_log(DWIN_TAG"init failed\n");
        return;
    }
    if ((desktop_thread = dwin_hal->thread->start(dwin_desktop, NULL)) == NULL)
    {
        dwin_hal_exit();
        dwin_log(DWIN_TAG"init failed\n");
        return;
    }
    if ((misc_thread = dwin_hal->thread->start(dwin_misc, NULL)) == NULL)
    {
        dwin_hal_exit();
        dwin_log(DWIN_TAG"init failed\n");
        return;
    }
    dwin_log(DWIN_TAG"init success\n");
}

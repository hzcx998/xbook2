#include <dwin/objects.h>
#include <dwin/dwin.h>
#include <dwin/workstation.h>
#include <dwin/hal.h>

static void mouse_motion(struct dwin_mouse *mouse)
{
    int lv;
    int local_x, local_y;
    
    dwin_layer_t *layer;

    // dwin_log("mouse motion: %d,%d\n", mouse->x, mouse->y);
    dwin_layer_move(dwin_current_workstation->mouse_layer, mouse->x, mouse->y);

    /* 处理鼠标移动，根据鼠标位置来设置悬停图层。并将鼠标的消息发送到悬停图层。 */
    
    for (lv = DWIN_LAYER_PRIO_TOPEST; lv >= DWIN_LAYER_PRIO_DESKTOP; lv--)
    {
        list_for_each_owner_reverse(layer, &dwin_current_workstation->priority_list_head[lv], list)
        {
            if (layer->flags & DWIN_LAYER_FLAG_NOMSG)
            {
                continue;
            }
            
            local_x = mouse->x - layer->x;
            local_y = mouse->y - layer->y;

            /* no in layer, ignore */
            if (!(local_x >= 0 && local_x < layer->width && local_y >= 0 && local_y < layer->height))
            {
                continue;
            }

#if 0
            dwin_log("mouse motion: %d,%d layer#%d:%d, %d\n", mouse->x, mouse->y,
                    layer->id, local_x, local_y);
#endif

            /* check mouse hover layer */
            dwin_workstation_switch_hover_layer(dwin_current_workstation, layer);
            return;
        }
    }
}

static void mouse_wheel(struct dwin_mouse *mouse, int wheel)
{
    dwin_log("mouse wheel: %d\n", wheel);

    /* no hover layer, mouse motion first */
    if (dwin_current_workstation->hover_layer == NULL)
    {
        mouse_motion(mouse);
        if (dwin_current_workstation->hover_layer == NULL)
        {
            return;
        }
    }
    
    dwin_log(DWIN_TAG "mouse wheel on layer %d\n", dwin_current_workstation->hover_layer->id);
}

static void mouse_button_down(struct dwin_mouse *mouse, int button)
{
    dwin_log("mouse button: %d down\n", button);

    /* no hover layer, mouse motion first */
    if (dwin_current_workstation->hover_layer == NULL)
    {
        mouse_motion(mouse);
        if (dwin_current_workstation->hover_layer == NULL)
        {
            return;
        }
    }
    
    /* update focus layer as hover layer */
    dwin_workstation_switch_focus_layer(dwin_current_workstation, dwin_current_workstation->hover_layer);

    dwin_log(DWIN_TAG "mouse button down on layer %d\n", dwin_current_workstation->hover_layer->id);

}

static void mouse_button_up(struct dwin_mouse *mouse, int button)
{
    dwin_log("mouse button: %d up\n", button);
    
    /* no hover layer, mouse motion first */
    if (dwin_current_workstation->hover_layer == NULL)
    {
        mouse_motion(mouse);
        if (dwin_current_workstation->hover_layer == NULL)
        {
            return;
        }
    }
    
    dwin_log(DWIN_TAG "mouse button up on layer %d\n", dwin_current_workstation->hover_layer->id);
}

void dwin_mouse_init(struct dwin_mouse *mouse)
{
    mouse->handle = -1;
    mouse->motion = mouse_motion;
    mouse->button_down = mouse_button_down;
    mouse->button_up = mouse_button_up;
    mouse->wheel = mouse_wheel;
    mouse->x = 0;
    mouse->y = 0;
    
    mouse->local_x = mouse->local_y = 0;
    mouse->click_x = mouse->click_y = 0;
}

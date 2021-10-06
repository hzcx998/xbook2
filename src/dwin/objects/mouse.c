#include <dwin/objects.h>
#include <dwin/dwin.h>
#include <dwin/workstation.h>
#include <dwin/hal.h>

static void mouse_motion(struct dwin_mouse *mouse)
{
    int lv;
    int local_x, local_y;
    dwin_layer_t *layer;

    if (mouse->x < 0)
        mouse->x = 0;
    if (mouse->y < 0)
        mouse->y = 0;
    if (mouse->x > dwin_current_workstation->width - 1)
        mouse->x = dwin_current_workstation->width - 1;
    if (mouse->y > dwin_current_workstation->height - 1)
        mouse->y = dwin_current_workstation->height - 1;

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
            
            dwin_message_t msg;
            dwin_message_head(&msg, DWM_MOUSE_MOTION, layer->id);
            dwin_message_body(&msg, local_x, local_y, mouse->x, mouse->y);
            dwin_layer_send_message(layer, &msg, DWIN_NOBLOCK);

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
    
    dwin_layer_t *layer = dwin_current_workstation->hover_layer;

    dwin_log(DWIN_TAG "mouse wheel on layer %d\n", layer->id);
    
    int mid = (wheel == 0) ? DWM_MOUSE_WHEEL_UP : DWM_MOUSE_WHEEL_DOWN;
    dwin_message_t msg;
    dwin_message_head(&msg, mid, layer->id);
    dwin_message_body(&msg, mouse->x - layer->x, mouse->y - layer->y, mouse->x, mouse->y);
    dwin_layer_send_message(layer, &msg, DWIN_NOBLOCK);

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
    dwin_layer_t *layer = dwin_current_workstation->hover_layer;

    /* update focus layer as hover layer */
    dwin_workstation_switch_focus_layer(dwin_current_workstation, layer);

    dwin_log(DWIN_TAG "mouse button down on layer %d\n", layer->id);
    
    int mid = DWM_NONE;
    switch (button) {
    case 0:
        mid = DWM_MOUSE_LBTN_DOWN;
        break;
    case 1:
        mid = DWM_MOUSE_MBTN_DOWN;
        break;
    case 2:
        mid = DWM_MOUSE_RBTN_DOWN;
        break;
    default:
        break;
    }
    
    dwin_message_t msg;
    dwin_message_head(&msg, mid, layer->id);
    dwin_message_body(&msg, mouse->x - layer->x, mouse->y - layer->y, mouse->x, mouse->y);
    dwin_layer_send_message(layer, &msg, DWIN_NOBLOCK);
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
    dwin_layer_t *layer = dwin_current_workstation->hover_layer;

    dwin_log(DWIN_TAG "mouse button up on layer %d\n", layer->id);
    
    int mid = DWM_NONE;
    switch (button) {
    case 0:
        mid = DWM_MOUSE_LBTN_UP;
        break;
    case 1:
        mid = DWM_MOUSE_MBTN_UP;
        break;
    case 2:
        mid = DWM_MOUSE_RBTN_UP;
        break;
    default:
        break;
    }

    dwin_message_t msg;
    dwin_message_head(&msg, mid, layer->id);
    dwin_message_body(&msg, mouse->x - layer->x, mouse->y - layer->y, mouse->x, mouse->y);
    dwin_layer_send_message(layer, &msg, DWIN_NOBLOCK);
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

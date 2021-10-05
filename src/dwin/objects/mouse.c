#include <dwin/objects.h>
#include <dwin/dwin.h>
#include <dwin/workstation.h>
#include <dwin/hal.h>

static void mouse_motion(struct dwin_mouse *mouse)
{
    dwin_log("mouse motion: %d,%d\n", mouse->x, mouse->y);
    dwin_layer_move(dwin_current_workstation->mouse_layer, mouse->x, mouse->y);
}

static void mouse_wheel(struct dwin_mouse *mouse, int wheel)
{
    dwin_log("mouse wheel: %d\n", wheel);
}

static void mouse_button_down(struct dwin_mouse *mouse, int button)
{
    dwin_log("mouse button: %d down\n", button);
    
}

static void mouse_button_up(struct dwin_mouse *mouse, int button)
{
    dwin_log("mouse button: %d up\n", button);
    
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
    
    mouse->view_off_x = mouse->view_off_y = 0;
    mouse->local_x = mouse->local_y = 0;
    mouse->click_x = mouse->click_y = 0;
}

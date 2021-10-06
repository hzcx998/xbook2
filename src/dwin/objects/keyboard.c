#include <dwin/objects.h>
#include <dwin/dwin.h>
#include <dwin/workstation.h>

static int keyboard_key_down(struct dwin_keyboard *keyboard, int code)
{
    dwin_log("keyboard key: %d down\n", code);

    /* 处理修饰按键 */
    if (code == KEY_NUMLOCK) {
        /* 如果数字锁按键已经置位，那么就清除 */
        if (keyboard->key_modify & DWIN_KMOD_NUM) {
            keyboard->key_modify &= ~DWIN_KMOD_NUM;
        } else {    /* 没有则添加数字锁 */
            keyboard->key_modify |= DWIN_KMOD_NUM;    
        }
    }
    if (code == KEY_CAPSLOCK) {
        /* 如果大写锁按键已经置位，那么就清除 */
        if (keyboard->key_modify & DWIN_KMOD_CAPS) {
            keyboard->key_modify &= ~DWIN_KMOD_CAPS;
        } else {    /* 没有则添加数字锁 */
            keyboard->key_modify |= DWIN_KMOD_CAPS;    
        }
    }
    
    /* 处理CTRL, ALT, SHIFT*/
    switch (code) {
    case KEY_LSHIFT:    /* left shift */
        keyboard->key_modify |= DWIN_KMOD_SHIFT_L;
        break;
    case KEY_RSHIFT:    /* right shift */
        keyboard->key_modify |= DWIN_KMOD_SHIFT_R;
        break;
    case KEY_LALT:    /* left alt */
        keyboard->key_modify |= DWIN_KMOD_ALT_L;
        break;
    case KEY_RALT:    /* right alt */
        keyboard->key_modify |= DWIN_KMOD_ALT_R;
        break;
    case KEY_LCTRL:    /* left ctl */
        keyboard->key_modify |= DWIN_KMOD_CTRL_L;
        break;
    case KEY_RCTRL:    /* right ctl */
        keyboard->key_modify |= DWIN_KMOD_CTRL_R;
        break;
    default:
        break;
    }
    
    /* 如果是一些特殊按键，就做预处理 */

    if (dwin_current_workstation->focus_layer == NULL)
    {
        dwin_current_workstation->focus_layer = dwin_workstation_get_lowest_layer(dwin_current_workstation);        
        if (dwin_current_workstation->focus_layer == NULL)
        {
            return -1;
        }
    }
    dwin_layer_t *layer = dwin_current_workstation->focus_layer;

    /* send msg to focus layer */
    dwin_log(DWIN_TAG "keyboard down on layer %d\n", layer->id);

    dwin_message_t msg;
    dwin_message_head(&msg, DWM_KEY_DOWN, layer->id);
    dwin_message_body(&msg, code, keyboard->key_modify, 0, 0);
    dwin_layer_send_message(layer, &msg, DWIN_NOBLOCK);

    return 0;
}

static int keyboard_key_up(struct dwin_keyboard *keyboard, int code)
{
    dwin_log("keyboard key: %d up\n", code);
    
    /* special process */
    switch (code) {
    case KEY_LSHIFT:    /* left shift */
        keyboard->key_modify &= ~DWIN_KMOD_SHIFT_L;
        break;
    case KEY_RSHIFT:    /* right shift */
        keyboard->key_modify &= ~DWIN_KMOD_SHIFT_R;
        break;
    case KEY_LALT:    /* left alt */
        keyboard->key_modify &= ~DWIN_KMOD_ALT_L;
        break;
    case KEY_RALT:    /* right alt */
        keyboard->key_modify &= ~DWIN_KMOD_ALT_R;
        break;
    case KEY_LCTRL:    /* left ctl */
        keyboard->key_modify &= ~DWIN_KMOD_CTRL_L;
        break;
    case KEY_RCTRL:    /* right ctl */
        keyboard->key_modify &= ~DWIN_KMOD_CTRL_R;
        break;
    default:
        break;
    }

    /* 如果是一些特殊按键，就做预处理 */

    if (dwin_current_workstation->focus_layer == NULL)
    {
        dwin_current_workstation->focus_layer = dwin_workstation_get_lowest_layer(dwin_current_workstation);        
        if (dwin_current_workstation->focus_layer == NULL)
        {
            return -1;
        }
    }

    dwin_layer_t *layer = dwin_current_workstation->focus_layer;

    /* send msg to focus layer */
    dwin_log(DWIN_TAG "keyboard up on layer %d\n", layer->id);

    dwin_message_t msg;
    dwin_message_head(&msg, DWM_KEY_UP, layer->id);
    dwin_message_body(&msg, code, keyboard->key_modify, 0, 0);
    dwin_layer_send_message(layer, &msg, DWIN_NOBLOCK);

    return 0;
}

void dwin_keyboard_init(struct dwin_keyboard *keyboard)
{
    keyboard->handle = -1;
    keyboard->ledstate = 0;
    keyboard->key_modify = 0;
    keyboard->key_down = keyboard_key_down;
    keyboard->key_up = keyboard_key_up;
}

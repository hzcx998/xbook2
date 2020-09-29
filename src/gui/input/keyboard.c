#include <string.h>
#include <stdio.h>
#include <xbook/driver.h>
#include <sys/ioctl.h>
#include <xbook/kmalloc.h>
#include <xbook/vmarea.h>

/// 程序本地头文件
#include <gui/keyboard.h>
#include <sys/input.h>

#include <gui/message.h>
#include <gui/mouse.h>

/*
ALT+?
方向键，小移动

小键盘：
78946123大移动
5鼠标左键
enter鼠标右键
.(del)鼠标中键
+滚轮下
-滚轮上
*/
#define VIR_MOUSE_MOVE_BIG  50
#define VIR_MOUSE_MOVE_SMALL  12

/* 特殊按键：
ALT+TAB 切换窗口 */

/* 需要记录一些按键的状态，尤其是组合按键的时候就很有必要了。 */

int __process_special_key(int keycode, int press)
{

    if (keycode == KEY_E || keycode == KEY_e) {
        /* alt + tab */
        if (gui_keyboard.key_modify & GUI_KMOD_ALT_L) {
            if (press) {
#ifdef DEBUG_DRV                
                /* switch window */
                printf("[keyboard] [alt + tab] switch window.\n");
#endif
            }
            return 1;
        }
    }

    /* alt + ? */
    if (gui_keyboard.key_modify & GUI_KMOD_ALT_L) {
        if (press) {
            switch (keycode)
            {
            case KEY_LEFT:
                gui_mouse.x -= VIR_MOUSE_MOVE_SMALL;
                gui_mouse_motion();
                break;
            case KEY_RIGHT:
                gui_mouse.x += VIR_MOUSE_MOVE_SMALL;
                gui_mouse_motion();
                break;
            case KEY_UP:
                
                gui_mouse.y -= VIR_MOUSE_MOVE_SMALL;
                gui_mouse_motion();
                break;
            case KEY_DOWN:
                
                gui_mouse.y += VIR_MOUSE_MOVE_SMALL;
                gui_mouse_motion();
                break;
            case KEY_KP4:
                
                gui_mouse.x -= VIR_MOUSE_MOVE_BIG;
                gui_mouse_motion();
                break;
            case KEY_KP6:
                
                gui_mouse.x += VIR_MOUSE_MOVE_BIG;
                gui_mouse_motion();
                break;
            case KEY_KP8:
                
                gui_mouse.y -= VIR_MOUSE_MOVE_BIG;
                gui_mouse_motion();
                break;
            case KEY_KP2:
                
                gui_mouse.y += VIR_MOUSE_MOVE_BIG;
                gui_mouse_motion();
                break;
            case KEY_KP7:
                
                gui_mouse.x -= VIR_MOUSE_MOVE_BIG;
                gui_mouse.y -= VIR_MOUSE_MOVE_BIG;

                gui_mouse_motion();
                break;
            case KEY_KP9:
                
                gui_mouse.x += VIR_MOUSE_MOVE_BIG;
                gui_mouse.y -= VIR_MOUSE_MOVE_BIG;

                gui_mouse_motion();
                break;
            case KEY_KP1:
                gui_mouse.x -= VIR_MOUSE_MOVE_BIG;
                gui_mouse.y += VIR_MOUSE_MOVE_BIG;
                
                gui_mouse_motion();
                break;
            case KEY_KP3:
                gui_mouse.x += VIR_MOUSE_MOVE_BIG;
                gui_mouse.y += VIR_MOUSE_MOVE_BIG;
                gui_mouse_motion();
                break;
            case KEY_KP5:   /* left */
                gui_mouse_button_down(0);
                break;
            case KEY_KP_ENTER:   /* right */
            case KEY_ENTER:   /* right */
                gui_mouse_button_down(2);
                break;
            case KEY_KP_PERIOD:   /* middle */
                gui_mouse_button_down(1);
                break;
            case KEY_KP_PLUS:   /* wheel down */
                //gui_mouse_button_down(2);
                break;
            case KEY_KP_MINUS:   /* whell up */
                //gui_mouse_button_down(1);
                break;
            default:
                return 0;
            }
            return 1;
        } else {
            switch (keycode)
            {
            case KEY_KP5:   /* left */
                gui_mouse_button_up(0);
                break;
            case KEY_KP_ENTER:   /* right */
            case KEY_ENTER:   /* right */
                gui_mouse_button_up(2);
                break;
            case KEY_KP_PERIOD:   /* middle */
                gui_mouse_button_up(1);
                break;
            default:
                return 0;
            }
            return 1;
        }
    }
    return 0;
}

int gui_key_pressed(int keycode)
{
#ifdef DEBUG_DRV
    printf("[keyboard ] key %x->%c pressed.\n", keycode, keycode);
#endif
    /* 处理修饰按键 */
    if (keycode == KEY_NUMLOCK) {
        /* 如果数字锁按键已经置位，那么就清除 */
        if (gui_keyboard.key_modify & GUI_KMOD_NUM) {
            gui_keyboard.key_modify &= ~GUI_KMOD_NUM;
        } else {    /* 没有则添加数字锁 */
            gui_keyboard.key_modify |= GUI_KMOD_NUM;    
        }
    }
    if (keycode == KEY_CAPSLOCK) {
        /* 如果大写锁按键已经置位，那么就清除 */
        if (gui_keyboard.key_modify & GUI_KMOD_CAPS) {
            gui_keyboard.key_modify &= ~GUI_KMOD_CAPS;
        } else {    /* 没有则添加数字锁 */
            gui_keyboard.key_modify |= GUI_KMOD_CAPS;    
        }
    }
    
    /* 处理CTRL, ALT, SHIFT*/
    switch (keycode) {
    case KEY_LSHIFT:    /* left shift */
        gui_keyboard.key_modify |= GUI_KMOD_SHIFT_L;
        break;
    case KEY_RSHIFT:    /* right shift */
        gui_keyboard.key_modify |= GUI_KMOD_SHIFT_R;
        break;
    case KEY_LALT:    /* left alt */
        gui_keyboard.key_modify |= GUI_KMOD_ALT_L;
        break;
    case KEY_RALT:    /* right alt */
        gui_keyboard.key_modify |= GUI_KMOD_ALT_R;
        break;
    case KEY_LCTRL:    /* left ctl */
        gui_keyboard.key_modify |= GUI_KMOD_CTRL_L;
        break;
    case KEY_RCTRL:    /* right ctl */
        gui_keyboard.key_modify |= GUI_KMOD_CTRL_R;
        break;
    default:
        break;
    }
    
    /* 如果是一些特殊按键，就做预处理 */
    if (__process_special_key(keycode, 1))
        return -1;

    /* 使用按键,keydown, keycode, modify */
    /*gui_keyboard.keyevent.state = 0;
    gui_keyboard.keyevent.code = code_switch(keycode);
    gui_keyboard.keyevent.modify = gui_keyboard.key_modify;
    return 0;*/

    
    
    g_msg_t m;
    memset(&m, 0, sizeof(g_msg_t));
    m.id        = GM_KEY_DOWN;
    m.data0     = keycode;
    m.data1     = gui_keyboard.key_modify;
    return gui_push_msg(&m);

    /*
    gui_event e;
    e.type = GUI_EVENT_KEY;
    e.key.code = code_switch(keycode);
    e.key.modify = gui_keyboard.key_modify;
    e.key.state = GUI_PRESSED;
    return gui_event_add(&e);*/
}

int gui_key_released(int keycode)
{
#ifdef DEBUG_DRV
    printf("[keyboard ] key %x->%c released.\n", keycode, keycode);
#endif
    /* 处理CTRL, ALT, SHIFT*/
    switch (keycode) {
    case KEY_LSHIFT:    /* left shift */
        gui_keyboard.key_modify &= ~GUI_KMOD_SHIFT_L;
        break;
    case KEY_RSHIFT:    /* right shift */
        gui_keyboard.key_modify &= ~GUI_KMOD_SHIFT_R;
        break;
    case KEY_LALT:    /* left alt */
        gui_keyboard.key_modify &= ~GUI_KMOD_ALT_L;
        break;
    case KEY_RALT:    /* right alt */
        gui_keyboard.key_modify &= ~GUI_KMOD_ALT_R;
        break;
    case KEY_LCTRL:    /* left ctl */
        gui_keyboard.key_modify &= ~GUI_KMOD_CTRL_L;
        break;
    case KEY_RCTRL:    /* right ctl */
        gui_keyboard.key_modify &= ~GUI_KMOD_CTRL_R;
        break;
    default:
        break;
    }

    /* 如果是一些特殊按键，就做预处理 */
    if (__process_special_key(keycode, 0))
        return -1;

    g_msg_t m;
    memset(&m, 0, sizeof(g_msg_t));
    m.id        = GM_KEY_UP;
    m.data0     = keycode;
    m.data1     = gui_keyboard.key_modify;
    return gui_push_msg(&m);
}

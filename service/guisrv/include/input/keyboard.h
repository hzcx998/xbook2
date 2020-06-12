#ifndef __GUISRV_INPUT_KEYBOARD_H__
#define __GUISRV_INPUT_KEYBOARD_H__

#include <window/window.h>

#include <sgi/sgik.h>

#define GUI_KMOD_SHIFT_L    SGI_KMOD_LSHIFT
#define GUI_KMOD_SHIFT_R    SGI_KMOD_RSHIFT
#define GUI_KMOD_CTRL_L     SGI_KMOD_LCTRL
#define GUI_KMOD_CTRL_R     SGI_KMOD_RCTRL
#define GUI_KMOD_ALT_L      SGI_KMOD_LALT
#define GUI_KMOD_ALT_R      SGI_KMOD_RALT
#define GUI_KMOD_PAD	    SGI_KMOD_PAD
#define GUI_KMOD_NUM	    SGI_KMOD_NUM
#define GUI_KMOD_CAPS	    SGI_KMOD_CAPS

typedef struct _input_keyboard {
    int key_modify;                 /* 修饰按键 */
    int (*key_pressed) (int keycode);
    int (*key_released) (int keycode);
} input_keyboard_t;

extern input_keyboard_t input_keyboard;

int init_keyboard_input();

#endif  /* __GUISRV_INPUT_KEYBOARD_H__ */

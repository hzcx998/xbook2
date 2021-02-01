#ifndef _LIB_XTK_KEYBOARD_H
#define _LIB_XTK_KEYBOARD_H

#include "xtk_spirit.h"

int xtk_keyboard_key_down(xtk_spirit_t *spirit, int keycode, int modify);
int xtk_keyboard_key_up(xtk_spirit_t *spirit, int keycode, int modify);

#endif /* _LIB_XTK_KEYBOARD_H */
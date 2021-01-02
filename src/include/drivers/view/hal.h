#ifndef _XBOOK_DRIVERS_VIEW_HAL_H
#define _XBOOK_DRIVERS_VIEW_HAL_H

#include "drivers/view/screen.h"
#include "drivers/view/mouse.h"
#include "drivers/view/keyboard.h"
#include "drivers/view/section.h"

int view_screen_open(view_screen_t *screen);
int view_screen_close(view_screen_t *screen);

int view_mouse_open(view_mouse_t *mouse);
int view_mouse_close(view_mouse_t *mouse);
int view_mouse_read(view_mouse_t *mouse);

int view_keyboard_open(view_keyboard_t *keyboard);
int view_keyboard_close(view_keyboard_t *keyboard);

int view_section_open(view_section_t *section);
int view_section_close(view_section_t *section);

#endif /* _XBOOK_DRIVERS_VIEW_HAL_H */

#ifndef _DESKTOP_H
#define _DESKTOP_H

#include <stdint.h>

#define TASKBAR_HEIGHT 28

extern uint32_t desktop_width, desktop_height;

int init_desktop();
int init_taskbar();

#endif  /* _DESKTOP_H */
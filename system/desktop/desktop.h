#ifndef _DESKTOP_H
#define _DESKTOP_H

#include <stdint.h>

extern int desktop_layer;
extern uint32_t desktop_width, desktop_height;

int init_desktop();
int init_taskbar();

int jpg_load_bitmap(char * path, uint32_t *bitmap);
int png_load_bitmap(const char *filename, uint32_t *bitmap);

int desktop_launch_app(char *path);

#endif  /* _DESKTOP_H */
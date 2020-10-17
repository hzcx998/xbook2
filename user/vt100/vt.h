#ifndef _VT_H
#define _VT_H

#include "vt100.h"

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 300

extern struct vt100 *vt_term;
unsigned long vt100_test(unsigned long arg);
void test_colors();

#define get_cursor_x  (vt_term->cursor_x)
#define get_cursor_y  (vt_term->cursor_y)

#define in_line_end   (vt_term->cursor_x >= vt_term->width - 1)

void _puts(char *str);
void _putc(char c);

int init_console(int screen_width, int screen_height);

#endif /* _VT_H */
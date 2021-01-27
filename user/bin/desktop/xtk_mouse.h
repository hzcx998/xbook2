#ifndef _LIB_XTK_MOUSE_H
#define _LIB_XTK_MOUSE_H

#include "xtk_spirit.h"

int xtk_mouse_motion(xtk_spirit_t *spirit, int x, int y);
int xtk_mouse_lbtn_down(xtk_spirit_t *spirit, int x, int y);
int xtk_mouse_lbtn_up(xtk_spirit_t *spirit, int x, int y);
void xtk_mouse_load_cursors(int view, char *pathname);

#endif /* _LIB_XTK_MOUSE_H */
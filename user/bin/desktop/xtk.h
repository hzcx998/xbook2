#ifndef _LIB_XTK_H
#define _LIB_XTK_H

#include <stdint.h>

/* X tool kit
window
spirit
canvas
dialog
*/

#include "xtk_text.h"
#include "xtk_image.h"
#include "xtk_spirit.h"

// 更高级的抽象
#include "xtk_label.h"
#include "xtk_button.h"
#include "xtk_window.h"
#include "xtk_container.h"
#include "xtk_box.h"
#include "xtk_mouse.h"
#include "xtk_view.h"

void xtk_test(int fd, uview_bitmap_t *wbmp);
#if 0
void xtk_mouse_motion(int x, int y);
void xtk_mouse_lbtn_down(int x, int y);
void xtk_mouse_lbtn_up(int x, int y);
#endif
#endif /* _LIB_XTK_H */
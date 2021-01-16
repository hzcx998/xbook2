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
#include "xtk_window.h"

void xtk_test(int fd, uview_bitmap_t *wbmp);

#endif /* _LIB_XTK_H */
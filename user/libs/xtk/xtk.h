#ifndef _LIB_XTK_H
#define _LIB_XTK_H

#define _XTK_VER "0.1"

#include "xtk_color.h"
#include "xtk_text.h"
#include "xtk_image.h"
#include "xtk_spirit.h"
#include "xtk_signal.h"

// 更高级的抽象
#include "xtk_label.h"
#include "xtk_button.h"
#include "xtk_window.h"
#include "xtk_container.h"
#include "xtk_box.h"
#include "xtk_mouse.h"
#include "xtk_view.h"
#include "xtk_keyboard.h"

int xtk_init(int *argc, char **argv[]);
void xtk_exit(void);

int xtk_main();
int xtk_main_quit();

int xtk_poll();
int xtk_check_main_loop();

#endif /* _LIB_XTK_H */
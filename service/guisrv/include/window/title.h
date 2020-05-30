#ifndef __GUISRV_WINDOW_H__
#define __GUISRV_WINDOW_H__

#include "color.h"

#include <layer/layer.h>

#include <sys/list.h>

#define GUIW_TITLE_LEN    128   /* 标题字符串长度 */

#define GUIW_NO_TITLE       0x01    /* 没有标题 */
#define GUIW_BTN_MINIM      0x02    /* 最小化按钮 */
#define GUIW_BTN_MAXIM      0x04    /* 最大化按钮 */
#define GUIW_BTN_CLOSE      0x08    /* 关闭按钮 */

#define GUIW_BTN_MASK       (GUIW_BTN_MINIM | GUIW_BTN_MAXIM | GUIW_BTN_CLOSE)

#define GUIW_TITLE_HEIGHT    24     /* 窗口标题高度 */



typedef struct _gui_window {
    int x, y;               /* 窗口的位置 */
    int width, height;      /* 窗口的宽高 */
    int attr;               /* 窗口的属性 */
    char title[GUIW_TITLE_LEN]; /* 窗口标题 */
    layer_t *layer;         /* 窗口对应的图层 */
    list_t list;            /* 窗口链表 */
    list_t child_list;      /* 子窗口链表 */
} gui_window_t;



#endif  /* __GUISRV_WINDOW_H__ */

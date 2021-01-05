#ifndef _LIB_UVIEW_H
#define _LIB_UVIEW_H

#include <stdint.h>
#include "uview_bitmap.h"
#include "uview_msg.h"
#include "uview_shape.h"

/* 视图类型 */
enum uview_type {
    UVIEW_TYPE_FIXED      = 0,
    UVIEW_TYPE_WINDOW,
    UVIEW_TYPE_FLOAT,
};

enum uview_attr {
    UVIEW_ATTR_RESIZABLE     = 0x01,
    UVIEW_ATTR_MOVEABLE      = 0x02,
};

int uview_open(int width, int height);
int uview_close(int vfd);
int uview_set_pos(int vfd, int x, int y);
int uview_set_type(int vfd, int type);
int uview_set_wait(int vfd, int iswait);
int uview_show(int vfd);
int uview_hide(int vfd);
int uview_update(int vfd, int left, int top, int right, int bottom);
int uview_bitblt(int vfd, int vx, int vy, uview_bitmap_t *vbmp);
int uview_bitblt_update(int vfd, int vx, int vy, uview_bitmap_t *vbmp);
int uview_bitblt_ex(int vfd, int vx, int vy, uview_bitmap_t *vbmp, 
        int bx, int by, int bw, int bh);
int uview_bitblt_update_ex(int vfd, int vx, int vy, uview_bitmap_t *vbmp, 
        int bx, int by, int bw, int bh);
int uview_get_msg(int vfd, uview_msg_t *msg);
int uview_post_msg(int vfd, uview_msg_t *msg);

int uview_set_moveable(int vfd);
int uview_set_unmoveable(int vfd);
int uview_set_resizable(int vfd);
int uview_set_unresizable(int vfd);
int uview_resize(int vfd, int x, int y, int width, int height);
int uview_get_screensize(int vfd, int *width, int *height);

#endif  /* _LIB_UVIEW_H */
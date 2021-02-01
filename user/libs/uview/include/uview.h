#ifndef _LIB_UVIEW_H
#define _LIB_UVIEW_H

#include <stdint.h>
#include "uview_bitmap.h"
#include "uview_msg.h"
#include "uview_shape.h"
#include "uview_image.h"
#include "uview_mouse.h"

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


#define UVIEW_BAD_ID(view)  (view < 0)

int uview_open(int width, int height);
int uview_close(int vfd);
int uview_set_pos(int vfd, int x, int y);
int uview_set_type(int vfd, int type);
int uview_set_nowait(int vfd, int is_nowait);
int uview_set_size_min(int vfd, int width, int height);
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
int uview_send_msg(int vfd, uview_msg_t *msg);

int uview_set_moveable(int vfd);
int uview_set_unmoveable(int vfd);
int uview_set_resizable(int vfd);
int uview_set_unresizable(int vfd);
int uview_resize(int vfd, int width, int height);
int uview_get_screensize(int vfd, int *width, int *height);
int uview_get_lastpos(int vfd, int *x, int *y);
int uview_get_mousepos(int vfd, int *x, int *y);
int uview_get_pos(int vfd, int *x, int *y);
int uview_get_vid(int vfd, int *vid);
int uview_set_drag_region(int vfd, int left, int top, int right, int bottom);

typedef struct {
    unsigned long timer_id;
    unsigned long interval;
} uviewio_timer_t;

// 定时器间隔的最大值和最小值
#define UVIEW_TIMER_MINIMUM 0x0000000A
#define UVIEW_TIMER_MAXIMUM 0x7FFFFFFF

int uview_add_timer(int vfd, unsigned long interval);
int uview_del_timer(int vfd, unsigned long timer_id);
int uview_restart_timer(int vfd, unsigned long timer_id, unsigned long interval);

#endif  /* _LIB_UVIEW_H */
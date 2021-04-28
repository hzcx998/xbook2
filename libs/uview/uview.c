#include <uview.h>
#include <sys/udev.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "uview_io.h"

/**
 * 打开一个临时视图
 */
int uview_open_tmp(void)
{
    int flags = ((UVIEW_TYPE_FIXED & 0x1f) << 26) | ((32 & 0x1fff) << 12) | (32 & 0x1fff);
    return openclass("view", flags);
}

/**
 * 打开用户态视图
 */
int uview_open(int width, int height, int type)
{
    if (width <= 0 || height <= 0)
        return -1;
    uview_repair_size(&width, &height);
    int flags = ((type & 0x1f) << 26) | ((width & 0x1fff) << 12) | (height & 0x1fff);
    int vfd = openclass("view", flags);
    if (vfd < 0) {
        printf("uview: open view failed!\n");
        return -1;
    }
    return vfd;
}

int uview_close(int vfd)
{
    if (vfd < 0)
        return -1;
    return close(vfd);
}

int uview_set_pos(int vfd, int x, int y)
{
    if (vfd < 0)
        return -1;
    int pos = (x << 16) | y;
    return fastio(vfd, VIEWIO_SETPOS, &pos);
}

int uview_get_pos(int vfd, int *x, int *y)
{
    if (vfd < 0)
        return -1;
    int pos = 0;
    if (fastio(vfd, VIEWIO_GETPOS, &pos) < 0)
        return -1;
    if (x)
        *x = (pos >> 16) & 0xffff;
    if (y)
        *y = pos & 0xffff;
    return 0;
}

int uview_set_size_min(int vfd, int width, int height)
{
    if (vfd < 0)
        return -1;
    unsigned int sz = (width << 16) | height;
    return fastio(vfd, VIEWIO_SETSIZEMIN, &sz);
}

int uview_set_type(int vfd, int type)
{
    if (vfd < 0)
        return -1;
    return fastio(vfd, VIEWIO_SETTYPE, &type);
}

int uview_set_moveable(int vfd)
{
    if (vfd < 0)
        return -1;
    int attr = UVIEW_ATTR_MOVEABLE;
    return fastio(vfd, VIEWIO_ADDATTR, &attr);
}

int uview_set_unmoveable(int vfd)
{
    if (vfd < 0)
        return -1;
    int attr = UVIEW_ATTR_MOVEABLE;
    return fastio(vfd, VIEWIO_DELATTR, &attr);
}

int uview_set_resizable(int vfd)
{
    if (vfd < 0)
        return -1;
    int attr = UVIEW_ATTR_RESIZABLE;
    return fastio(vfd, VIEWIO_ADDATTR, &attr);
}

int uview_set_unresizable(int vfd)
{
    if (vfd < 0)
        return -1;
    int attr = UVIEW_ATTR_RESIZABLE;
    return fastio(vfd, VIEWIO_DELATTR, &attr);
}

int uview_set_nowait(int vfd, int is_nowait)
{
    if (vfd < 0)
        return -1;
    int vflags;
    if (fastio(vfd, VIEWIO_GETFLGS, &vflags) < 0)
        return -1;
    if (is_nowait)
        vflags |= DEV_NOWAIT;
    else
        vflags &= ~DEV_NOWAIT;
    return fastio(vfd, VIEWIO_SETFLGS, &vflags);
}

int uview_set_monitor(int vfd, int is_monitor)
{
    if (vfd < 0)
        return -1;
    return fastio(vfd, VIEWIO_SETMONITOR, &is_monitor);
}

int uview_show(int vfd)
{
    if (vfd < 0)
        return -1;
    return fastio(vfd, VIEWIO_SHOW, NULL);
}

int uview_get_screensize(int vfd, int *width, int *height)
{
    if (vfd < 0)
        return -1;
    uint32_t screensize;
    if (fastio(vfd, VIEWIO_GETSCREENSZ, &screensize) < 0)
        return -1;
    if (width)
        *width = (screensize >> 16)  & 0xffff;
    if (height)
        *height = screensize & 0xffff;
    return 0;
}

int uview_resize(int vfd, int width, int height)
{
    if (vfd < 0)
        return -1;
    unsigned int vsize = ((width & 0xffff ) << 16) | (height & 0xffff);
    return fastio(vfd, VIEWIO_RESIZE, &vsize);
}

int uview_hide(int vfd)
{
    if (vfd < 0)
        return -1;
    return fastio(vfd, VIEWIO_HIDE, 0);
}

int uview_update(int vfd, int left, int top, int right, int bottom)
{
    if (vfd < 0)
        return -1;
    uview_region_t region;
    region.left = left;
    region.top = top;
    region.right = right;
    region.bottom = bottom;
    return fastio(vfd, VIEWIO_REFRESH, &region);
}

int uview_set_drag_region(int vfd, int left, int top, int right, int bottom)
{
    if (vfd < 0)
        return -1;
    uview_region_t region;
    region.left = left;
    region.top = top;
    region.right = right;
    region.bottom = bottom;
    return fastio(vfd, VIEWIO_SETDRAGREGION, &region);
}

int uview_bitblt(int vfd, int vx, int vy, uview_bitmap_t *vbmp)
{
    if (vfd < 0)
        return -1;
    uview_io_t vio;
    memset(&vio, 0, sizeof(uview_io_t));
    vio.x = vx;
    vio.y = vy;
    vio.bmp.width = vbmp->width;
    vio.bmp.height = vbmp->height;
    vio.bmp.bits = vbmp->bits;
    vio.bx = 0;
    vio.by = 0;
    vio.bw = vbmp->width;
    vio.bh = vbmp->height;
    vio.refresh = 0; // no refresh
    return fastio(vfd, VIEWIO_WRBMP, &vio);
}

int uview_bitblt_update(int vfd, int vx, int vy, uview_bitmap_t *vbmp)
{
    if (vfd < 0)
        return -1;
    uview_io_t vio;
    memset(&vio, 0, sizeof(uview_io_t));
    vio.x = vx;
    vio.y = vy;
    vio.bmp.width = vbmp->width;
    vio.bmp.height = vbmp->height;
    vio.bmp.bits = vbmp->bits;
    vio.bx = 0;
    vio.by = 0;
    vio.bw = vbmp->width;
    vio.bh = vbmp->height;
    vio.refresh = 1; // with refresh
    return fastio(vfd, VIEWIO_WRBMP, &vio);
}

int uview_bitblt_ex(int vfd, int vx, int vy, uview_bitmap_t *vbmp, 
        int bx, int by, int bw, int bh)
{
    if (vfd < 0)
        return -1;
    uview_io_t vio;
    memset(&vio, 0, sizeof(uview_io_t));
    vio.x = vx;
    vio.y = vy;
    vio.bmp.width = vbmp->width;
    vio.bmp.height = vbmp->height;
    vio.bmp.bits = vbmp->bits;
    vio.bx = bx;
    vio.by = by;
    vio.bw = bw;
    vio.bh = bh;
    vio.refresh = 0; // no refresh
    return fastio(vfd, VIEWIO_WRBMP, &vio);
}

int uview_bitblt_update_ex(int vfd, int vx, int vy, uview_bitmap_t *vbmp, 
        int bx, int by, int bw, int bh)
{
    if (vfd < 0)
        return -1;
    uview_io_t vio;
    memset(&vio, 0, sizeof(uview_io_t));
    vio.x = vx;
    vio.y = vy;
    vio.bmp.width = vbmp->width;
    vio.bmp.height = vbmp->height;
    vio.bmp.bits = vbmp->bits;
    vio.bx = bx;
    vio.by = by;
    vio.bw = bw;
    vio.bh = bh;
    vio.refresh = 1; // with refresh
    return fastio(vfd, VIEWIO_WRBMP, &vio);
}

int uview_get_msg(int vfd, uview_msg_t *msg)
{
    if (vfd < 0)
        return -1;
    return fastread(vfd, msg, sizeof(uview_msg_t));
}

int uview_get_vid(int vfd, int *vid)
{
    if (vfd < 0)
        return -1;
    int _vid = -1;
    if (fastio(vfd, VIEWIO_GETVID, &_vid) < 0)
        return -1;
    if (vid)
        *vid = _vid;
    return 0;
}

int uview_send_msg(int vfd, uview_msg_t *msg)
{
    if (vfd < 0)
        return -1;
    return fastwrite(vfd, msg, sizeof(uview_msg_t));
}

int uview_get_lastpos(int vfd, int *x, int *y)
{
    if (vfd < 0)
        return -1;
    uint32_t lastpos;
    if (fastio(vfd, VIEWIO_GETLASTPOS, &lastpos) < 0)
        return -1;
    if (x)
        *x = (lastpos >> 16)  & 0xffff;
    if (y)
        *y = lastpos & 0xffff;
    return 0;
}

int uview_get_mousepos(int vfd, int *x, int *y)
{
    if (vfd < 0)
        return -1;
    uint32_t mousepos;
    if (fastio(vfd, VIEWIO_GETMOUSEPOS, &mousepos) < 0)
        return -1;
    if (x)
        *x = (mousepos >> 16)  & 0xffff;
    if (y)
        *y = mousepos & 0xffff;
    return 0;
}

int uview_add_timer(int vfd, unsigned long interval)
{
    if (vfd < 0)
        return -1;
    if (!interval)
        return -1;
    if (interval < UVIEW_TIMER_MINIMUM)
        interval = UVIEW_TIMER_MINIMUM;
    if (interval > UVIEW_TIMER_MAXIMUM)
        interval = UVIEW_TIMER_MAXIMUM;
    unsigned long timer_id_and_interval = interval;
    /* 输入时为定时器间隔，输出时为定时器id */
    if (fastio(vfd, VIEWIO_ADDTIMER, &timer_id_and_interval) < 0)
        return -1;
    return timer_id_and_interval;
}

int uview_del_timer(int vfd, unsigned long timer_id)
{
    if (vfd < 0)
        return -1;
    if (fastio(vfd, VIEWIO_DELTIMER, &timer_id) < 0)
        return -1;
    return 0;
}

int uview_restart_timer(int vfd, unsigned long timer_id, unsigned long interval)
{
    if (vfd < 0)
        return -1;
    if (interval < UVIEW_TIMER_MINIMUM)
        interval = UVIEW_TIMER_MINIMUM;
    if (interval > UVIEW_TIMER_MAXIMUM)
        interval = UVIEW_TIMER_MAXIMUM;
    uviewio_timer_t vio_timer;
    vio_timer.timer_id = timer_id;
    vio_timer.interval = interval;
    if (fastio(vfd, VIEWIO_RESTARTTIMER, &vio_timer) < 0)
        return -1;
    return 0;
}

void uview_repair_size(int *width, int *height)
{
    if (*width > UVIEW_MAX_SIZE_WIDTH) {
        printf("uview: width repair from %d to %d.\n", *width, UVIEW_MAX_SIZE_WIDTH);
        *width = UVIEW_MAX_SIZE_WIDTH;
    }
    if (*height > UVIEW_MAX_SIZE_HEIGHT) {
        printf("uview: height repair from %d to %d.\n", *height, UVIEW_MAX_SIZE_HEIGHT);
        *height = UVIEW_MAX_SIZE_HEIGHT;
    }
}

int uview_set_win_maxim_rect(int vfd, int x, int y, int width, int height)
{
    if (vfd < 0)
        return -1;
    if (width < 0 || height < 0)
        return -1;
    uview_rect_t rect;
    uview_rect_init(&rect, x, y, width, height);
    if (fastio(vfd, VIEWIO_SETWINMAXIMRECT, &rect) < 0)
        return -1;
    return 0;
}

int uview_get_win_maxim_rect(int vfd, int *x, int *y, int *width, int *height)
{
    if (vfd < 0)
        return -1;
    uview_rect_t rect;
    if (fastio(vfd, VIEWIO_GETWINMAXIMRECT, &rect) < 0)
        return -1;
    if (x)
        *x = rect.x;
    if (y)
        *y = rect.y;
    if (width)
        *width = rect.w.sw;
    if (height)
        *height = rect.h.sh;
    return 0;
}
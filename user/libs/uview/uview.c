#include <uview.h>
#include <sys/udev.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

/**
 * 打开用户态视图
 */
int uview_open(int width, int height)
{
    if (width <= 0 || height <= 0)
        return -1;
    int flags = (width << 16) | height;
    char buf[12] = {0};
    if (probedev("view", buf, 12) < 0) {
        printf("uview: no free view existed!\n");
        return -1;
    }
    int vfd = opendev(buf, flags);
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
    return ioctl(vfd, VIEWIO_SETPOS, &pos);
}

int uview_set_type(int vfd, int type)
{
    if (vfd < 0)
        return -1;
    return ioctl(vfd, VIEWIO_SETTYPE, &type);
}

int uview_set_wait(int vfd, int iswait)
{
    if (vfd < 0)
        return -1;
    int vflags;
    if (ioctl(vfd, VIEWIO_GETFLGS, &vflags) < 0)
        return -1;
    if (iswait)
        vflags |= DEV_NOWAIT;
    else
        vflags &= ~DEV_NOWAIT;
    return ioctl(vfd, VIEWIO_SETFLGS, &vflags);
}

int uview_show(int vfd)
{
    if (vfd < 0)
        return -1;
    return ioctl(vfd, VIEWIO_SHOW, 0);
}

int uview_hide(int vfd)
{
    if (vfd < 0)
        return -1;
    return ioctl(vfd, VIEWIO_HIDE, 0);
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
    return ioctl(vfd, VIEWIO_REFRESH, &region);
}

int uview_bitblt(int vfd, int vx, int vy, uview_bitmap_t *vbmp)
{
    if (vfd < 0)
        return -1;
    uview_io_t vio;
    memset(&vio, 0, sizeof(uview_io_t));
    vio.x = vx;
    vio.x = vy;
    vio.bmp.width = vbmp->width;
    vio.bmp.height = vbmp->height;
    vio.bmp.bits = vbmp->bits;
    vio.bx = 0;
    vio.by = 0;
    vio.bw = vbmp->width;
    vio.bh = vbmp->height;
    vio.refresh = 0; // no refresh
    return ioctl(vfd, VIEWIO_WRBMP, &vio);
}

int uview_bitblt_update(int vfd, int vx, int vy, uview_bitmap_t *vbmp)
{
    if (vfd < 0)
        return -1;
    uview_io_t vio;
    memset(&vio, 0, sizeof(uview_io_t));
    vio.x = vx;
    vio.x = vy;
    vio.bmp.width = vbmp->width;
    vio.bmp.height = vbmp->height;
    vio.bmp.bits = vbmp->bits;
    vio.bx = 0;
    vio.by = 0;
    vio.bw = vbmp->width;
    vio.bh = vbmp->height;
    vio.refresh = 1; // with refresh
    return ioctl(vfd, VIEWIO_WRBMP, &vio);
}

int uview_bitblt_ex(int vfd, int vx, int vy, uview_bitmap_t *vbmp, 
        int bx, int by, int bw, int bh)
{
    if (vfd < 0)
        return -1;
    uview_io_t vio;
    memset(&vio, 0, sizeof(uview_io_t));
    vio.x = vx;
    vio.x = vy;
    vio.bmp.width = vbmp->width;
    vio.bmp.height = vbmp->height;
    vio.bmp.bits = vbmp->bits;
    vio.bx = bx;
    vio.by = by;
    vio.bw = bw;
    vio.bh = bh;
    vio.refresh = 0; // no refresh
    return ioctl(vfd, VIEWIO_WRBMP, &vio);
}

int uview_bitblt_update_ex(int vfd, int vx, int vy, uview_bitmap_t *vbmp, 
        int bx, int by, int bw, int bh)
{
    if (vfd < 0)
        return -1;
    uview_io_t vio;
    memset(&vio, 0, sizeof(uview_io_t));
    vio.x = vx;
    vio.x = vy;
    vio.bmp.width = vbmp->width;
    vio.bmp.height = vbmp->height;
    vio.bmp.bits = vbmp->bits;
    vio.bx = bx;
    vio.by = by;
    vio.bw = bw;
    vio.bh = bh;
    vio.refresh = 1; // with refresh
    return ioctl(vfd, VIEWIO_WRBMP, &vio);
}

int uview_get_msg(int vfd, uview_msg_t *msg)
{
    if (vfd < 0)
        return -1;
    return read(vfd, msg, sizeof(uview_msg_t));
}

int uview_post_msg(int vfd, uview_msg_t *msg)
{
    if (vfd < 0)
        return -1;
    return write(vfd, msg, sizeof(uview_msg_t));
}

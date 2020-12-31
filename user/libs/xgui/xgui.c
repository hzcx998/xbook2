#include <xgui.h>
#include <gui.client.h>
#include <sys/lpc.h>
#include <sys/ipc.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define XGUI_CLIENT_MAX_VIEW_NR 8

typedef struct {
    int width;
    int height;
    int handle;
    void *mapaddr;
} xgui_client_view_t;

static xgui_client_view_t xgui_view_table[XGUI_CLIENT_MAX_VIEW_NR] = {{0,0,0,NULL},};

#define __VIEW_TO_HANDLE(view) ((view) - xgui_view_table)
#define __HANDLE_TO_VIEW(handle) (xgui_view_table + (handle))
#define __ASSERT_VIEW_HANDLE(handle) (assert((handle) >= 0 && (handle) < XGUI_CLIENT_MAX_VIEW_NR))

static xgui_client_view_t *get_client_view()
{
    xgui_client_view_t *view;
    int i; for (i = 0; i < XGUI_CLIENT_MAX_VIEW_NR; i++) {
        view = &xgui_view_table[i];
        if (view->mapaddr == NULL) {
            view->width = 0;
            view->height = 0;
            view->handle = 0;
            return view;
        }
    }
    return NULL;
}

static int put_client_view(xgui_client_view_t *view)
{
    xgui_client_view_t *view_;
    int i; for (i = 0; i < XGUI_CLIENT_MAX_VIEW_NR; i++) {
        view_ = &xgui_view_table[i];
        if (view_ == view) {
            view_->mapaddr = NULL;
            return 0;
        }
    }
    return -1;
}

int xgui_create_view(int x, int y, int width, int height)
{
    xgui_client_view_t *view = get_client_view();
    if (!view)
        return -1;
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, x);
    lpc_parcel_write_int(parcel, y);
    lpc_parcel_write_int(parcel, width);
    lpc_parcel_write_int(parcel, height);
    if (lpc_call(LPC_ID_GRAPH, GUICALL_create_view, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int view_handle = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&view_handle);
    if (view_handle < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    uint32_t shmid;
    lpc_parcel_read_int(parcel, (uint32_t *)&shmid);
    void *addr = shmmap(shmid, NULL, IPC_RND);
    if (addr == (void *)-1) {
        xgui_destroy_view(view_handle);
        lpc_parcel_put(parcel);
        return -1;
    }
    // record addr
    view->width = width;
    view->height = height;
    view->mapaddr = addr;
    view->handle = view_handle;
    lpc_parcel_put(parcel);
    return __VIEW_TO_HANDLE(view);
}

int xgui_show_view(int handle)
{
    __ASSERT_VIEW_HANDLE(handle);
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    xgui_client_view_t *view = __HANDLE_TO_VIEW(handle);
    lpc_parcel_write_int(parcel, view->handle);
    if (lpc_call(LPC_ID_GRAPH, GUICALL_show_view, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    return retval;
}

int xgui_destroy_view(int handle)
{
    __ASSERT_VIEW_HANDLE(handle);
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    xgui_client_view_t *view = __HANDLE_TO_VIEW(handle);
    if (view->mapaddr)
        shmunmap(view->mapaddr, IPC_RND);
    view->mapaddr = NULL;
    lpc_parcel_write_int(parcel, view->handle);
    if (lpc_call(LPC_ID_GRAPH, GUICALL_destroy_view, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    put_client_view(view);
    return retval;
}

int xgui_move_view(int handle, int x, int y)
{
    __ASSERT_VIEW_HANDLE(handle);
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    xgui_client_view_t *view = __HANDLE_TO_VIEW(handle);
    lpc_parcel_write_int(parcel, view->handle);
    lpc_parcel_write_int(parcel, x);
    lpc_parcel_write_int(parcel, y);
    if (lpc_call(LPC_ID_GRAPH, GUICALL_move_view, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    return retval;
}

int xgui_hide_view(int handle)
{
    __ASSERT_VIEW_HANDLE(handle);
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    xgui_client_view_t *view = __HANDLE_TO_VIEW(handle);
    lpc_parcel_write_int(parcel, view->handle);
    if (lpc_call(LPC_ID_GRAPH, GUICALL_hide_view, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    return retval;
}

int xgui_update_view(int handle, int left, int top, int right, int buttom)
{
    __ASSERT_VIEW_HANDLE(handle);
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    xgui_client_view_t *view = __HANDLE_TO_VIEW(handle);
    lpc_parcel_write_int(parcel, view->handle);
    lpc_parcel_write_int(parcel, left);
    lpc_parcel_write_int(parcel, top);
    lpc_parcel_write_int(parcel, right);
    lpc_parcel_write_int(parcel, buttom);
    if (lpc_call(LPC_ID_GRAPH, GUICALL_update_view, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    return retval;
}

int __xgui_render_putpixel(xgui_client_view_t *view, int x, int y, xgui_color_t color)
{
    if (x < 0 || x >= view->width || y < 0 || y >= view->height) {
        return -1;
    }
    xgui_color_t *buf = (xgui_color_t *)view->mapaddr;
    buf[y * view->width + x] = color;
    return 0;
}

int __xgui_render_getpixel(xgui_client_view_t *view, int x, int y, xgui_color_t *color)
{
    if (x < 0 || x >= view->width || y < 0 || y >= view->height) {
        return -1;
    }
    xgui_color_t *buf = (xgui_color_t *)view->mapaddr;
    *color = buf[y * view->width + x];
    return 0;
}

int xgui_render_putpixel(int handle, int x, int y, xgui_color_t color)
{
    __ASSERT_VIEW_HANDLE(handle);
    xgui_client_view_t *view = __HANDLE_TO_VIEW(handle);
    if (x < 0 || x >= view->width || y < 0 || y >= view->height) {
        return -1;
    }
    xgui_color_t *buf = (xgui_color_t *)view->mapaddr;
    buf[y * view->width + x] = color;
    return 0;
}

int xgui_render_getpixel(int handle, int x, int y, xgui_color_t *color)
{
    __ASSERT_VIEW_HANDLE(handle);
    xgui_client_view_t *view = __HANDLE_TO_VIEW(handle);
    if (x < 0 || x >= view->width || y < 0 || y >= view->height) {
        return -1;
    }
    xgui_color_t *buf = (xgui_color_t *)view->mapaddr;
    *color = buf[y * view->width + x];
    return 0;
}

int xgui_render_bitblt(int handle, int x, int y, 
        xgui_bitmap_t *bmp, xgui_rect_t *rect)
{
    if (!bmp)
        return -1;
    __ASSERT_VIEW_HANDLE(handle);
    xgui_rect_t rect_;
    if (rect == NULL) {
        rect_.left = 0;
        rect_.top = 0;
        rect_.right = bmp->width;
        rect_.bottom = bmp->height;
        rect = &rect_;
    }
    xgui_client_view_t *view = __HANDLE_TO_VIEW(handle);
    int w = min((rect->right - rect->left), bmp->width - rect->left);
    int h = min((rect->bottom - rect->top), bmp->height - rect->top);
    if (w <= 0 || h <= 0)
        return -1;
    xgui_color_t color;
    int i, j;
    int vx, vy;
    for (j = 0; j < h; j++) {
        vy = y + j;
        if (vy >= view->height)
            break;
        for (i = 0; i < w; i++) {
            vx = x + i;
            if (vx >= view->width)
                break;
            xgui_bitmap_getpixel(bmp, rect->left + i, rect->top + j, &color);
            if (((color >> 24) & 0xff))
                __xgui_render_putpixel(view, vx, vy, color);
        }
    }
    return 0;
}

int xgui_render_bitblt_reverse(int handle, int x, int y, 
        xgui_bitmap_t *bmp, xgui_rect_t *rect)
{
    if (!bmp)
        return -1;
    __ASSERT_VIEW_HANDLE(handle);
    xgui_rect_t rect_;
    if (rect == NULL) {
        rect_.left = 0;
        rect_.top = 0;
        rect_.right = bmp->width;
        rect_.bottom = bmp->height;
        rect = &rect_;
    }
    xgui_client_view_t *view = __HANDLE_TO_VIEW(handle);
    int w = min((rect->right - rect->left), bmp->width - rect->left);
    int h = min((rect->bottom - rect->top), bmp->height - rect->top);
    if (w <= 0 || h <= 0)
        return -1;
    xgui_color_t color;
    int i, j;
    int vx, vy;
    for (j = 0; j < h; j++) {
        vy = y + j;
        if (vy >= view->height)
            break;
        for (i = 0; i < w; i++) {
            vx = x + i;
            if (vx >= view->width)
                break;
            color = 0;
            __xgui_render_getpixel(view, vx, vy, &color);
            xgui_bitmap_putpixel(bmp, rect->left + i, rect->top + j, color);
        }
    }
    return 0;
}


void xgui_render_clear(int handle)
{
    __ASSERT_VIEW_HANDLE(handle);
    xgui_client_view_t *view = __HANDLE_TO_VIEW(handle);
    xgui_color_t *buf = (xgui_color_t *)view->mapaddr;
    int i, j;
    for (j = 0; j < view->height; j++) {
        for (i = 0; i < view->width; i++) {
            buf[j * view->width + i] = 0;
        }
    }
}

void xgui_render_vline(int handle, int x, int y1, int y2, xgui_color_t color)
{
    __ASSERT_VIEW_HANDLE(handle);
    xgui_client_view_t *view = __HANDLE_TO_VIEW(handle);
    xgui_color_t *buf = (xgui_color_t *)view->mapaddr;
    int offset = 0;
    int i = 0;

    if (x > (view->width - 1))
        return;
    if (y1 > (view->height - 1))
        return;
    if (y2 > (view->height - 1))
        return;

    for (i = 0; i <= y2 - y1; i++)
    {
        offset = (view->width * (y1 + i) + x);
        if (offset >= (view->width * view->height - 1))
            return;
        *(buf + offset) = color;
    }
}

void xgui_render_hline(int handle, int x1, int y, int x2, xgui_color_t color)
{
    __ASSERT_VIEW_HANDLE(handle);
    xgui_client_view_t *view = __HANDLE_TO_VIEW(handle);
    xgui_color_t *buf = (xgui_color_t *)view->mapaddr;

    int offset = 0;
    int i = 0;
    
    if (x1 > (view->width - 1))
        return;
    if (x2 > (view->width - 1))
        return;
    if (y > (view->height - 1))
        return;

    offset = ((view->width) * y + x1);
    if (offset >= (view->width * view->height - 1))
        return;
    for (i = 0; i <= x2 - x1; i++ )
        *(buf + offset + i) = color;

}

void xgui_render_line(int handle, int x1, int y1, int x2, int y2, xgui_color_t color)
{
    __ASSERT_VIEW_HANDLE(handle);
    xgui_client_view_t *view = __HANDLE_TO_VIEW(handle);
    if (x1 == x2) { /* 垂直的线 */
        if (y1 < y2) 
            xgui_render_vline(handle, x1, y1, y2, color);
        else 
            xgui_render_vline(handle, x1, y2, y1, color);
        return;
    } else if (y1 == y2) {  /* 水平的直线 */
        if (x1 < x2) 
            xgui_render_hline(handle, x1, x2, y1, color);
        else 
            xgui_render_hline(handle, x2, x1, y1, color);
        return;
    }
    int i, x, y, len, dx, dy;
	dx = x2 - x1;
	dy = y2 - y1;
	
	x = x1 << 10;
	y = y1 << 10;
	
	if(dx < 0){
		dx = -dx;
	}
	if(dy < 0){
		dy = -dy;
	}
	if(dx >= dy ){
		len = dx + 1;
		if(x1 > x2){
			dx = -1024;
		} else {
			dx = 1024;
		}
		if(y1 <= y2){
			dy = ((y2 - y1 + 1) << 10)/len;
		} else {
			dy = ((y2 - y1 - 1) << 10)/len;
		}
	}else{
		len = dy + 1;
		if(y1 > y2){
			dy = -1024;
		} else {
			dy = 1024;
		}
		if(x1 <= x2){
			dx = ((x2 - x1 + 1) << 10)/len;
		} else {
			dx = ((x2 - x1 - 1) << 10)/len;
		}	
	}
	for(i = 0; i < len; i++){
        __xgui_render_putpixel(view, (x >> 10), (y >> 10), color);
		x += dx;
		y += dy;
	}
}

void xgui_render_rect_ex(int handle, int x1, int y1, int x2, int y2, xgui_color_t color)
{
    /* left */
    xgui_render_vline(handle, x1, y1, y2, color);
    /* right */
    xgui_render_vline(handle, x2, y1, y2, color);
    /* top */
    xgui_render_hline(handle, x1, y1, x2, color);
    /* bottom */
    xgui_render_hline(handle, x1, y2, x2, color);
}

void xgui_render_rectfill_ex(int handle, int x1, int y1, int x2, int y2, xgui_color_t color)
{
    int i;
    for (i = 0; i <= y2 - y1; i++) {
        xgui_render_hline(handle, x1, y1 + i, x2, color);
    }
}

void xgui_render_rect(int handle, int x, int y, uint32_t width, uint32_t height, xgui_color_t color)
{
    xgui_render_rect_ex(handle, x, y, x + width - 1, y + height - 1, color);
}

void xgui_render_rectfill(int handle, int x, int y, uint32_t width, uint32_t height, xgui_color_t color)
{
    xgui_render_rectfill_ex(handle, x, y, x + width - 1, y + height - 1, color);
}

void xgui_render_char(
        int handle, 
        int x,
        int y,
        char ch,
        xgui_color_t color)
{
    __ASSERT_VIEW_HANDLE(handle);
    xgui_client_view_t *view = __HANDLE_TO_VIEW(handle);
    xgui_dotfont_t *font = xgui_dotfont_current();
    if (!font)
        return;
    if (!font->addr)
        return;
    uint8_t *data = font->addr + ch * font->char_height;
    unsigned int i;
	uint8_t d /* data */;
	for (i = 0; i < 16; i++) {
		d = data[i];
		if ((d & 0x80) != 0)
            __xgui_render_putpixel(view, x + 0, y + i, color);
		if ((d & 0x40) != 0)
            __xgui_render_putpixel(view, x + 1, y + i, color);
		if ((d & 0x20) != 0)
             __xgui_render_putpixel(view, x + 2, y + i, color);
		if ((d & 0x10) != 0)
            __xgui_render_putpixel(view, x + 3, y + i, color);
		if ((d & 0x08) != 0)
            __xgui_render_putpixel(view, x + 4, y + i, color);
		if ((d & 0x04) != 0)
            __xgui_render_putpixel(view, x + 5, y + i, color);
		if ((d & 0x02) != 0)
            __xgui_render_putpixel(view, x + 6, y + i, color);
		if ((d & 0x01) != 0)
            __xgui_render_putpixel(view, x + 7, y + i, color);
	}
}

void xgui_render_text(
    int handle, 
    int x,
    int y,
    char *text,
    xgui_color_t color)
{
    int cur_x = x;
    int cur_y = y;
    xgui_dotfont_t *font = xgui_dotfont_current();
    while (*text) {
        switch (*text) {
        case '\n':
            cur_x = x;
            cur_y += font->char_height;
            break;
        case '\b':
            cur_x -= font->char_width;
            if (cur_x < x)
                cur_x = x;
            break;
        default:
            xgui_render_char(handle, cur_x, cur_y, *text, color);
            cur_x += 8;
            break;
        }
        text++;
    }
}


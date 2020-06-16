#include <sgi/sgi.h>
#include <sgi/sgii.h>
#include <sgi/sgim.h>
#include <sgi/sgie.h>
#include <sys/srvcall.h>
#include <srv/guisrv.h>
#include <string.h>
#include <stdio.h>
#include <sys/res.h>
#include <sys/ipc.h>

#define DEBUG_LOCAL 1

SGI_Window SGI_CreateSimpleWindow(
    SGI_Display *display,
    SGI_Window parent,
    int x,
    int y,
    unsigned int width,
    unsigned int height,
    unsigned int background
) {
    if (display == NULL)
        return -1;
    if (!display->connected)
        return -1;
    if (parent < 0)
        return -1;

    /* 先检测是否有可用窗口句柄 */
    if (!SGI_DisplayWindowInfoCheck(display)) {
        return -1;      /* 没有可用窗口句柄就返回 */
    }
    
    /* 构建服务调用 */
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_CREATE_WIN, 0);
    SETSRV_ARG(&srvarg, 1, parent, 0);
    SETSRV_ARG(&srvarg, 2, x, 0);
    SETSRV_ARG(&srvarg, 3, y, 0);
    SETSRV_ARG(&srvarg, 4, width, 0);
    SETSRV_ARG(&srvarg, 5, height, 0);
    SETSRV_ARG(&srvarg, 6, background, 0);
    SETSRV_ARG(&srvarg, 7, display->id, 0);
    SETSRV_RETVAL(&srvarg, -1);

    /* 执行服务调用 */
    if (srvcall(SRV_GUI, &srvarg)) {
        return -1;
    }
    if (GETSRV_RETVAL(&srvarg, int) == -1) {
        return -1;
    }

    unsigned int wid = GETSRV_RETVAL(&srvarg, unsigned int);

    char shmname[16];
    memset(shmname, 0, 16);
    sprintf(shmname, "guisrv-win%d", wid);

#if DEBUG_LOCAL == 1    
    printf("[SGI] create window: shm name %s\n", shmname);
#endif
    /* 连接一个共享内存 */
    int shmid = res_open(shmname, RES_IPC | IPC_SHM | IPC_CREAT, 0);
    if (shmid < 0) /* 创建共享内存失败 */
        goto scroll_destry_win;
    
    SGI_WindowInfo winfo;
    winfo.winid = wid;
    winfo.shmid = shmid;
    winfo.mapped_addr = NULL;
    winfo.start_off = 0;
    winfo.width = width;
    winfo.height = height;
    winfo.input_mask = SGI_EventMaskDefault;
    /* 加载默认字体 */
    winfo.font = SGI_LoadFont(display, SGI_FONT_DEFAILT_NAME);
    if (winfo.font == NULL)
        goto scroll_close_shm;
    
    /* 把窗口id放入窗口句柄表 */
    SGI_Window win = SGI_DisplayWindowInfoAdd(display, &winfo);
    /*  */
    return win; /* 返回窗口句柄 */

scroll_close_shm:
    res_close(shmid);
scroll_destry_win:
    /* 销毁窗口 */
    SETSRV_ARG(&srvarg, 0, GUISRV_DESTROY_WIN, 0);
    SETSRV_ARG(&srvarg, 1, wid, 0);
    SETSRV_RETVAL(&srvarg, -1);
    /* 执行服务调用 */
    if (srvcall(SRV_GUI, &srvarg))
        return -1;
    
    if (GETSRV_RETVAL(&srvarg, int) == -1)
        return -1;
    /* 销毁成功 */
    SETSRV_RETVAL(&srvarg, -1);
    return -1;
}

int SGI_DestroyWindow(SGI_Display *display, SGI_Window window)
{
    if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(window))
        return -1;
    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, window);
    if (!winfo)
        return -1;

    printf("[SGI] %s begin.\n", __func__);
    
    /* 构建服务调用消息 */
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_DESTROY_WIN, 0);
    SETSRV_ARG(&srvarg, 1, winfo->winid, 0);
    SETSRV_RETVAL(&srvarg, -1);
    printf("[SGI] %s winid=%d.\n", __func__, winfo->winid);
    
    /* 执行服务调用 */
    if (srvcall(SRV_GUI, &srvarg))
        return -1;
    
    if (GETSRV_RETVAL(&srvarg, int) == -1)
        return -1;
    
    printf("[SGI] shmid:%d\n", winfo->shmid);
    
    /* 关闭共享内存 */
    res_close(winfo->shmid);
    winfo->shmid = -1;
    winfo->mapped_addr = NULL;
    
    SGI_DisplayWindowInfoDel(display, window);

    /* 执行成功 */
    return 0;
}

/**
 * 从服务器映射窗口到客户端
 */
int SGI_MapWindow(SGI_Display *display, SGI_Window window)
{
    if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(window))
        return -1;

    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, window);
    if (!winfo)
        return -1;
    
    /* 已经映射后就不能映射 */
    if (winfo->mapped_addr)
        return -1;
    
    /* 构建服务调用消息 */
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_MAP_WIN, 0);
    SETSRV_ARG(&srvarg, 1, winfo->winid, 0);
    SETSRV_RETVAL(&srvarg, -1);
    /* 执行服务调用 */
    if (srvcall(SRV_GUI, &srvarg))
        return -1;
    
    int retval = GETSRV_RETVAL(&srvarg, int);
    if (retval == -1)
        return -1;
    
    /* 映射窗口显示区域到客户端 */
    long mapped; /* 保存映射后的地址 */
    if (res_write(winfo->shmid, 0, NULL, (size_t) &mapped) < 0) /* 映射共享内存 */
        return -1;
    if (mapped == -1)
        return -1;
 
    winfo->mapped_addr = (void *)mapped;
    winfo->start_off = retval;
#if DEBUG_LOCAL == 1  
    printf("[sgi] map window at %x, start off:%x\n", mapped, winfo->start_off);
#endif   
    /* 执行成功 */
    return 0;
}

/**
 * 从服务器解除客户端窗口映射
 */
int SGI_UnmapWindow(SGI_Display *display, SGI_Window window)
{
    if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(window))
        return -1;
    
    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, window);
    if (!winfo)
        return -1;

    /* 已经映射才能解除映射 */
    if (!winfo->mapped_addr)
        return -1;

#if DEBUG_LOCAL == 1  
    printf("[sgi] unmap window at %x\n", winfo->mapped_addr);
#endif   
    /* 取消映射 */
    if (res_read(winfo->shmid, 0, winfo->mapped_addr, 0) < 0) /* 解除共享内存映射 */
        return -1;

    /* 构建服务调用消息 */
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_UNMAP_WIN, 0);
    SETSRV_ARG(&srvarg, 1, winfo->winid, 0);
    SETSRV_RETVAL(&srvarg, -1);
    /* 执行服务调用 */
    if (srvcall(SRV_GUI, &srvarg))
        return -1;
    
    if (GETSRV_RETVAL(&srvarg, int) == -1)
        return -1;

    winfo->mapped_addr = NULL;
    winfo->start_off = 0;

    /* 执行成功 */
    return 0;
}

/**
 * 刷新窗口的某个区域
 */
int SGI_UpdateWindow(
    SGI_Display *display,
    SGI_Window window,
    int left,
    int top,
    int right,
    int bottom
) {
    if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(window))
        return -1;
    
    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, window);
    if (!winfo)
        return -1;
    /* 使用消息来传递交互信息，速度比服务调用快许多，但是不支持同步 */
    SGI_Msg msg;
    msg.type    = 1;     /* 类型是窗口id */
    msg.arg0    = winfo->winid;
    msg.arg1    = left;
    msg.arg2    = top;
    msg.arg3    = right;
    msg.arg4    = bottom;
    /* 写入消息 */    
    if (res_write(display->request_msgid, IPC_NOWAIT, &msg, SGI_MSG_LEN) < 0)
        return -1;

#if 0
    /* 构建服务调用消息 */
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_UPDATE_WIN, 0);
    SETSRV_ARG(&srvarg, 1, winfo->winid, 0);
    SETSRV_ARG(&srvarg, 2, left, 0);
    SETSRV_ARG(&srvarg, 3, top, 0);
    SETSRV_ARG(&srvarg, 4, right, 0);
    SETSRV_ARG(&srvarg, 5, bottom, 0);
    
    SETSRV_RETVAL(&srvarg, -1);
    /* 执行服务调用 */
    if (srvcall(SRV_GUI, &srvarg))
        return -1;
    
    if (GETSRV_RETVAL(&srvarg, int) == -1)
        return -1;
#endif    
    /* 执行成功 */
    return 0;
}

/**
 * 绘制像素
 */
int SGI_WindowDrawPixel(
    SGI_Display *display,
    SGI_Window window,
    int x,
    int y,
    SGI_Argb color
) {
    if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(window))
        return -1;
    
    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, window);
    if (!winfo)
        return -1;

    /* 参数检测 */
    if (x < 0 || y < 0 || x >= winfo->width || y >= winfo->height)
        return -1;
    
    //printf("[SGI] draw pixel.\n");
    SGI_Argb *p = (SGI_Argb *) ((unsigned char *) winfo->mapped_addr + winfo->start_off);
    p[y *  winfo->width + x] = color;
    /* 执行成功 */
    return 0;
}

/**
 * 绘制像素
 */
static int SGI_WindowDrawPixelWithWinfo(
    SGI_WindowInfo *winfo,
    int x,
    int y,
    SGI_Argb color
) {
    /* 参数检测 */
    if (x < 0 || y < 0 || x >= winfo->width || y >= winfo->height)
        return -1;
    
    //printf("[SGI] draw pixel.\n");
    SGI_Argb *p = (SGI_Argb *) ((unsigned char *) winfo->mapped_addr + winfo->start_off);
    p[y *  winfo->width + x] = color;
    /* 执行成功 */
    return 0;
}

static int SGI_WindowDrawHLine(
    SGI_WindowInfo *winfo,
    int left,
    int right,
    int top,
    SGI_Argb color
) {
    int offset = 0;
    int i      = 0;
    if (left > (winfo->width - 1))
        return  -1;
    if (right > (winfo->width - 1))
        return  -1;
    if (top > (winfo->height - 1))
        return  -1;
    
    unsigned char *vram = (unsigned char *) ((unsigned char *) winfo->mapped_addr + winfo->start_off);
    offset = 4 * ((winfo->width)*top + left);
    if (offset >= ((winfo->width) * (winfo->height) * 4 - 4))
        return  -1;

    for (i = 0; i <= right - left; i++)
        *((SGI_Argb *) ((vram) + offset + 4 * i)) = (SGI_Argb) color;

    return  0;
}

static int SGI_WindowDrawVLine(
    SGI_WindowInfo *winfo,
    int left,
    int top,
    int bottom,
    SGI_Argb color
) {
    int offset = 0;
    int i      = 0;
    if (left > (winfo->width - 1))
        return  -1;
    if (top > (winfo->height - 1))
        return  -1;
    if (bottom > (winfo->height - 1))
        return  -1;

    unsigned char *vram = (unsigned char *) ((unsigned char *) winfo->mapped_addr + winfo->start_off);
    for (i = 0; i <= bottom - top; i++) {
        offset = 4 * ((winfo->width) * (top + i) + left);
        if (offset >= ((winfo->width) * (winfo->height) * 4 - 4))
            return -1;
        *((SGI_Argb *)((vram) + offset)) = (SGI_Argb) color;
    }
    return  0;
}

/**
 * 刷新窗口的某个区域
 */
int SGI_WindowDrawRect(
    SGI_Display *display,
    SGI_Window window,
    int x,
    int y,
    unsigned int width,
    unsigned int height,
    SGI_Argb color
) {
    if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(window))
        return -1;
    
    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, window);
    if (!winfo)
        return -1;
    
    /* left */
    SGI_WindowDrawVLine(winfo, x, y, y + height - 1, color);
    /* right */
    SGI_WindowDrawVLine(winfo, x + width - 1, y, y + height - 1, color);
    /* top */
    SGI_WindowDrawHLine(winfo, x, x + width - 1, y, color);
    /* bottom */
    SGI_WindowDrawHLine(winfo, x, x + width - 1, y + height - 1, color);
    return 0;
}

/* Hardware accelerbrate interface */
static  int  __WindowDrawRectFill(
    SGI_WindowInfo *winfo,
    int left,
    int top,
    int right,
    int bottom,
    SGI_Argb color
) {
    int  offset = 0;
    int  i      = 0;
    int  j      = 0;
    
    if ( left > (winfo->width-1) )
        return  -1;
    if ( right > (winfo->width-1) )
        return  -1;
    if ( top > (winfo->height-1) )
        return  -1;

    unsigned char *vram = (unsigned char *) ((unsigned char *) winfo->mapped_addr + winfo->start_off);
    
    for ( j = 0; j <= bottom - top; j++ )
    {
        offset = 4*((winfo->width)*(top+j)+left);
        if ( offset >= ((winfo->width)*(winfo->height)*4-4))
            return  -1;

        for ( i = 0; i <= right - left; i++ )
            *((SGI_Argb *) ((vram) + offset + 4 * i)) = (SGI_Argb) color;
    }
 
    return  1;
}

/**
 * 刷新窗口的某个区域
 */
int SGI_WindowDrawRectFill(
    SGI_Display *display,
    SGI_Window window,
    int x,
    int y,
    unsigned int width,
    unsigned int height,
    SGI_Argb color
) {
    if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(window))
        return -1;
    
    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, window);
    if (!winfo)
        return -1;
    
    return __WindowDrawRectFill(winfo, x, y, x + width - 1, y + height -1, color);
}

/**
 * 绘制像素位图
 */
int SGI_WindowDrawPixmap(
    SGI_Display *display,
    SGI_Window window,
    int x,
    int y,
    unsigned int width,
    unsigned int height,
    SGI_Argb *pixmap
) {
    if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(window))
        return -1;
    if (!pixmap)
        return -1;
    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, window);
    if (!winfo)
        return -1;
    
    int x0, y0, wx, wy;

    for (y0 = 0; y0 < height; y0++) {
        wy = y + y0;
        if (wy < 0)
            continue;
        if (wy >= winfo->height)  /* 越界 */
            break;
        for (x0 = 0; x0 < width; x0++) {
            wx = x + x0;
            if (wx < 0)
                continue;
            if (wx >= winfo->width)  /* 越界 */
                break;
            /* 写入像素值 */
            SGI_Argb *vram = (SGI_Argb *) ((SGI_Argb *) ((unsigned char *) winfo->mapped_addr + 
                 + winfo->start_off) + wy * winfo->width + wx);
            *vram = pixmap[y0 *  width + x0];
        }
    }
    return 0;
}

int SGI_SetWMName(
    SGI_Display *display,
    SGI_Window window,
    char *name
) {
     if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(window))
        return -1;
    if (!name)
        return -1;

    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, window);
    if (!winfo)
        return -1;

    /* 构建服务调用消息 */
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_SET_WMNAME, 0);
    SETSRV_ARG(&srvarg, 1, winfo->winid, 0);
    SETSRV_ARG(&srvarg, 2, name, strlen(name)+1);

    SETSRV_RETVAL(&srvarg, -1);
    /* 执行服务调用 */
    if (srvcall(SRV_GUI, &srvarg))
        return -1;
    if (GETSRV_RETVAL(&srvarg, int) == -1)
        return -1;
    return 0;
}


int SGI_SetWMIconName(
    SGI_Display *display,
    SGI_Window window,
    char *name
) {
     if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(window))
        return -1;
    if (!name)
        return -1;
        
    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, window);
    if (!winfo)
        return -1;

    /* 构建服务调用消息 */
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_SET_WMICONNAME, 0);
    SETSRV_ARG(&srvarg, 1, winfo->winid, 0);
    SETSRV_ARG(&srvarg, 2, name, strlen(name)+1);

    SETSRV_RETVAL(&srvarg, -1);
    /* 执行服务调用 */
    if (srvcall(SRV_GUI, &srvarg))
        return -1;
    if (GETSRV_RETVAL(&srvarg, int) == -1)
        return -1;
    return 0;
}

int SGI_SetWMIcon(
    SGI_Display *display,
    SGI_Window window,
    SGI_Argb *pixmap,
    unsigned int width,
    unsigned int height
) {
    if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(window))
        return -1;

    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, window);
    if (!winfo)
        return -1;

    /* 构建服务调用消息 */
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_SET_WMICON, 0);
    SETSRV_ARG(&srvarg, 1, winfo->winid, 0);
    SETSRV_ARG(&srvarg, 2, pixmap, width * height * sizeof(SGI_Argb));
    SETSRV_ARG(&srvarg, 3, width, 0);
    SETSRV_ARG(&srvarg, 4, height, 0);

    SETSRV_RETVAL(&srvarg, -1);
    /* 执行服务调用 */
    if (srvcall(SRV_GUI, &srvarg))
        return -1;
    if (GETSRV_RETVAL(&srvarg, int) == -1)
        return -1;
    return 0;
}

int SGI_SelectInput(
    SGI_Display *display,
    SGI_Window w,
    long mask
) {
    if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(w))
        return -1;

    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, w);
    if (!winfo)
        return -1;

    winfo->input_mask = mask;

    /* 构建服务调用消息 */
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_SELECT_INPUT, 0);
    SETSRV_ARG(&srvarg, 1, winfo->winid, 0);
    SETSRV_ARG(&srvarg, 2, mask, 0);
    SETSRV_RETVAL(&srvarg, -1);

    /* 执行服务调用 */
    if (srvcall(SRV_GUI, &srvarg))
        return -1;
    if (GETSRV_RETVAL(&srvarg, int) == -1)
        return -1;

    return 0;   
}

static int SGI_DrawCharBit(
    SGI_Display *display,
    SGI_Window window,
    int x,
    int y,
    unsigned char *data,
    SGI_Argb color
) {
    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, window);
    if (!winfo)
        return -1;

    unsigned int i, j;
    unsigned char d;
    for (j = 0; j < winfo->font->height; j++) {
        d = data[j];
        for (i = 0; i < winfo->font->width; i++) {
            if (d & (1 << i))
                SGI_WindowDrawPixelWithWinfo(winfo, x + (winfo->font->width - i - 1), y + j, color);
        }
    }
    return 0;
}

int SGI_DrawChar(
    SGI_Display *display,
    SGI_Window window,
    int x,
    int y,
    char ch,
    SGI_Argb color
) {
    if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(window))
        return -1;
    
    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, window);
    if (!winfo)
        return -1;
    if (!winfo->font)
        return -1;
    SGI_DrawCharBit(display, window, x, y, winfo->font->addr + ch *
        winfo->font->height, color);
    
    return 0;
}

/**
 * 绘制字符串
 */
int SGI_DrawString(
    SGI_Display *display,
    SGI_Window window,
    int x,
    int y,
    char *str,
    size_t len,
    SGI_Argb color
) {
    if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(window))
        return -1;

    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, window);
    if (!winfo)
        return -1;
    if (!winfo->font)
        return -1;

    int x_begin = x;
    while (*str && len > 0) {
        if (*str == '\n') {
            x = x_begin;
            y += winfo->font->height;
        } else if (*str == '\r') {
        } else {
            SGI_DrawChar(display, window, x, y, *str, color);
            x += winfo->font->width;
        }
        str++;
        len--;
	}
    return 0;
}
#include <sgi/sgi.h>
#include <sgi/sgii.h>
#include <sys/srvcall.h>
#include <srv/guisrv.h>
#include <string.h>
#include <stdio.h>

bool __SGI_DisplayWindowHandleCheck(SGI_Display *display)
{
    int i;
    for (i = 0; i < SGI_WINDOW_HANDLE_NR; i++) {
        if (!display->win_handle_table[i])    /* 找到一个空闲的句柄 */
            return true;
    }
    return false;
}

/*
把窗口id添加到窗口句柄表，并返回句柄值
*/
int __SGI_DisplayWindowHandleAdd(SGI_Display *display, unsigned int wid)
{
    int i;
    for (i = 0; i < SGI_WINDOW_HANDLE_NR; i++) {
        if (!display->win_handle_table[i])    /* 找到一个空闲的句柄 */
            break;
    }
    if (i >= SGI_WINDOW_HANDLE_NR)
        return -1;
    display->win_handle_table[i] = wid;
    return i;   /* 返回窗口句柄值 */
}

/*
* 把窗口从窗口句柄表中移除
*/
int __SGI_DisplayWindowHandleDel(SGI_Display *display, SGI_Window window)
{
    int i;
    for (i = 0; i < SGI_WINDOW_HANDLE_NR; i++) {
        if (display->win_handle_table[i] == window) {
            display->win_handle_table[i] = 0;
            return 0;
        }
    }
    return -1;
}


/*
* 从窗口句柄表查找窗口id
*/
int __SGI_DisplayWindowHandleFind(SGI_Display *display, SGI_Window window)
{
    if (SGI_BAD_WIN_HANDLE(window))
        return -1;
    return display->win_handle_table[window];
}

SGI_Display *SGI_OpenDisplay()
{
    SGI_Display *display = SGI_Malloc(sizeof(SGI_Display));
    if (display == NULL) {
        return NULL;
    }
    memset(display, 0, sizeof(SGI_Display));
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_OPEN_DISPLAY, 0);
    SETSRV_ARG(&srvarg, 1, display, sizeof(SGI_Display));
    SETSRV_IO(&srvarg, SRVIO_USER << 1);
    SETSRV_RETVAL(&srvarg, -1);

    if (srvcall(SRV_GUI, &srvarg)) {
        goto od_free_display;
    }
    if (GETSRV_RETVAL(&srvarg, int) == -1) {
        goto od_free_display;
    }
    /* 链接失败 */
    if (!display->connected)
        goto od_free_display;
    /* success! */
    
    /* 把根窗口添加到窗口句柄 */
    display->root_window = __SGI_DisplayWindowHandleAdd(display, display->root_window);
    
    printf("[SGI] oepn display: root window:%d width:%d height:%d\n",
        display->root_window, display->width, display->height);

    return display;
od_free_display:
    SGI_Free(display);
    return NULL;
}

int SGI_CloseDisplay(SGI_Display *display)
{
    if (!display) {
        return -1;
    }
    /* 释放显示的所有窗口 */
    int i;
    for (i = 0; i < SGI_WINDOW_HANDLE_NR; i++) {
        /* 关闭窗口 */
        display->win_handle_table[i] = 0;
    }
    SGI_Free(display);
    return 0;
}

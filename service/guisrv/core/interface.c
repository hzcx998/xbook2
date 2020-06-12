#include <srv/guisrv.h>
#include <pthread/pthread.h>
#include <guisrv.h>
#include <stdio.h>
#include <string.h>
#include <sgi/sgi.h>
#include <drivers/screen.h>
#include <environment/desktop.h>
#include <environment/interface.h>
#include <window/window.h>
#include <sys/res.h>
#include <sys/ipc.h>

#define DEBUG_LOCAL 0

/* 下一个显示id */
static unsigned int next_display_id;

int do_null(srvarg_t *arg)
{
    arg->retval = 0;
    return 0;
}

int do_open_display(srvarg_t *arg)
{
#if DEBUG_LOCAL == 1    
    printf("[guisrv] open display\n");
#endif
    /* 返回服务信息 */
    static SGI_Display display;
    memset(&display, 0, sizeof(SGI_Display));

    display.id = next_display_id++;     /* 获取id，并指向下一个id */

    /* 构建消息队列名字 */
    char msgname[16];
    memset(msgname, 0, 16);
    sprintf(msgname, "guisrv-display%d", display.id);
    printf("[GUISRV] open msg %s.\n", msgname);
    /* 创建一个消息队列，用来和客户端进程交互 */
    int msgid = res_open(msgname, RES_IPC | IPC_MSG | IPC_CREAT | IPC_EXCL, 0);
    if (msgid < 0) {
        printf("[%s] open msg queue failed!\n", SRV_NAME);
        goto od_error;
    }

    env_display_t disp;
    disp.dispid = display.id;
    disp.msgid = msgid;
    if (env_display_add(&disp)) {
        res_close(msgid);
        goto od_error;
    }

    display.connected = 1;      /* 连接上 */
    display.width = drv_screen.width;
    display.height = drv_screen.height;
    display.root_window = env_desktop.window->id;   /* 桌面窗口的id */

    SETSRV_DATA(arg, 1, &display);
    SETSRV_SIZE(arg, 1, sizeof(SGI_Display));
    SETSRV_RETVAL(arg, 0);  /* 成功 */
    return 0;
od_error:
    SETSRV_DATA(arg, 1, &display);
    SETSRV_SIZE(arg, 1, sizeof(SGI_Display));
    SETSRV_RETVAL(arg, -1);  /* 失败 */
    return -1;
}

int do_close_display(srvarg_t *arg)
{
    unsigned int display_id = GETSRV_DATA(arg, 1, unsigned int);  // display id
    env_display_t *disp = env_display_find(display_id);
    if (!disp) {
        SETSRV_RETVAL(arg, -1);  /* 失败 */
        return -1;
    }
    res_close(disp->msgid);

    env_display_del(display_id);

    SETSRV_RETVAL(arg, 0);
    return 0;
}

int do_create_window(srvarg_t *arg)
{
    gui_window_t *parent = gui_window_get_by_id(GETSRV_DATA(arg, 1, unsigned int));
    if (parent == NULL) {   /* 没有找到父窗口，失败 */
        SETSRV_RETVAL(arg, -1);
        return -1;
    }

    unsigned int display_id = GETSRV_DATA(arg, 7, unsigned int);
    /* 参数检测 */
    if (display_id <= 0) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }

    gui_window_t *win = gui_create_window(
        "win",
        GETSRV_DATA(arg, 2, int), // x
        GETSRV_DATA(arg, 3, int), // y
        GETSRV_DATA(arg, 4, unsigned int), // width
        GETSRV_DATA(arg, 5, unsigned int), // height
        GETSRV_DATA(arg, 6, GUI_COLOR), // background color
        0,  /* 使用默认属性 */
        parent
    );
    
    if (win == NULL) {  /* 创建窗口失败 */
        SETSRV_RETVAL(arg, -1);
        return -1;
    }

    /* 创建一段独一无二的共享内存 */
    char shmname[16];
    memset(shmname, 0, 16);
    sprintf(shmname, "guisrv-win%d", win->id);
#if DEBUG_LOCAL == 1
    printf("[guisrv] create window: shm name %s.\n", shmname);
#endif
    /* 创建一个共享内存 */
    int shmid = res_open(shmname, RES_IPC | IPC_SHM | IPC_CREAT | IPC_EXCL, win->window_size);
    if (shmid < 0) { /* 创建共享内存失败 */
        /* 销毁窗口 */
        gui_destroy_window(win);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    win->shmid = shmid;
    win->display_id = display_id;
#if DEBUG_LOCAL == 1
    printf("[%s] window display id=%d.\n", SRV_NAME, win->display_id);
#endif

    /* 创建成功，添加到窗口缓存表 */
    gui_window_cache_add(win);

    /* 返回窗口id */
    SETSRV_RETVAL(arg, win->id);
    return 0;
}

int do_destroy_window(srvarg_t *arg)
{
    /* 获取窗口id */
    int wid = GETSRV_DATA(arg, 1, int);
    if (wid < 0)
        goto mw_error;

    gui_window_t *win = gui_window_cache_find(wid);   
    if (win == NULL) {  /* 没有在缓存中找到窗口 */
        /*  尝试在窗口链表中寻找 */
        win = gui_window_get_by_id(wid);
        if (win == NULL) /* 在窗口链表中也没有找到，就出错 */
            goto mw_error;
    }
    
    /* 先从窗口缓存表中删除 */
    gui_window_cache_del(win);

    /* 关闭共享内存 */
    res_close(win->shmid);
    win->shmid = -1;

    /* 断开显示连接 */
    win->display_id = 0;

    /* 销毁窗口 */
    if (gui_destroy_window(win))
        goto mw_error;
    
    SETSRV_RETVAL(arg, 0);
    return 0;
mw_error:
    SETSRV_RETVAL(arg, -1);
    return -1;
}

int do_map_window(srvarg_t *arg)
{
    /* 获取窗口id */
    int wid = GETSRV_DATA(arg, 1, int);
    if (wid < 0)
        goto mw_error;

    gui_window_t *win = gui_window_cache_find(wid);   
    if (win == NULL) {  /* 没有在缓存中找到窗口 */
        /*  尝试在窗口链表中寻找 */
        win = gui_window_get_by_id(wid);
        if (win == NULL) /* 在窗口链表中也没有找到，就出错 */
            goto mw_error;
    }

    /* 对窗口进行映射操作 */
    void *mapaddr = (void *) (win->layer->buffer + win->y_off * win->width);
#if DEBUG_LOCAL == 1
    printf("[guisrv] map window share memory at %x\n", mapaddr);
#endif
    long mapped; /* 保存映射后的地址 */
    if (res_write(win->shmid, IPC_RND, mapaddr, (size_t) &mapped) < 0) /* 映射共享内存 */
        goto mw_error;
    if (mapped == -1)
        goto mw_error;
    
    win->mapped_addr = (void *) mapped;
    win->start_off = (unsigned long) mapaddr & 0xfff;    /* 页内偏移 */

#if DEBUG_LOCAL == 1
    printf("[guisrv] mapped window share memory at %x, start off is %x\n", 
        mapped, win->start_off);
#endif    
    /* 映射完后显示窗口 */
    gui_window_show(win);

    /* 返回页内偏移 */
    SETSRV_RETVAL(arg, win->start_off);
    return 0;
mw_error:
    SETSRV_RETVAL(arg, -1);
    return -1;
}


int do_unmap_window(srvarg_t *arg)
{
    /* 获取窗口id */
    int wid = GETSRV_DATA(arg, 1, int);
    if (wid < 0)
        goto mw_error;

    gui_window_t *win = gui_window_cache_find(wid);   
    if (win == NULL) {  /* 没有在缓存中找到窗口 */
        /*  尝试在窗口链表中寻找 */
        win = gui_window_get_by_id(wid);
        if (win == NULL) /* 在窗口链表中也没有找到，就出错 */
            goto mw_error;
    }

    /* 先隐藏窗口 */
    gui_window_hide(win);
    /* 解除窗口的映射 */
#if DEBUG_LOCAL == 1
    printf("[guisrv] unmap window share memory at %x\n", win->mapped_addr);
#endif
    if (res_read(win->shmid, IPC_RND, win->mapped_addr, 0) < 0) /* 解除共享内存映射 */
        goto mw_error;    
    win->mapped_addr = NULL;

    SETSRV_RETVAL(arg, 0);
    return 0;
mw_error:
    SETSRV_RETVAL(arg, -1);
    return -1;
}

int do_update_window(srvarg_t *arg)
{
    /* 获取窗口id */
    int wid = GETSRV_DATA(arg, 1, int);
    if (wid < 0)
        goto mw_error;

    gui_window_t *win = gui_window_cache_find(wid);   
    if (win == NULL) {  /* 没有在缓存中找到窗口 */
        /*  尝试在窗口链表中寻找 */
        win = gui_window_get_by_id(wid);
        if (win == NULL) /* 在窗口链表中也没有找到，就出错 */
            goto mw_error;
    }

    gui_window_update(
        win,
        GETSRV_DATA(arg, 2, int),   // left
        GETSRV_DATA(arg, 3, int),   // top
        GETSRV_DATA(arg, 4, int),   // right
        GETSRV_DATA(arg, 5, int));  // bottom
#if DEBUG_LOCAL == 1
    printf("[GUISRV] update window.\n");
#endif

    SETSRV_RETVAL(arg, 0);
    return 0;
mw_error:
    SETSRV_RETVAL(arg, -1);
    return -1;
}

/* 调用表 */
guisrv_func_t guisrv_call_table[] = {
    do_open_display,
    do_close_display,
    do_create_window,
    do_destroy_window,
    do_map_window,
    do_unmap_window,
    do_update_window,
};

void *guisrv_echo_thread(void *arg)
{
#if DEBUG_LOCAL == 1
    printf("[guisrv] ready bind service.\n");
#endif    
    
    /* 绑定成为服务调用 */
    if (srvcall_bind(SRV_GUI) == -1)  {
        printf("%s: bind srvcall failed, service stopped!\n", SRV_NAME);
        return (void *) -1;
    }

#if DEBUG_LOCAL == 1
    printf("[guisrv] bind service ok.\n");
#endif

    int seq;
    srvarg_t srvarg;
    int callnum;
    while (1)
    {
        memset(&srvarg, 0, sizeof(srvarg_t));
        /* 1.监听服务 */
        if (srvcall_listen(SRV_GUI, &srvarg)) {  
            continue;
        }

#if DEBUG_LOCAL == 1
        printf("%s: srvcall seq=%d.\n", SRV_NAME, seq);
#endif 
        /* 2.处理服务 */
        callnum = GETSRV_DATA(&srvarg, 0, int);
        if (callnum >= 0 && callnum < GUISRV_CALL_NR) {
            guisrv_call_table[callnum](&srvarg);
        }

        seq++;
        /* 3.应答服务 */
        srvcall_ack(SRV_GUI, &srvarg);   

    }
    pthread_exit((void *) -1);
    return NULL;
}

int init_guisrv_interface()
{
    next_display_id = 1;    /* 从1开始 */
    /* 开一个线程来接收服务 */
    pthread_t thread_echo;
    int retval = pthread_create(&thread_echo, NULL, guisrv_echo_thread, NULL);
    if (retval == -1) 
        return -1;

    return 0;
}

#include <srv/guisrv.h>
#include <pthread.h>
#include <guisrv.h>
#include <stdio.h>
#include <string.h>
#include <sgi/sgi.h>
#include <drivers/screen.h>
#include <environment/desktop.h>
#include <environment/interface.h>
#include <environment/winctl.h>
#include <window/window.h>
#include <sys/res.h>
#include <sys/ipc.h>
#include <sys/proc.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#define DEBUG_LOCAL 0

#define SRVBUF_256      256
#define SRVBUF_32K      32768
#define SRVBUF_128K     131072

unsigned char *srvbuf256;
unsigned char *srvbuf32k;
unsigned char *srvbuf128k;

/* 下一个显示id */
static unsigned int next_display_id;

static int do_null(srvarg_t *arg)
{
    SETSRV_RETVAL(arg, 0);
    return 0;
}

static int do_open_display(srvarg_t *arg)
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
#if DEBUG_LOCAL == 1     
    printf("[GUISRV] open msg %s.\n", msgname);
#endif
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

static int do_close_display(srvarg_t *arg)
{
    unsigned int display_id = GETSRV_DATA(arg, 1, unsigned int);  // display id
    env_display_t *disp = env_display_find(display_id);
    if (!disp) {
        SETSRV_RETVAL(arg, -1);  /* 失败 */
        return -1;
    }
    
    /* 删除消息队列 */
    res_ioctl(disp->msgid, IPC_DEL, RES_IPC | IPC_MSG);
    res_close(disp->msgid);
#if DEBUG_LOCAL == 1     
    printf("[%s] %s: msgid=%d\n", SRV_NAME, __func__, disp->msgid);
#endif
    env_display_del(display_id);

    SETSRV_RETVAL(arg, 0);
    return 0;
}

static int do_create_window(srvarg_t *arg)
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
    printf("[guisrv] create window: shm name %s, size %x\n", shmname, win->map_size);
#endif
    /* 创建一个共享内存 */
    int shmid = res_open(shmname, RES_IPC | IPC_SHM | IPC_CREAT | IPC_EXCL, win->map_size);
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
    /* 创建窗口控制 */
    win->winctl = gui_create_winctl(win);
    if (win->winctl == NULL) {
        res_close(win->shmid);      /* 关闭共享内存 */
        gui_destroy_window(win);    /* 销毁窗口 */
        SETSRV_RETVAL(arg, -1);
        return -1;
    }

    /* 创建成功，添加到窗口缓存表 */
    gui_window_cache_add(win);

    /* 返回窗口id */
    SETSRV_RETVAL(arg, win->id);
    return 0;
}

static int do_destroy_window(srvarg_t *arg)
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

    /* 销毁窗口控制 */
    gui_destroy_winctl(win->winctl);
    win->winctl = NULL;

    /* 删除共享内存 */
    res_ioctl(win->shmid, IPC_DEL, RES_IPC | IPC_SHM);
    /* 关闭共享内存 */
    res_close(win->shmid);
#if DEBUG_LOCAL == 1     
    printf("[%s] %s: shmid=%d\n", SRV_NAME, __func__, win->shmid);
#endif    
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

static int do_map_window(srvarg_t *arg)
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

    /* 添加到窗口控制器，并显示 */
    gui_winctl_add(win->winctl);
    gui_winctl_show();

    /* 返回页内偏移 */
    SETSRV_RETVAL(arg, win->start_off);
    return 0;
mw_error:
    SETSRV_RETVAL(arg, -1);
    return -1;
}

static int do_unmap_window(srvarg_t *arg)
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
#if DEBUG_LOCAL == 1     
    printf("[%s] %s addr=%x\n", SRV_NAME, __func__, win->mapped_addr);
#endif
    /* 解除共享内存映射 */
    if (res_read(win->shmid, 0, win->mapped_addr, 0) < 0) 
        goto mw_error;    
    win->mapped_addr = NULL;

    /* 从窗口控制器删除窗口控制 */
    gui_winctl_del(win->winctl);

    /* 先隐藏窗口 */
    gui_window_hide(win);

    /* 解除窗口的映射 */
#if DEBUG_LOCAL == 1
    printf("[guisrv] unmap window share memory at %x\n", win->mapped_addr);
#endif
    
    SETSRV_RETVAL(arg, 0);
    return 0;
mw_error:
    SETSRV_RETVAL(arg, -1);
    return -1;
}

static int do_set_wm_name(srvarg_t *arg)
{
    /* 获取窗口id */
    int wid = GETSRV_DATA(arg, 1, int);
    if (wid < 0)
        goto setwmn_error;

    gui_window_t *win = gui_window_cache_find(wid);   
    if (win == NULL) {  /* 没有在缓存中找到窗口 */
        /*  尝试在窗口链表中寻找 */
        win = gui_window_get_by_id(wid);
        if (win == NULL) /* 在窗口链表中也没有找到，就出错 */
            goto setwmn_error;
    }

    /* 读取标题字符串 */
    size_t nbytes = GETSRV_SIZE(arg, 2);
    int len = MIN(nbytes, SRVBUF_256);
    
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        memset(srvbuf256, 0, len);
        SETSRV_DATA(arg, 2, srvbuf256);
        SETSRV_SIZE(arg, 2, len);
        if (srvcall_fetch(SRV_GUI, arg))
            goto setwmn_error;
    }

    if (!(win->attr & GUIW_NO_TITLE) && win->text_title) {
        /* 设置新内容并显示 */
        win->text_title->set_text(win->text_title, (char *) srvbuf256);
        win->text_title->show(win->text_title);
    }

    SETSRV_RETVAL(arg, 0);
    return 0;
setwmn_error:
    SETSRV_RETVAL(arg, -1);
    return -1;
}

static int do_set_wm_icon_name(srvarg_t *arg)
{
    /* 获取窗口id */
    int wid = GETSRV_DATA(arg, 1, int);
    if (wid < 0)
        goto setwmn_error;

    gui_window_t *win = gui_window_cache_find(wid);   
    if (win == NULL) {  /* 没有在缓存中找到窗口 */
        /*  尝试在窗口链表中寻找 */
        win = gui_window_get_by_id(wid);
        if (win == NULL) /* 在窗口链表中也没有找到，就出错 */
            goto setwmn_error;
    }

    /* 读取标题字符串 */
    size_t nbytes = GETSRV_SIZE(arg, 2);
    int len = MIN(nbytes, SRVBUF_256);
    
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        memset(srvbuf256, 0, len);
        SETSRV_DATA(arg, 2, srvbuf256);
        SETSRV_SIZE(arg, 2, len);
        if (srvcall_fetch(SRV_GUI, arg))
            goto setwmn_error;
    }

    /* 设置小窗口的名字 */

    SETSRV_RETVAL(arg, 0);
    return 0;
setwmn_error:
    SETSRV_RETVAL(arg, -1);
    return -1;
}

static int do_set_wm_icon(srvarg_t *arg)
{
    /* 获取窗口id */
    int wid = GETSRV_DATA(arg, 1, int);
    if (wid < 0)
        goto setwmn_error;

    gui_window_t *win = gui_window_cache_find(wid);   
    if (win == NULL) {  /* 没有在缓存中找到窗口 */
        /*  尝试在窗口链表中寻找 */
        win = gui_window_get_by_id(wid);
        if (win == NULL) /* 在窗口链表中也没有找到，就出错 */
            goto setwmn_error;
    }

    /* 读取标题字符串 */
    size_t nbytes = GETSRV_SIZE(arg, 2);
    int len = MIN(nbytes, SRVBUF_32K);
    
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        memset(srvbuf32k, 0, len);
        SETSRV_DATA(arg, 2, srvbuf32k);
        SETSRV_SIZE(arg, 2, len);
        if (srvcall_fetch(SRV_GUI, arg))
            goto setwmn_error;
    }
    unsigned int width, height;
    width = GETSRV_DATA(arg, 3, unsigned int);
    height = GETSRV_DATA(arg, 4, unsigned int);

    /* 修复图标大小 */
    if (width > GUI_WINCTL_ICON_SIZE)
        width = GUI_WINCTL_ICON_SIZE;
    if (height > GUI_WINCTL_ICON_SIZE)
        height = GUI_WINCTL_ICON_SIZE;
    
    /* 设置图标数据 */
#if DEBUG_LOCAL == 1     
    printf("[%s] set icon (%d, %d)\n", SRV_NAME, width, height);
#endif
    gui_winctl_t *winctl = (gui_winctl_t *) win->winctl;
    if (winctl) {
        winctl->button->set_pixmap(winctl->button, width, height, (GUI_COLOR *) srvbuf32k);
    }
    SETSRV_RETVAL(arg, 0);
    return 0;
setwmn_error:
    SETSRV_RETVAL(arg, -1);
    return -1;
}

static int do_select_input(srvarg_t *arg)
{
    /* 获取窗口id */
    int wid = GETSRV_DATA(arg, 1, int);
    if (wid < 0)
        goto si_error;

    gui_window_t *win = gui_window_cache_find(wid);   
    if (win == NULL) {  /* 没有在缓存中找到窗口 */
        /*  尝试在窗口链表中寻找 */
        win = gui_window_get_by_id(wid);
        if (win == NULL) /* 在窗口链表中也没有找到，就出错 */
            goto si_error;
    }

    /// input mask
    win->input_mask = GETSRV_DATA(arg, 2, long);
#if DEBUG_LOCAL == 1     
    printf("[%s] select input mask %x\n", SRV_NAME, win->input_mask);
#endif
    SETSRV_RETVAL(arg, 0);
    return 0;
si_error:
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
    do_set_wm_name,
    do_set_wm_icon_name,
    do_set_wm_icon,
    do_select_input,
};

/* 文件映射 */
struct file_map {
    char *path;     /* 路径 */
    char execute;   /* 是否需要执行 */
    const char **argv;
};

struct file_map file_map_table[] = {
    {"c:/bin/bosh", 1, NULL},
    {"c:/sbin/xui", 0, NULL},
};

int guisrv_execute()
{
    struct file_map *fmap;
    int pid = 0;
    int i;
    for (i = 0; i < ARRAY_SIZE(file_map_table); i++) {
        fmap = &file_map_table[i];
        if (fmap->execute) {
            pid = fork();
            if (pid < 0) {
                printf("%s: %s: fork failed!\n", SRV_NAME, __func__);
                return -1;
            }
            if (!pid) { /* 子进程执行新进程 */
                if (execv(fmap->path, fmap->argv)) {
                    printf("%s: %s: execv failed!\n", SRV_NAME, __func__);
                    exit(-1);
                }
            }
        }
    }
    return 0;
}

/* 掌控互斥 */
pthread_mutex_t guisrv_master_mutex;

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

    guisrv_execute();


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

        pthread_mutex_lock(&guisrv_master_mutex);

        /* 2.处理服务 */
        callnum = GETSRV_DATA(&srvarg, 0, int);
#if DEBUG_LOCAL == 1
        printf("%s: srvcall seq=%d callnum %d.\n", SRV_NAME, seq, callnum);
#endif 
        if (callnum >= 0 && callnum < GUISRV_CALL_NR) {
            guisrv_call_table[callnum](&srvarg);
        }
#if DEBUG_LOCAL == 1
        printf("%s: srvcall seq=%d callnum %d.\n", SRV_NAME, seq, callnum);
#endif 
        seq++;

        /* 3.应答服务 */
        srvcall_ack(SRV_GUI, &srvarg);   
        pthread_mutex_unlock(&guisrv_master_mutex);
    }
    pthread_exit((void *) -1);
    return NULL;
}

int init_guisrv_interface()
{
    next_display_id = 1;    /* 从1开始 */

    srvbuf256 = gui_malloc(SRVBUF_256);
    if (srvbuf256 == NULL) {
        return -1;
    }
    memset(srvbuf256, 0, SRVBUF_256);
    srvbuf32k = gui_malloc(SRVBUF_32K);
    if (srvbuf32k == NULL) {
        return -1;
    }
    memset(srvbuf32k, 0, SRVBUF_32K);
    srvbuf128k = gui_malloc(SRVBUF_128K);
    if (srvbuf128k == NULL) {
        return -1;
    }
    memset(srvbuf128k, 0, SRVBUF_128K);

    pthread_mutex_init(&guisrv_master_mutex, NULL);

    /* 开一个线程来接收服务 */
    pthread_t thread_echo;
    int retval = pthread_create(&thread_echo, NULL, guisrv_echo_thread, NULL);
    if (retval == -1) 
        return -1;

    return 0;
}

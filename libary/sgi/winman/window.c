#include <sgi/sgi.h>
#include <sgi/sgii.h>
#include <sys/srvcall.h>
#include <srv/guisrv.h>
#include <string.h>
#include <stdio.h>
#include <sys/res.h>
#include <sys/ipc.h>

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
    if (!__SGI_DisplayWindowHandleCheck(display)) {
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
    SETSRV_RETVAL(&srvarg, -1);

    /* 执行服务调用 */
    if (srvcall(SRV_GUI, &srvarg)) {
        return -1;
    }
    if (GETSRV_RETVAL(&srvarg, int) == -1) {
        return -1;
    }

    unsigned int wid = GETSRV_RETVAL(&srvarg, unsigned int);
#if 0   
    char shmname[16];
    memset(shmname, 0, 16);
    sprintf(shmname, "guisrv-win%d", wid);
    printf("[SGI] create window: shm name %s.\n", shmname);
    /* 连接一个共享内存 */
    int shmid = res_open(shmname, RES_IPC | IPC_SHM | IPC_CREAT, width * height * sizeof(unsigned int));
    if (shmid < 0) { /* 创建共享内存失败 */
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
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
#endif
    /* 把窗口id放入窗口句柄表 */
    SGI_Window win = __SGI_DisplayWindowHandleAdd(display, wid);

    /*  */
    return win; /* 返回窗口句柄 */
}

int SGI_DestroyWindow(SGI_Display *display, SGI_Window window)
{
    if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(window))
        return -1;
    int wid = __SGI_DisplayWindowHandleFind(display, window);
    if (wid < 0)
        return -1;
    
    /* 构建服务调用消息 */
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_DESTROY_WIN, 0);
    SETSRV_ARG(&srvarg, 1, wid, 0);
    SETSRV_RETVAL(&srvarg, -1);
    /* 执行服务调用 */
    if (srvcall(SRV_GUI, &srvarg))
        return -1;
    
    if (GETSRV_RETVAL(&srvarg, int) == -1)
        return -1;
    
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
    
    int wid = __SGI_DisplayWindowHandleFind(display, window);
    if (wid < 0)
        return -1;
    
    /* 构建服务调用消息 */
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_MAP_WIN, 0);
    SETSRV_ARG(&srvarg, 1, wid, 0);
    SETSRV_RETVAL(&srvarg, -1);
    /* 执行服务调用 */
    if (srvcall(SRV_GUI, &srvarg))
        return -1;
    
    if (GETSRV_RETVAL(&srvarg, int) == -1)
        return -1;
    
    /* 映射窗口显示区域到客户端 */
    


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
    
    int wid = __SGI_DisplayWindowHandleFind(display, window);
    if (wid < 0)
        return -1;
    
    /* 构建服务调用消息 */
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_UNMAP_WIN, 0);
    SETSRV_ARG(&srvarg, 1, wid, 0);
    SETSRV_RETVAL(&srvarg, -1);
    /* 执行服务调用 */
    if (srvcall(SRV_GUI, &srvarg))
        return -1;
    
    if (GETSRV_RETVAL(&srvarg, int) == -1)
        return -1;
    
    /* 执行成功 */
    return 0;
}


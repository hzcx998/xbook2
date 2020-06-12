#ifndef __SGI_WM_H__    /* window management */
#define __SGI_WM_H__

typedef int SGI_Window;

typedef struct _SGI_Color {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
    unsigned char alpha;
} SGI_Color;

typedef unsigned int SGI_Argb;

#define __SGI_ARGB(a, r, g, b)  (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)) 
#define SGI_ARGB(a, r, g, b)    __SGI_ARGB((a) & 0xff, (r)  & 0xff, (g) & 0xff, (b) & 0xff)
#define SGI_RGB(r, g, b)        SGI_ARGB(0xff, r, g, b)

/* 常用颜色 */
#define SGIC_RED        SGI_RGB(255, 0, 0)
#define SGIC_GREEN      SGI_RGB(0, 255, 0)
#define SGIC_BLUE       SGI_RGB(0, 0, 255)
#define SGIC_WHITE      SGI_RGB(255, 255, 255)
#define SGIC_BLACK      SGI_RGB(0, 0, 0)
#define SGIC_GRAY       SGI_RGB(195, 195, 195)
#define SGIC_LEAD       SGI_RGB(127, 127, 127)
#define SGIC_YELLOW     SGI_RGB(255, 255, 0)
#define SGIC_NONE       SGI_RGB(0, 0, 0)

/* 窗口句柄信息描述 */
typedef struct _SGI_WindowInfo {
    int flags;                  /* 信息标志 */
    int winid;                  /* 窗口id */
    int shmid;                  /* 共享内存id，指向显存共享 */
    void *mapped_addr;          /* 共享内存映射后的地址 */
    unsigned short start_off;   /* 起始偏移 */
    unsigned int width;         /* 窗口宽度 */
    unsigned int height;        /* 窗口高度 */
} SGI_WindowInfo;

#endif  /* __SGI_WM_H__ */
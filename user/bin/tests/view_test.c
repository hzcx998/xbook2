#include "test.h"

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int *bits;
} viewbmp_t;

typedef struct {
    int x;      // 视图x
    int y;      // 视图y
    int bx;     // 位图x
    int by;     // 位图y
    int bw;     // 位图宽度
    int bh;     // 位图高度
    viewbmp_t bmp;
} viewio_t;

typedef struct {
    uint32_t id;        /* 消息id */
    int target;         /* 消息目标 */
    uint32_t data0;     
    uint32_t data1;     
    uint32_t data2;     
    uint32_t data3;
} view_msg_t;

void viewbmp_putpixel(viewbmp_t *bmp, int x, int y, uint32_t color)
{
    if (!bmp)
        return;
    if (x < 0 || y < 0 || x >= bmp->width || y >= bmp->height)
        return;
    bmp->bits[y * bmp->width + x] = color;
}

void viewbmp_rectfill(viewbmp_t *bmp, int w, int h, uint32_t color)
{
    if (!bmp)
        return;
    int x, y;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            viewbmp_putpixel(bmp, x, y, color);
        }
    }
}

int view_test(int argc, char *argv[])
{
    printf("start view test\n");
    int winsz = (640 << 16) | 480;
    int fd = opendev("view0", winsz);
    if (fd < 0) {
        printf("open view dev failed!\n");
        return -1;
    }
    int pos = (200 << 16) | 100;
    ioctl(fd, VIEWIO_SETPOS, &pos);
    ioctl(fd, VIEWIO_SHOW, 0);
    #if 0
    int buf[32 * 32];

    int i;
    for (i = 0; i < 32 * 32; i++) {
        buf[i] = (0xff << 24) + (((i * 20) & 0xff) << 16) + \
            (((i * 10) & 0xff) << 8) + ((i * 5) & 0xff);
    }

    viewio_t vio;
    memset(&vio, 0, sizeof(viewio_t));
    vio.bmp.width = 32;
    vio.bmp.height = 32;
    vio.bmp.bits = buf;
    vio.bw = 32;
    vio.bh = 32;
    ioctl(fd, VIEWIO_WRBMP, &vio);
    #else
        #if 1
        uint32_t *colors = malloc(640 * 480 * 4);
        viewio_t vio;
        memset(&vio, 0, sizeof(viewio_t));
        vio.bmp.width = 640;
        vio.bmp.height = 480;
        vio.bmp.bits = colors;
        vio.bw = 640;
        vio.bh = 480;
        int i = 0; 
        /*
        int flags = DEV_NOWAIT;
        ioctl(fd, VIEWIO_SETFLGS, &flags);*/
        view_msg_t msg;
        while (1) {
            viewbmp_rectfill(&vio.bmp, vio.bmp.width, vio.bmp.height, 
                (0xff << 24) + (((i * 20) & 0xff) << 16) + \
                (((i * 10) & 0xff) << 8) + ((i * 5) & 0xff));
            ioctl(fd, VIEWIO_WRBMP, &vio);
            i++;
            read(fd, &msg, sizeof(view_msg_t));
        }
        #else
        unsigned int *buf = (unsigned int *) 0x1000;
        
        viewio_t vio;
        memset(&vio, 0, sizeof(viewio_t));
        vio.bmp.width = 230;
        vio.bmp.height = 100;
        vio.bmp.bits = buf;
        vio.bw = 230;
        vio.bh = 100;
        ioctl(fd, VIEWIO_WRBMP, &vio);

        #endif
    #endif
    printf("sleep 3s!\n");
    sleep(3);
    printf("close view!\n");
    close(fd);
    return 0;
}
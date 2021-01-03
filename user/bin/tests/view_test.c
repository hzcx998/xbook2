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


/* 视图类型 */
enum view_type {
    VIEW_TYPE_FIXED      = 0,
    VIEW_TYPE_WINDOW,
    VIEW_TYPE_FLOAT,
};

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

typedef struct {
    int left;
    int right;
    int top;
    int bottom;
} view_region_t;

static inline void view_region_reset(view_region_t *rect)
{
    rect->left = -1;
    rect->top = -1;
    rect->right = -1;
    rect->bottom = -1;
}

static inline void view_region_init(view_region_t *rect, int left, int top, int right, int bottom)
{
    rect->left = left;
    rect->top = top;
    rect->right = right;
    rect->bottom = bottom;
}

static inline int view_region_valid(view_region_t *rect)
{
    if (rect->left != -1 && rect->right != -1 &&
        rect->top != -1 && rect->bottom != -1)
        return 1;
    return 0;
}

#define view_region_in_range(rect, x, y) \
        ((rect)->left <= (x) && (x) < (rect)->right && \
        (rect)->top <= (y) && (y) < (rect)->bottom)

/* view message */
enum {
    VIEW_MSG_NONE = 0,
    VIEW_MSG_KEY_DOWN,
    VIEW_MSG_KEY_UP,
    VIEW_MSG_MOUSE_MOTION,
    VIEW_MSG_MOUSE_LBTN_DOWN,
    VIEW_MSG_MOUSE_LBTN_UP,
    VIEW_MSG_MOUSE_LBTN_DBLCLK,
    VIEW_MSG_MOUSE_RBTN_DOWN,
    VIEW_MSG_MOUSE_RBTN_UP,
    VIEW_MSG_MOUSE_RBTN_DBLCLK,
    VIEW_MSG_MOUSE_MBTN_DOWN,
    VIEW_MSG_MOUSE_MBTN_UP,
    VIEW_MSG_MOUSE_MBTN_DBLCLK,
    VIEW_MSG_MOUSE_WHEEL_UP,
    VIEW_MSG_MOUSE_WHEEL_DOWN,
    VIEW_MSG_MOUSE_WHEEL_LEFT,
    VIEW_MSG_MOUSE_WHEEL_RIGHT,
    VIEW_MSG_TIMER,
    VIEW_MSG_QUIT,
    VIEW_MSG_ENTER,
    VIEW_MSG_LEAVE,
    VIEW_MSG_RESIZE,
    VIEW_MSG_ACTIVATE,
    VIEW_MSG_INACTIVATE,
    VIEW_MSG_MOVE,
    VIEW_MSG_CREATE,
    VIEW_MSG_CLOSE,
    VIEW_MSG_HIDE,
    VIEW_MSG_SHOW,
    VIEW_MSG_PAINT,
    VIEW_MSG_NR,
};

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
    
    int type = VIEW_TYPE_WINDOW;
    ioctl(fd, VIEWIO_SETTYPE, &type);
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
        int winw = 640, winh = 480;
        uint32_t *colors = malloc(winw * winh * 4);
        
        
        int i = 0; 
        /*
        int flags = DEV_NOWAIT;
        ioctl(fd, VIEWIO_SETFLGS, &flags);*/
        view_msg_t msg;
        while (1) {
            viewio_t vio;
            memset(&vio, 0, sizeof(viewio_t));
            vio.bmp.width = winw;
            vio.bmp.height = winh;
            vio.bmp.bits = colors;
            vio.bw = winw;
            vio.bh = winh;
            viewbmp_rectfill(&vio.bmp, vio.bmp.width, vio.bmp.height, 
                (0xff << 24) + (((i * 20) & 0xff) << 16) + \
                (((i * 10) & 0xff) << 8) + ((i * 5) & 0xff));
            ioctl(fd, VIEWIO_WRBMP, &vio);
            
            i++;
            if (!read(fd, &msg, sizeof(view_msg_t))) {
                /* 解析消息 */
                //printf("get msg %d\n", msg.id);
                switch (msg.id)
                {
                case VIEW_MSG_RESIZE:
                    //printf("resize msg %d, %d, %d, %d\n", msg.data0, msg.data1, msg.data2, msg.data3);
                    
                    free(colors);
                    winw = msg.data2;
                    winh = msg.data3;
                    colors = malloc(winw * winh * 4);
                    break;
                
                default:
                    break;
                }
            }
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
#include "test.h"

#include <libview.h>

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
            vio.refresh = 1;
            viewbmp_rectfill(&vio.bmp, vio.bmp.width, vio.bmp.height, 
                (0xff << 24) + (((i * 20) & 0xff) << 16) + \
                (((i * 10) & 0xff) << 8) + ((i * 5) & 0xff));
            ioctl(fd, VIEWIO_WRBMP, &vio);
            view_region_t region = {0, 0, winw, winh};
            ioctl(fd, VIEWIO_REFRESH, &region);
            
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
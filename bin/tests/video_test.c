#include "test.h"
#include <sys/vmm.h>
#include <sys/ioctl.h>
#include <sys/time.h>

int video_test(int argc, char *argv[])
{
    int fb0 = open("/dev/video", 0);
    if (fb0 < 0) {
        printf("open video device failed!\n");
        return -1;
    }
    video_info_t video_info;
    if (ioctl(fb0, VIDEOIO_GETINFO, &video_info) < 0) {
        printf("get video info failed!\n");
        close(fb0);
        return -1;
    }

    int bytes_per_pixel = video_info.bits_per_pixel / 8;
    size_t fb_len = bytes_per_pixel * video_info.y_resolution * video_info.x_resolution; 
    void *fb_buf = xmmap(fb0, fb_len, 0);
    if (fb_buf == NULL) {
        printf("mmap failed!\n");
        close(fb0);
        return -1;
    }
    struct timeval time1 = {0, 0};
    struct timeval time2 = {0, 0};
    int fps = 0;
    int total_fps = 0;
    int test_count = 0;
    gettimeofday(&time1, NULL);
    /* 查看1s可刷新的次数，测试10次，求平均数 */
    while (test_count < 10)
    {
        memset(fb_buf, test_count * 10, fb_len);
        memset(fb_buf, test_count * 10, fb_len);
        memset(fb_buf, test_count * 10, fb_len);
        memset(fb_buf, test_count * 10, fb_len);
        
        fps++;
        gettimeofday(&time2, NULL);
        unsigned long long mtime = (time2.tv_sec - time1.tv_sec) * 1000000 + (time2.tv_usec - time1.tv_usec);
        if (mtime > 1000000) {
            printf("fps %d\n", fps);
            total_fps += fps;
            fps = 0;
            time1 = time2;
            test_count++;
        }
    }
    printf("total fps:%d , per fps in 10s:%d\n", total_fps, total_fps / 10);
    xmunmap(fb_buf, fb_len);
    close(fb0);
    printf("video test done!\n");
    return 0;
}

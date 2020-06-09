#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>

#include <srv/guisrv.h>
#include <sys/srvcall.h>
#include <sys/proc.h>

/* SGI: Simple graphical interface */

typedef unsigned int SGI_Window;

typedef struct _SGI_Display
{
    unsigned int width;             /* 窗口宽度 */
    unsigned int height;            /* 窗口高度 */
    SGI_Window root_window;         /* 根窗口 */
} SGI_Display;

void *SGI_Malloc(size_t size)
{
    return malloc(size);
}

void SGI_Free(void *ptr)
{
    free(ptr);
}


SGI_Display *SGI_OpenDisplay()
{
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, GUISRV_OPEN_DISPLAY, 0);

    if (!srvcall(SRV_GUI, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return NULL;
        }
        SGI_Display *display = SGI_Malloc(sizeof(SGI_Display));
        if (display == NULL) {
            return NULL;
        }
        return display;
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    printf("hello, test!\n");
    sleep(1);

    SGI_Display *display = SGI_OpenDisplay();
    if (display == NULL) {
        printf("[test] open gui failed!\n");
        return -1;
    }
    printf("[test] open gui ok!\n");

    return 0;
}

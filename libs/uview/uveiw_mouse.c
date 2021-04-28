#include "uview.h"
#include "uview_io.h"
#include <sys/ioctl.h>
#include <unistd.h>

int uview_set_mouse_state(int vfd, uview_mouse_state_t state)
{
    return ioctl(vfd, VIEWIO_SETMOUSESTATE, &state);
}

int uview_set_mouse_state_info(int vfd, uview_mouse_state_info_t *info)
{
    if (!info)
        return -1;
    return ioctl(vfd, VIEWIO_SETMOUSESTATEINFO, info);
}

int uview_set_mouse_state_noview(uview_mouse_state_t state)
{
    /* open temp view */
    int vfd = uview_open_tmp();
    if (vfd < 0)
        return -1;
    if (ioctl(vfd, VIEWIO_SETMOUSESTATE, &state) < 0) {    
        close(vfd);
        return -1;
    }
    close(vfd);
    return 0;
}

int uview_set_mouse_state_info_noview(uview_mouse_state_info_t *info)
{
    if (!info)
        return -1;
    /* open temp view */
    int vfd = uview_open_tmp();
    if (vfd < 0)
        return -1;
    if (ioctl(vfd, VIEWIO_SETMOUSESTATEINFO, info) < 0) {    
        close(vfd);
        return -1;
    }
    close(vfd);
    return 0;
}
/*
#define VIEWIO_GETMOUSESTATE 0
#define VIEWIO_GETMOUSESTATEINFO 1
*/
int uview_get_mouse_state(int vfd, uview_mouse_state_t *state)
{
    if (!state)
        return -1;
        
    return ioctl(vfd, VIEWIO_GETMOUSESTATE, state);
}

int uview_get_mouse_state_noview(uview_mouse_state_t *state)
{
    if (!state)
        return -1;
    /* open temp view */
    int vfd = uview_open_tmp();
    if (vfd < 0)
        return -1;
    if (ioctl(vfd, VIEWIO_GETMOUSESTATE, state) < 0) {    
        close(vfd);
        return -1;
    }
    close(vfd);
    return 0;
}

int uview_get_mouse_state_info(int vfd, uview_mouse_state_info_t *info)
{
    if (!info)
        return -1;
    return ioctl(vfd, VIEWIO_GETMOUSESTATEINFO, info);
}

int uview_get_mouse_state_info_noview(uview_mouse_state_info_t *info)
{
    if (!info)
        return -1;
    /* open temp view */
    int vfd = uview_open_tmp();
    if (vfd < 0)
        return -1;
    if (ioctl(vfd, VIEWIO_GETMOUSESTATEINFO, info) < 0) {    
        close(vfd);
        return -1;
    }
    close(vfd);
    return 0;
}

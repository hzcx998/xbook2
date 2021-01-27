#include "uview_mouse.h"
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

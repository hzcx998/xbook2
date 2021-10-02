#include <xbook/debug.h>

#include <dwin/dwin.h>

void dwin_init(void)
{
    infoprint("[dwin] init\n");

    if (dwin_hal_init() < 0)
    {
        goto _failed;
    }

    

    infoprint("[dwin] init success\n");
    return;

_failed:
    infoprint("[dwin] init failed\n");
}
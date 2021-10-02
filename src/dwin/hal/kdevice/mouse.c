#include <dwin/hal.h>
#include <stddef.h>

struct dwin_hal_mouse __kdevice_mouse_hal = {
    .init = NULL,
    .exit = NULL,
    .extension = NULL,
};

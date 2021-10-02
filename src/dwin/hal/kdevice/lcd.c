#include <dwin/hal.h>
#include <stddef.h>

struct dwin_hal_lcd __kdevice_lcd_hal = {
    .init = NULL,
    .exit = NULL,
    .extension = NULL,
};

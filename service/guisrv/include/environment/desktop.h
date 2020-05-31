#ifndef __GUISRV_ENVIRONMENT_DESKTOP_H__
#define __GUISRV_ENVIRONMENT_DESKTOP_H__

#include <window/window.h>

typedef struct _env_desktop {
    gui_window_t *window;       /* 桌面窗口 */
} env_desktop_t;

extern env_desktop_t env_desktop;

int init_env_desktop();

#endif  /* __GUISRV_ENVIRONMENT_DESKTOP_H__ */

#ifndef __GUISRV_WINDOW_EVENT_H__
#define __GUISRV_WINDOW_EVENT_H__

#include "window.h"

#include <sgi/sgie.h>

#define gui_event_t     SGI_Event
#define gui_event_msg_t SGI_EventMsg

int gui_window_send_event(gui_window_t *window, gui_event_t *event);



#endif  /* __GUISRV_WINDOW_EVENT_H__ */

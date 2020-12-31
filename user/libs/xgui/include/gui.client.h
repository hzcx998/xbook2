#ifndef _CLIENT_XGUI_H
#define _CLIENT_XGUI_H

#include <sys/lpc.h>

enum gui_client_code {
    GUICALL_create_view = FIRST_CALL_CODE,
    GUICALL_destroy_view,
    GUICALL_show_view,
    GUICALL_hide_view,
    GUICALL_move_view,
    GUICALL_update_view,
    GUICALL_LAST_CALL,
};

#endif  /* _CLIENT_XGUI_H */
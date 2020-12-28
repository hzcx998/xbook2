#include <gui.client.h>
#include <assert.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>
#include <sys/lpc.h>

#include "xgui_view.h"

#include "xgui_vrender.h"

bool remote_xgui_view_create(lpc_parcel_t data, lpc_parcel_t reply)
{
    int x, y, width, height;
    lpc_parcel_read_int(data, (uint32_t *)&x);
    lpc_parcel_read_int(data, (uint32_t *)&y);
    lpc_parcel_read_int(data, (uint32_t *)&width);
    lpc_parcel_read_int(data, (uint32_t *)&height);
    xgui_view_t *view = xgui_view_create(x, y, width, height);
    if (!view) {
        lpc_parcel_write_int(reply, -EPERM);
        return false;
    }
    xgui_vrender_rectfill(view, 0, 0, view->width, view->height, XGUI_WHITE);
    lpc_parcel_write_int(reply, view->id);
    lpc_parcel_write_int(reply, view->section->handle);
    return true;    
}

bool remote_xgui_view_destroy(lpc_parcel_t data, lpc_parcel_t reply)
{
    int id;
    lpc_parcel_read_int(data, (uint32_t *)&id);
    xgui_view_t *view = xgui_view_find_by_id(id);
    if (!view) {
        printf("xgui: view %d not found\n", id);
        lpc_parcel_write_int(reply, -EPERM);
        return false;
    }
    xgui_view_hide(view);
    int retval = xgui_view_destroy(view);
    if (retval < 0) {
        lpc_parcel_write_int(reply, retval);
        return false;
    }
    lpc_parcel_write_int(reply, retval);
    return true;  
}
bool remote_xgui_view_move(lpc_parcel_t data, lpc_parcel_t reply)
{
    int id, x, y;
    lpc_parcel_read_int(data, (uint32_t *)&id);
    lpc_parcel_read_int(data, (uint32_t *)&x);
    lpc_parcel_read_int(data, (uint32_t *)&y);
    xgui_view_t *view = xgui_view_find_by_id(id);
    if (!view) {
        printf("xgui: view %d not found\n", id);
        lpc_parcel_write_int(reply, -EPERM);
        return false;
    }

    int retval = xgui_view_set_xy(view, x, y);
    if (retval < 0) {
        lpc_parcel_write_int(reply, retval);
        return false;
    }
    lpc_parcel_write_int(reply, retval);
    return true;
}
bool remote_xgui_view_show(lpc_parcel_t data, lpc_parcel_t reply)
{
    int id;
    lpc_parcel_read_int(data, (uint32_t *)&id);
    xgui_view_t *view = xgui_view_find_by_id(id);
    if (!view) {
        printf("xgui: view %d not found\n", id);
        lpc_parcel_write_int(reply, -EPERM);
        return false;
    }
    int retval = xgui_view_show(view);
    if (retval < 0) {
        lpc_parcel_write_int(reply, retval);
        return false;
    }
    lpc_parcel_write_int(reply, retval);
    return true;
}

bool remote_xgui_view_hide(lpc_parcel_t data, lpc_parcel_t reply)
{
    int id;
    lpc_parcel_read_int(data, (uint32_t *)&id);
    xgui_view_t *view = xgui_view_find_by_id(id);
    if (!view) {
        printf("xgui: view %d not found\n", id);
        lpc_parcel_write_int(reply, -EPERM);
        return false;
    }
    int retval = xgui_view_hide(view);
    if (retval < 0) {
        lpc_parcel_write_int(reply, retval);
        return false;
    }
    lpc_parcel_write_int(reply, retval);
    return true;
}
bool remote_xgui_view_update(lpc_parcel_t data, lpc_parcel_t reply)
{
    int id, left, right, top, bottom;
    lpc_parcel_read_int(data, (uint32_t *)&id);
    lpc_parcel_read_int(data, (uint32_t *)&left);
    lpc_parcel_read_int(data, (uint32_t *)&top);
    lpc_parcel_read_int(data, (uint32_t *)&right);
    lpc_parcel_read_int(data, (uint32_t *)&bottom);
    xgui_view_t *view = xgui_view_find_by_id(id);
    if (!view) {
        printf("xgui: view %d not found\n", id);
        lpc_parcel_write_int(reply, -EPERM);
        return false;
    }
    xgui_view_refresh(view, left, top, right, bottom);
    lpc_parcel_write_int(reply, 0);
    return true;
}

static lpc_remote_handler_t gui_remote_table[] = {
    remote_xgui_view_create,
    remote_xgui_view_destroy,
    remote_xgui_view_show,
    remote_xgui_view_hide,
    remote_xgui_view_move,
    remote_xgui_view_update
};

bool guiserv_echo_main(uint32_t code, lpc_parcel_t data, lpc_parcel_t reply)
{
    if (code >= FIRST_CALL_CODE && code < GUICALL_LAST_CALL)
        return gui_remote_table[code - 1](data, reply);
    return false;
}

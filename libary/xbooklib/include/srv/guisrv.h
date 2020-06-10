#ifndef _SRV_GUI_SRV_H
#define _SRV_GUI_SRV_H

/* gui server call */
enum guisrv_call_num {
    GUISRV_OPEN_DISPLAY = 0,
    GUISRV_CLOSE_DISPLAY,
    GUISRV_CREATE_WIN,
    GUISRV_DESTROY_WIN,
    GUISRV_MAP_WIN,
    GUISRV_UNMAP_WIN,
    GUISRV_CALL_NR,    /* 最大数量 */
};



#endif   /* _SRV_GUI_SRV_H */
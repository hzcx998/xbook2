#ifndef  _APP_WIN_MAIN_H
#define  _APP_WIN_MAIN_H

#include  <learninggui.h>

    
#ifdef  __cplusplus
extern  "C"
{
#endif


    extern  HWND  ghmain;

    int   main_frame_routine(/* GUI_MESSAGE *msg */ void *msg);
    HWND  create_user_main_window(void);


#ifdef  __cplusplus
}
#endif

#endif  /* _APP_WIN_MAIN_H */

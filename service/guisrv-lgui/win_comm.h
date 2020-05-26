#ifndef  _APP_WIN_COMM_H
#define  _APP_WIN_COMM_H

#include  <learninggui.h>

    
#ifdef  __cplusplus
extern  "C"
{
#endif

    extern  GUI_COMMON_WIDGET   gcomp;
    extern  WIDGET_UNION        ginput;


    HWND  create_frame1(void);
    int   close_frame1(void);

    HWND  create_frame2(void);
    int   close_frame2(void);

    HWND  create_frame3(void);
    int   close_frame3(void);

    HWND  create_frame4(void);
    int   close_frame4(void);

    HWND  create_frame5(void);
    int   close_frame5(void);


    int   close_all_frame(void);

#ifdef  __cplusplus
}
#endif

#endif  /* _APP_WIN_COMM_H */

#include  <learninggui.h>

#include  "win_comm.h"


GUI_COMMON_WIDGET   gcomp  = {0};
WIDGET_UNION        ginput = {{{0}}};

    
int   close_all_frame(void)
{
    close_frame1();
    close_frame2();
    close_frame3();
    close_frame4();
    close_frame5();

    return  1;
}

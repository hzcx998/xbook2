#include  <string.h>
#include  <stdio.h>

#include  <learninggui.h>

#include  "guisrv.h"


#include  "message_routine.h"
#include  "counter.h"
#include  "win_comm.h"


/* Message routine */
int  message_main_routine(/* GUI_MESSAGE *msg */ void *msg)
{         
    int   key_value  = 0;
    int x, y;

    if ( msg == NULL )
        return  -1;

    switch ( MESSAGE_GET_ID(msg) )
    {
        case  MSG_KEY_UP:
            key_value = MESSAGE_GET_KEY_VALUE(msg);
            switch ( key_value )
            {
                case  GUI_KEY_F1:
                    close_all_frame();
                    create_frame1();
                    return  0;

                case  GUI_KEY_F2:
                    close_all_frame();
                    create_frame2();
                    return  0;

                case  GUI_KEY_F3:
                    close_all_frame();
                    create_frame3();
                    return  0;

                case  GUI_KEY_F4:
                    close_all_frame();
                    create_frame4();
                    return  0;

                case  GUI_KEY_F5:
                    close_all_frame();
                    create_frame5();
                    return  0;

                case  GUI_KEY_Q:
                    message_post_quit();
                    return  0;

                default:
                    break;
            }
            break;

        case  MSG_QUIT: 
            message_post_quit();
            return  0;

        default:
            break;
    }

    return  1;
}

#include <string.h>
#include <stdio.h>
#include <stdio.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>


#include  <learninggui.h>

#include  "driver_lcd.h"
#include  "gui_main.h"
#include "guisrv.h"

int main(int argc, char *argv[])
{
    printf("%s: started.\n", SRV_NAME);


    GUI_MESSAGE  msg = {0};
    int          ret = 0;
  

    /*
     *  Step 1: register driver(s)
     */
    /* User screen */
    #ifdef  _LG_SCREEN_
    register_screen();
    #endif
    printf("register_screen done\n");

    /*
     *  Step 2: call gui_open
     */
    ret = gui_open( );
    if ( ret < 0 )
        return  -1;

    printf("gui_open done\n");

    /*
     *  Step 3: init system
     */


    /*
     *  Step 4: call message_set_routine
     */
    ret = message_set_routine(message_user_main_routine);
    if ( ret < 0 )
        return  -1;

    printf("message_set_routine done\n");


    /*
     *  Step 5: create and show user GUI
     */
    paint_gui_main( ); 

    printf("paint_gui_main done\n");

    /*
     *  Step 6: message loop
     */
    while( 1 )
    {
        /* Loop style 1 */
        ret = message_get(&msg);
        if (ret > 0)
        {
            if ( MESSAGE_IS_QUIT(&msg) )
                break;

            message_dispatch(&msg);
        }

        /* Deal app code */
        ;

    }
    printf("quit learning GUI.\n");


    /*
     *  Step 7: user clean 
     */


    /*
     *  Step 8: call gui_open
     */
    gui_close( );

    return 0;
}

#include <string.h>
#include <stdio.h>
#include <stdio.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>


#include  <learninggui.h>

#include  "guisrv.h"

#include  "driver_lcd.h"
#include  "driver_keyboard.h"
#include  "driver_mtjt.h"

#define DEBUG_WINDOW 1

#include  "message_routine.h"
#include  "counter.h"
#include  "gui_main.h"

#if DEBUG_WINDOW == 1
#include  "app_font.h"

#include  "win_comm.h"
#endif



int main(int argc, char *argv[])
{
    printf("%s: started.\n", SRV_NAME);
    GUI_MESSAGE  msg = {0};
    int          ret = 0;

#if DEBUG_WINDOW == 1
    HWND         p    = NULL; 
#endif

    /*
     *  Step 1: register driver(s)
     */
    /* User screen */
    #ifdef  _LG_SCREEN_
    register_screen();
    #endif
    /* User keyboard */
    #ifdef    _LG_KEYBOARD_
    register_keyboard();
    #endif
    /* User mtjt */
    #ifdef    _LG_MTJT_
    register_mtjt();
    #endif

    /*
     *  Step 2: call gui_open
     */
    ret = gui_open( );
    if ( ret < 0 )
        return  -1;

    /*
     *  Step 3: init system
     */
    
    /*
     *  Step 3: init system
     */
    /* Set window default font */

#if DEBUG_WINDOW == 1
    win_set_window_default_font((GUI_FONT *)(&app_font));
    /* Set client default_font */
    win_set_client_default_font((GUI_FONT *)(&app_font));
#endif

    /*
     *  Step 4: call message_set_routine
     */

#if DEBUG_WINDOW == 1    
    ret = message_set_routine(message_main_routine);
#else
    ret = message_set_routine(message_user_main_routine);
#endif
    if ( ret < 0 )
        return  -1;

    /*
     *  Step 5: create and show user GUI
     */
#if DEBUG_WINDOW == 1    
     /* Create frame1 */
    p = create_frame1();
    if ( p == NULL )
        return  -1;
#else
    paint_gui_main();
#endif

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

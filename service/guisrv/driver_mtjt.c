#include  <string.h>
#include  <unistd.h>
#include  <sys/input.h>
#include  <sys/res.h>
#include  <sys/ioctl.h>

#include  <message.h>
#include  <mtjt.h>
#include  <driver.h>

#include  "driver_mtjt.h"

#include  "guisrv.h"
    
#ifdef    _LG_MTJT_


#ifndef  GUI_MTJT_DEVICE_NAME
#define  GUI_MTJT_DEVICE_NAME        "mouse"
#endif



static  int   mtjt_res = 0;


static  int  input_open_mtjt(void)
{
    /* 
     * Your mtjt device name: /dev/input/event3. 
     * Yes or no ? 
     */
    mtjt_res = res_open( GUI_MTJT_DEVICE_NAME, RES_DEV, 0);
    if ( mtjt_res < 0 )
        return  -1;

    return  1;
}

static  int  input_close_mtjt(void)
{
    return  res_close(mtjt_res);
}

static  int  input_read_mtjt(void *msg)
{
    static int  x_rel                     = 0;
    static int  y_rel                     = 0;
    static int  flag_rel                  = 0;

    GUI_MESSAGE *p = (GUI_MESSAGE *)msg;

    struct      input_event  event;
    GUI_POINT   point;
    int         ret = 0;


    READ_MTJT_START:
    memset( &event, 0, sizeof(event));
    ret = res_read( mtjt_res, 0, &event, sizeof(event) );
    if ( ret < 1 )
        return  0;
    //printf("get mouse data\n");
    switch (event.type)
    {        
        case EV_REL:
            if ( (event.code) == REL_X )
            {
                x_rel    += event.value; 
                flag_rel  = 1;

                goto  READ_MTJT_START;

            } else if ( (event.code) == REL_Y ) {
                y_rel    += event.value; 
                flag_rel  = 1;

                goto  READ_MTJT_START;

            } else if ( (event.code) == REL_WHEEL ) {
                p->id          = MSG_MTJT_WHEEL;
                p->data0.value = event.value;
                return  1;           
            } else {
                p->id = MSG_MTJT_TYPE;
                p->data0.value = event.value;
                p->data1.value = event.code;
                return  1;
            }
            break;

        case EV_KEY:
            if ( (event.code) == BTN_LEFT )
            {
                if ( (event.value) > 0 )
                    p->id = MSG_MTJT_LBUTTON_DOWN;
                else
                    p->id = MSG_MTJT_LBUTTON_UP;

                in_mtjt_get_point_abs(&point);
                p->data0.value = point.x;
                p->data1.value = point.y;
                return  1;
            } else if ( (event.code) == BTN_MIDDLE ) {
                if ( (event.value) > 0 )
                    p->id = MSG_MTJT_MBUTTON_DOWN;
                else
                    p->id = MSG_MTJT_MBUTTON_UP;

                in_mtjt_get_point_abs(&point);
                p->data0.value = point.x;
                p->data1.value = point.y;
                return  1;
            } else if ( (event.code) == BTN_RIGHT ) {
                if ( (event.value) > 0 )
                    p->id = MSG_MTJT_RBUTTON_DOWN;
                else
                    p->id = MSG_MTJT_RBUTTON_UP;

                in_mtjt_get_point_abs(&point);
                p->data0.value = point.x;
                p->data1.value = point.y;
                return  1;
            } else {
                p->id = MSG_KEY_TYPE;
                p->data0.value = event.value;
                p->data1.value = event.code;
                return  1;
            }
            break;

        case EV_MSC:
            p->id = MSG_MSC_TYPE;
            p->data0.value = event.value;
            p->data1.value = event.code;
            return  1;

        case EV_SYN:
            point.x = x_rel;
            point.y = y_rel;
            in_mtjt_set_point_rel(&point);

            x_rel = 0;
            y_rel = 0;

            if ( flag_rel == 1 )
            {
                p->id = MSG_MTJT_MOVE;

                in_mtjt_get_point_abs(&point);
                p->data0.value = point.x;
                p->data1.value = point.y;
                flag_rel = 0;
                return  1;
            }
            flag_rel = 0;
            break;

        default:
            break;
    }

    return  0;
}

static  int  input_write_mtjt(void *buffer, unsigned int len)
{
    return  1;
}

static  int  input_control_mtjt(void *p1, void *p2)
{
    return  1;
}

static  int  input_reinit_mtjt(void)
{
    return  1;
}

int  register_mtjt(void)
{
    GUI_MTJT   mtjt  = { 0 };
    memset(&mtjt, 0, sizeof(mtjt));

    mtjt.open        = input_open_mtjt;
    mtjt.close       = input_close_mtjt;

    mtjt.read        = input_read_mtjt;
    mtjt.write       = input_write_mtjt;

    mtjt.control     = input_control_mtjt;
    mtjt.reinit      = input_reinit_mtjt;

    in_driver_register(DRIVER_MTJT, &mtjt);

    return  1;
}

#endif  /* _LG_MTJT_ */

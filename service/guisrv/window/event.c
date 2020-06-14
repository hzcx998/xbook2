#include <environment/interface.h>
#include <window/event.h>
#include <stdio.h>
#include <guisrv.h>
#include <sys/res.h>
#include <sys/ipc.h>

int gui_window_send_event(gui_window_t *window, gui_event_t *event)
{
    //printf("[%s] send event to window.\n", SRV_NAME);
    if (window->display_id > 0) {   /* 有显示id */
        env_display_t *disp = env_display_find(window->display_id);
        if (disp) {
            /* 比对窗口选择的输入，如果有，才会进行发送 */
            switch (event->type)
            {
            case SGI_KEY:
                if (event->key.state == SGI_PRESSED) {
                    if (!(window->input_mask & SGI_KeyPressMask))
                        goto wsend_end;
                } else { 
                    if (!(window->input_mask & SGI_KeyRleaseMask))
                        goto wsend_end;
                }
                break;
            case SGI_MOUSE_BUTTON:
                if (event->button.state == SGI_PRESSED) {
                    if (!(window->input_mask & SGI_ButtonPressMask))
                        goto wsend_end;
                } else { 
                    if (!(window->input_mask & SGI_ButtonRleaseMask))
                        goto wsend_end;
                }
                break;
            case SGI_MOUSE_MOTION:
                if (event->motion.state == SGI_ENTER) {
                    if (!(window->input_mask & SGI_EnterWindow))
                        goto wsend_end;
                } else if (event->motion.state == SGI_LEAVE) {
                    if (!(window->input_mask & SGI_LeaveWindow))
                        goto wsend_end;
                } else {
                    if (!(window->input_mask & SGI_PointerMotionMask))
                        goto wsend_end;
                }
                break;
            default:
                break;
            }
            gui_event_msg_t msg;
            msg.type = window->id;  // 窗口id
            msg.event = *event;
            /* 往消息队列发送消息 */
            if (res_write(disp->msgid, IPC_NOWAIT, &msg, sizeof(gui_event_t)) < 0)
                return -1;
            
            return 0;
        }
    } else {    /* 没有显示id */

    }
wsend_end:
    return -1;
}

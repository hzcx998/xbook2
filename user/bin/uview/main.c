#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/proc.h>
#include <uview.h>
static void child();
int main(int argc, char **argv)
{
    int vid1 = uview_open(100, 100);
    if (vid1 < 0) {
        printf("open %d failed!\n", vid1);
        return -1;
    }
    uview_set_pos(vid1, 200, 400);
    uview_set_monitor(vid1, 1);

    uview_show(vid1);
    if (!fork())
        child();
    uview_msg_t msg;
    uview_msg_t xmsg;
    int childid = -1;
    int tcount = 0;
    uview_add_timer(vid1, 1000);

    while (1)
    {
        if (read(vid1, &msg, sizeof(uview_msg_t)) >= 0) {
            int target = uview_msg_get_target(&msg);
            switch (uview_msg_get_type(&msg))
            {
            case UVIEW_MSG_CREATE:
                childid = target;
                printf("taskbar get create msg from %d\n", target);
                break;
            case UVIEW_MSG_CLOSE:
                if (childid == target)
                    childid = -1;
                printf("taskbar get close msg from %d\n", target);
                break;
            case UVIEW_MSG_HIDE:
                printf("taskbar get hide msg from %d\n", target);
                break;
            case UVIEW_MSG_SHOW:
                printf("taskbar get show msg from %d\n", target);
                break;
            case UVIEW_MSG_ACTIVATE:
                printf("taskbar get focus msg from %d\n", target);
                break;
            case UVIEW_MSG_INACTIVATE:
                printf("taskbar lost focus msg from %d\n", target);
                break;
            case UVIEW_MSG_KEY_DOWN:
                {
                    int code = uview_msg_get_key_code(&msg);
                    switch (code) {
                    case UVIEW_KEY_1:
                        
                        uview_msg_header(&xmsg, UVIEW_MSG_HIDE, childid);
                        if (uview_send_msg(vid1, &xmsg) < 0)
                            printf("send bad\n");
                        
                        break;
                    case UVIEW_KEY_2:
                        uview_msg_header(&xmsg, UVIEW_MSG_SHOW, childid);
                        if (uview_send_msg(vid1, &xmsg) < 0)
                            printf("send bad\n");
                        break;
                    case UVIEW_KEY_3:
                        uview_msg_header(&xmsg, UVIEW_MSG_ACTIVATE, childid);
                        if (uview_send_msg(vid1, &xmsg) < 0)
                            printf("send bad\n");
                        break;
                    case UVIEW_KEY_4:
                        uview_msg_header(&xmsg, UVIEW_MSG_INACTIVATE, childid);
                        if (uview_send_msg(vid1, &xmsg) < 0)
                            printf("send bad\n");
                        break;
                    case UVIEW_KEY_5:
                        uview_msg_header(&xmsg, UVIEW_MSG_CLOSE, childid);
                        if (uview_send_msg(vid1, &xmsg) < 0)
                            printf("send bad\n");
                        break;
                    default:
                        break;
                    }
                }
                break;
            case UVIEW_MSG_TIMER:    
                uview_add_timer(vid1, 1000);
                printf("tcount:%d\n", tcount);
                tcount++;
                break;
            default:
                break;
            }
            
        }
    }
    return 0;
}

static void child()
{
    int vid = uview_open(320, 240);
    if (vid < 0) {
        printf("open %d failed!\n", vid);
        exit(-1);
    }
    uview_show(vid);
    int vreadid;
    uview_get_vid(vid, &vreadid);
    uview_msg_t msg;
    bool loop = true;
    while (loop)
    {
        if (read(vid, &msg, sizeof(uview_msg_t)) >= 0) {
            int target = uview_msg_get_target(&msg);
            if (vreadid == target)
                printf("child same %d\n", target);
            switch (uview_msg_get_type(&msg))
            {
            case UVIEW_MSG_CREATE:
                printf("child get create msg from %d\n", target);
                
                break;
            case UVIEW_MSG_CLOSE:
                printf("child get close msg from %d\n", target);
                uview_close(vid);
                loop = false;
                break;
            case UVIEW_MSG_HIDE:
                printf("child get hide msg from %d\n", target);
                uview_hide(vid);
                break;
            case UVIEW_MSG_SHOW:
                printf("child get show msg from %d\n", target);
                uview_show(vid);
                break;
            case UVIEW_MSG_ACTIVATE:
                printf("child get focus msg from %d\n", target);
                break;
            case UVIEW_MSG_INACTIVATE:
                printf("child lost focus msg from %d\n", target);
                break;
            }
        }
    }
    exit(0);
}
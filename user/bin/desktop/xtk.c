#include "xtk.h"
#include <uview.h>
#include <stdlib.h>
#include <assert.h>
#include <dotfont.h>

static int __xtk_init_done = 0;
static int __xtk_main_loop = 0;

int __xtk_has_window_close = 0;

int xtk_init(int *argc, char **argv[])
{
    // TODO: init everything...    
    xtk_text_init();
    xtk_view_init();
    __xtk_main_loop = 1;
    __xtk_has_window_close = 0;
    __xtk_init_done = 1;
    return 0;
}

int xtk_exit(int exit_code)
{
    if (!__xtk_init_done) {
        perror("xtk_init not called before xtk_exit!");
    }
    // TODO: do exit everything
    
    __xtk_init_done = 0;
    
    exit(exit_code);
    return 0;
}

int xtk_main_poll()
{
    if (!__xtk_init_done) {
        perror("xtk_init not called before xtk_main!");
        abort();
    }
    xtk_spirit_t *spirit;
    uview_msg_t msg;
    xtk_view_t *pview, *vnext;
    int filter_val; 
    int view_nr = xtk_view_length();
    if (__xtk_main_loop && view_nr > 0) {    
        xtk_view_for_each_safe (pview, vnext) {
            if (view_nr > 1) { // 2个及其以上的视图才不阻塞
                uview_set_nowait(pview->view, 1);
            } else {    // 只有一个的时候就要阻塞
                uview_set_nowait(pview->view, 0);
            }
            if (uview_get_msg(pview->view, &msg) < 0) {
                continue;
            }
            __xtk_has_window_close = 0; /* 没有窗口关闭 */
            filter_val = 1; /* 消息没有过滤掉 */

            // 遍历每一个视图来获取上面的精灵
            list_for_each_owner (spirit, &pview->spirit_list_head, list) {                
                if (!(filter_val = xtk_window_main(spirit, &msg)))
                    break;
                if (__xtk_has_window_close)
                    break;
            }

            // 需要检测一下是否有窗口已经关闭掉了，如果是那么就不再往后面执行
            if (__xtk_has_window_close) {
                continue;
            }
            spirit = pview->spirit;
            // 没有过滤掉才处理用户消息
            if (filter_val)
                xtk_window_filter_msg(XTK_WINDOW(spirit), &msg);
        }
    }
    return 0;
}
int xtk_main()
{
    if (!__xtk_init_done) {
        perror("xtk_init not called before xtk_main!");
        abort();
    }
    while (__xtk_main_loop && xtk_view_length()) {
        xtk_main_poll();
    }
    return 0;
}

int xtk_main_quit()
{
    if (!__xtk_init_done) {
        perror("xtk_init not called before xtk_main_quit!");
        abort();
    }
    // 退出xtk_view
    xtk_view_t *pview, *pnext;
    xtk_view_for_each_safe (pview, pnext) {
        assert(pview->spirit);
        xtk_window_quit(pview->spirit);
            // xtk_xxx_quit
    }
    __xtk_main_loop = 0;
    return 0;
}

#include "xtk.h"
#include <uview.h>
#include <stdlib.h>
#include <assert.h>
#include <dotfont.h>

static int __xtk_init_done = 0;
static int __xtk_main_loop = 0;

int xtk_init(int *argc, char **argv[])
{
    if (!__xtk_init_done) {
        // TODO: init everything...
        __xtk_main_loop = 1;

        __xtk_init_done = 1;
        return 0;
    }
    return -1;
}

int xtk_exit(int exit_code)
{
    if (!__xtk_init_done)
        abort();
    // TODO: do exit everything
    
    __xtk_init_done = 0;
    
    exit(exit_code);
    return 0;
}

int xtk_main()
{
    if (!__xtk_init_done)
        abort();
    xtk_spirit_t *spirit;
    uview_msg_t msg;
    xtk_view_t *pview;
    while (__xtk_main_loop) {    
        xtk_view_for_each (pview) {
            uview_set_wait(pview->view, 1);
            if (uview_get_msg(pview->view, &msg) < 0) {
                continue;
            }
            // 遍历每一个视图来获取上面的精灵
            list_for_each_owner (spirit, &pview->spirit_list_head, list) {
                xtk_window_main(spirit, &msg);
                // xtk_xxx_main
            }
        }
    }
    return 0;
}

int xtk_main_quit()
{
    if (!__xtk_init_done)
        return -1;
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
